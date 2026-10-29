#include <cassert>
#include <cstddef>
#include <utility>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include "atlas/array.h"
#if ATLAS_VERSION >= 4100
#include "atlas/linalg/sparse/SparseMatrixToTriplets.h"
#endif
#include "atlas/grid.h"
#include "atlas/functionspace.h"
#include "atlas/field.h"
#include "atlas/interpolation.h"
#include "atlas/library.h"
#include "eckit/runtime/Main.h"


namespace py = pybind11;
using namespace pybind11::literals;


namespace {

#if ATLAS_VERSION >= 4100
    using SparseMatrixStorage = atlas::linalg::SparseMatrixStorage;
#else
    using SparseMatrixStorage = eckit::linalg::SparseMatrix;
#endif

    SparseMatrixStorage assemble_global_matrix(const atlas::Interpolation& interpolation, int mpi_root) {

        using namespace atlas;

        auto src_fs = interpolation.source();
        auto tgt_fs = interpolation.target();

        auto& mpi_comm = mpi::comm();
        auto mpi_size  = mpi_comm.size();
        auto mpi_rank  = mpi_comm.rank();

        std::vector<gidx_t> global_cols;
        std::vector<gidx_t> global_rows;
        std::vector<double> global_vals;

        // Compute global_cols, global_rows, global_vals
        {
            std::vector<gidx_t> local_cols;
            std::vector<gidx_t> local_rows;
            std::vector<double> local_vals;

            // Compute local_cols, local_rows, local_vals
            {
                interpolation::MatrixCache interpolation_cache = interpolation.createCache();
#if ATLAS_VERSION >= 4100
                auto local_matrix_storage = interpolation_cache.matrix();
                auto local_matrix = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(local_matrix_storage);
#else
                auto local_matrix = interpolation_cache.matrix();
#endif
                const auto src_ridx = array::make_view<idx_t, 1>(src_fs.remote_index());

                const auto src_global_index = array::make_view<gidx_t, 1>(src_fs.global_index());
                const auto tgt_global_index = array::make_view<gidx_t, 1>(tgt_fs.global_index());
                const auto src_part         = array::make_view<int, 1>(src_fs.partition());
                const auto tgt_ghost        = array::make_view<int, 1>(tgt_fs.ghost());

                eckit::mpi::Buffer<gidx_t> recv_global_idx_buf(mpi_size);
                {
                    std::vector<gidx_t> send_global_idx(src_global_index.size());
                    for (int i = 0; i < send_global_idx.size(); ++i) {
                        send_global_idx[i] = src_global_index[i];
                    }
                    mpi_comm.allGatherv(send_global_idx.begin(), send_global_idx.end(), recv_global_idx_buf);
                }
                auto recv_global_idx = [&recv_global_idx_buf](int part, int ridx) {
                    return recv_global_idx_buf.buffer[ recv_global_idx_buf.displs[part] + ridx ];
                };
                auto src_gidx = [&](auto idx) {
                    return recv_global_idx(src_part(idx), src_ridx(idx));
                };

#if ATLAS_VERSION >= 4100
                auto nnz = local_matrix.nnz();
#else
                auto nnz = local_matrix.nonZeros();
#endif
                local_cols.reserve(nnz);
                local_rows.reserve(nnz);
                local_vals.reserve(nnz);

                const auto* outer  = local_matrix.outer();
                const auto* inner  = local_matrix.inner();
#if ATLAS_VERSION >= 4100
                const auto* value  = local_matrix.value();
#else
                const auto* value  = local_matrix.data();
#endif
                const size_t nrows = local_matrix.rows();

                for(idx_t r = 0; r < nrows; ++r) {
                    if( not tgt_ghost(r) ) {
                        idx_t row = tgt_global_index(r);
                        for (idx_t c = outer[r]; c < outer[r + 1]; ++c) {
                            auto src_idx = inner[c];
                            auto col = src_gidx(src_idx);
                            auto val = value[c];
                            local_rows.emplace_back(row);
                            local_cols.emplace_back(col);
                            local_vals.emplace_back(val);
                        }
                    }
                }
            }

            // Gather local to global on mpi_root
            size_t local_nnz = local_vals.size();
            std::vector<size_t> local_nnz_per_rank(mpi_size);
            mpi_comm.gather(local_nnz, local_nnz_per_rank, mpi_root);

            constexpr int mpi_tag = 0;
            if (mpi_rank == mpi_root) {
                size_t global_nnz = std::accumulate(local_nnz_per_rank.begin(), local_nnz_per_rank.end(), 0);
                global_cols.resize(global_nnz);
                global_rows.resize(global_nnz);
                global_vals.resize(global_nnz);
                size_t pos = 0;
                size_t pos_root = 0;
                for (int jproc = 0; jproc < mpi_size; ++jproc) {
                    if (jproc == mpi_root) {
                        pos += (jproc == 0) ? 0 : local_nnz_per_rank[jproc - 1];
                        pos_root = pos;
                        continue;
                    }
                    pos += (jproc == 0 ? 0 : local_nnz_per_rank[jproc - 1]);
                    mpi_comm.receive(global_cols.data() + pos, local_nnz_per_rank[jproc], jproc, mpi_tag);
                    mpi_comm.receive(global_rows.data() + pos, local_nnz_per_rank[jproc], jproc, mpi_tag);
                    mpi_comm.receive(global_vals.data() + pos, local_nnz_per_rank[jproc], jproc, mpi_tag);
                }
                for (int i = 0; i < local_nnz; ++i) {
                    global_cols[i + pos_root] = local_cols[i];
                    global_rows[i + pos_root] = local_rows[i];
                    global_vals[i + pos_root] = local_vals[i];
                }
            }
            else {
                mpi_comm.send(local_cols.data(), local_cols.size(), mpi_root, mpi_tag);
                mpi_comm.send(local_rows.data(), local_rows.size(), mpi_root, mpi_tag);
                mpi_comm.send(local_vals.data(), local_vals.size(), mpi_root, mpi_tag);
            }
        }


        auto compute_max_global_index = [&mpi_comm,mpi_root](const FunctionSpace& fs) {
            auto global_index = array::make_view<gidx_t, 1>(fs.global_index());
            auto ghost        = array::make_view<int, 1>(fs.ghost());
            gidx_t max_gidx{0};
            for( size_t i = 0; i < global_index.size(); ++i) {
                if (not ghost(i)) {
                    max_gidx = std::max(max_gidx, global_index(i));
                }
            }
            // eckit::mpi::Comm::reduceInPlace() was only added in eckit 1.22.2
            mpi_comm.reduceInPlace(max_gidx, eckit::mpi::max(), mpi_root);
            return max_gidx;
        };

        // compute max column and row indices
        gidx_t tgt_max_gidx = compute_max_global_index(tgt_fs);
        gidx_t src_max_gidx = compute_max_global_index(src_fs);

#if ATLAS_VERSION >= 4100
        if (mpi_rank == mpi_root) {
            size_t nrows = tgt_max_gidx;
            size_t ncols = src_max_gidx;
            size_t index_base = 1;
            bool is_sorted = false;
            return atlas::linalg::make_sparse_matrix_storage_from_rows_columns_values(nrows, ncols, global_rows, global_cols, global_vals, index_base, is_sorted);
        }
        else {
            return atlas::linalg::SparseMatrixStorage();
        }
#else
        if (mpi_rank == mpi_root) {
            size_t nrows = tgt_max_gidx;
            size_t ncols = src_max_gidx;
            size_t index_base = 1;
            bool is_sorted = false;
            std::vector<eckit::linalg::Triplet> triplets;
            triplets.reserve(global_vals.size());
            for (size_t i = 0; i < global_vals.size(); ++i) {
                triplets.emplace_back(global_rows[i] - index_base, global_cols[i] - index_base, global_vals[i]);
            }
            std::sort(triplets.begin(), triplets.end());
            return eckit::linalg::SparseMatrix(nrows, ncols, triplets);
        } else {
            return eckit::linalg::SparseMatrix(0,0);
        }
#endif
    }

    template<typename To, typename From>
    auto atlas_dynamic_cast(const From& from) {
        auto view = To(from);
        if (view.valid()) {
            return std::make_optional(std::move(view));
        } else {
            return std::optional<decltype(view)>();
        }
    }
} //  namespace



template<typename T, int DIM>
atlas::array::LocalView<T, DIM> make_local_view(py::array_t<T>& arr) {
    auto buffer_info = arr.request();
    assert(buffer_info.ndim == DIM);

    int shape[DIM];
    for (size_t i = 0; i < DIM; ++i) {
        shape[i] = buffer_info.shape[i];
    }

    int strides[DIM];
    for (size_t i = 0; i < DIM; ++i) {
        strides[i] = buffer_info.strides[i] / sizeof(T);
    }

    return atlas::array::LocalView<T, DIM>{static_cast<T*>(buffer_info.ptr), shape, strides};
}


template<typename Fn, size_t... idx>
decltype(auto) py_apply_impl(Fn&& f, const py::tuple& t, std::index_sequence<idx...>) {
    return f(t[idx]...);
}


template<int ArgN, typename Fn, size_t... idx>
decltype(auto) py_apply(Fn&& f, const py::tuple& t) {
    if(t.size() != ArgN) {
        throw std::runtime_error("Wrong number of input args.");
    }
    return py_apply_impl(f, t, std::make_index_sequence<ArgN>{});
}


std::string to_py_buffer_format(const atlas::array::DataType& atlas_dtype) {
    if (atlas_dtype == atlas::array::DataType::byte()) {
        return py::format_descriptor<std::byte>::format();
    } else if (atlas_dtype == atlas::array::DataType::int32()) {
        return py::format_descriptor<int>::format();
    } else if (atlas_dtype == atlas::array::DataType::int64()) {
        return py::format_descriptor<long>::format();
    } else if (atlas_dtype == atlas::array::DataType::real32()) {
        return py::format_descriptor<float>::format();
    } else if (atlas_dtype == atlas::array::DataType::real64()) {
        return py::format_descriptor<double>::format();
    } else if (atlas_dtype == atlas::array::DataType::uint64()) {
        return py::format_descriptor<unsigned long>::format();
    } else {
        throw std::runtime_error("Unsupported data type.");
    }
};


atlas::array::DataType to_atlas_dtype(const std::string& py_format) {
    if (py_format == py::format_descriptor<std::byte>::format()) {
        return atlas::array::DataType::byte();
    } else if (py_format == py::format_descriptor<int>::format()) {
        return atlas::array::DataType::int32();
    } else if (py_format == py::format_descriptor<long>::format()) {
        return atlas::array::DataType::int64();
    } else if (py_format == py::format_descriptor<float>::format()) {
        return atlas::array::DataType::real32();
    } else if (py_format == py::format_descriptor<double>::format()) {
        return atlas::array::DataType::real64();
    } else if (py_format == py::format_descriptor<unsigned long>::format()) {
        return atlas::array::DataType::uint64();
    } else {
        throw std::runtime_error("Unsupported buffer format.");
    }
}


template<typename ValueTy, int RANK>
void register_array_view(py::module& m) {

    const auto suffix = "_" + atlas::array::DataType::str<ValueTy>() + "_" + std::to_string(RANK) + "d";

    const auto class_name = "ArrayView" + suffix;
    py::class_<atlas::array::ArrayView<ValueTy, RANK>>(m, class_name.c_str())
        .def_property_readonly("shape", [](const atlas::array::ArrayView<ValueTy, RANK>& self) {
            py::tuple py_shape(RANK);
            for (size_t i = 0; i < RANK; ++i) {
                py_shape[i] = self.shape(i);
            }
            return py_shape;
        })
        .def_property_readonly("strides", [](const atlas::array::ArrayView<ValueTy, RANK>& self) {
            py::tuple py_stride(RANK);
            for (size_t i = 0; i < RANK; ++i) {
                py_stride[i] = self.stride(i);
            }
            return py_stride;
        })
        .def_property_readonly("size", &atlas::array::ArrayView<ValueTy, RANK>::size)
        .def_property_readonly("datatype", [](const atlas::array::ArrayView<ValueTy, RANK>& self) {
            return atlas::array::DataType::create<ValueTy>();
        })
        .def_property_readonly("rank", [](const atlas::array::ArrayView<ValueTy, RANK>& self) {
            return RANK;
        })
        .def("data", [](atlas::array::ArrayView<ValueTy, RANK>& self) {
            return reinterpret_cast<std::uintptr_t>(self.data());
        })
        .def("data", [](const atlas::array::ArrayView<ValueTy, RANK>& self) {
            return reinterpret_cast<std::uintptr_t>(self.data());
        })
        .def("__getitem__", [](atlas::array::ArrayView<ValueTy, RANK>& self, const py::tuple& indices) {
            return py_apply<RANK>(
                [&self](auto&&... args) {
                    return self(py::cast<unsigned int>(args)...);
                },
                indices
            );
        })
        .def("__setitem__", [](atlas::array::ArrayView<ValueTy, RANK>& self, const py::tuple& indices, ValueTy val) {
            py_apply<RANK>(
                [&self, val](auto&&... args) {
                    self(py::cast<unsigned int>(args)...) = val;
                },
                indices
            );
        });

    const auto fn_name = "make_array_view" + suffix;
    m.def(fn_name.c_str(), [](atlas::Field& field) {
        return atlas::array::make_view<ValueTy, RANK>(field);
    });
}


template<typename ValueTy>
void register_make_datatype(py::module& m){
    // const auto fn_name = "make_datatype_" + atlas::array::DataType::str<ValueTy>();
    const auto fn_name = atlas::array::DataType::str<ValueTy>();
    m.def(fn_name.c_str(), [](){ return atlas::array::DataType::create<ValueTy>(); });
}


PYBIND11_MODULE(pyatlas, m) {

    m.def("initialise_runtime", []() {
        // Create fake argc/argv for eckit::Main set arg 0 to "embedded_interpreter"
        int argc = 1;
        char* argv[1];
        argv[0] = const_cast<char*>("embedded_interpreter");

        eckit::Main::initialise(argc, argv);
        eckit::Main::instance().taskID(eckit::mpi::comm("world").rank());
        atlas::initialise();
    });

    m.def("finalise_runtime", []() {
        atlas::finalise();
    });

    py::class_<atlas::array::DataType>(m, "DataType")
        .def("__str__", [](const atlas::array::DataType& self) {
            return self.str();
        })
        .def("__repr__", [](const atlas::array::DataType& self) {
            return self.str();
        })
        .def("__eq__", [](const atlas::array::DataType& self, const atlas::array::DataType& other) {
            return self == other;
        })
        .def("__ne__", [](const atlas::array::DataType& self, const atlas::array::DataType& other) {
            return self != other;
        })
        .def("to_numpy", [](const atlas::array::DataType& self) {
            // Map Atlas DataType to NumPy dtype
            if (self == atlas::array::DataType::byte()) {
                return py::dtype::of<std::byte>();
            } else if (self == atlas::array::DataType::int32()) {
                return py::dtype::of<int32_t>();
            } else if (self == atlas::array::DataType::int64()) {
                return py::dtype::of<int64_t>();
            } else if (self == atlas::array::DataType::real32()) {
                return py::dtype::of<float>();
            } else if (self == atlas::array::DataType::real64()) {
                return py::dtype::of<double>();
            } else if (self == atlas::array::DataType::uint64()) {
                return py::dtype::of<uint64_t>();
            } else {
                throw std::runtime_error("Unsupported DataType for NumPy conversion.");
            }
        });

    register_make_datatype<std::byte>(m);
    register_make_datatype<int>(m);
    register_make_datatype<long>(m);
    register_make_datatype<float>(m);
    register_make_datatype<double>(m);
    register_make_datatype<unsigned long>(m);

    py::class_<atlas::Grid>(m, "Grid")
        .def(py::init<const std::string&>())
        .def_property_readonly("name", &atlas::Grid::name)
        .def_property_readonly("type", &atlas::Grid::type)
        .def_property_readonly("uid", &atlas::Grid::uid)
        .def_property_readonly("size", &atlas::Grid::size)
        .def_property_readonly("spec", &atlas::Grid::spec)
        .def("xy", [](const atlas::Grid& self) {
            auto xy_iter = self.xy();
            py::ssize_t num_points = self.size();

            // Create a 2D numpy array with shape (num_points, 2)
            py::ssize_t shape[2] = {num_points, 2};
            py::array_t<double> xy_arr(shape);
            auto xy_view = xy_arr.mutable_unchecked<2>();

            size_t i = 0;
            for (auto point : xy_iter) {
                xy_view(i, 0) = point.x();
                xy_view(i, 1) = point.y();
                ++i;
            }

            return xy_arr;
        });

    auto m_grid = m.def_submodule( "grid" );

    py::class_<atlas::StructuredGrid, atlas::Grid>(m_grid, "StructuredGrid")
        .def(py::init<const std::string&>())
        .def_property_readonly("valid", &atlas::StructuredGrid::valid)
        .def_property_readonly("ny", &atlas::StructuredGrid::ny)
        .def_property_readonly("nx", py::overload_cast<>(&atlas::StructuredGrid::nx, py::const_))
        .def("from_grid", [](const atlas::Grid& other){
            return atlas_dynamic_cast<atlas::StructuredGrid>(other);
        });

    py::class_<atlas::RegularGrid, atlas::StructuredGrid>(m_grid, "RegularGrid")
        .def(py::init<const std::string&>())
        .def_property_readonly("valid", &atlas::RegularGrid::valid)
        .def_property_readonly("nx", &atlas::RegularGrid::nx)
        .def("from_grid", [](const atlas::Grid& other){
            return atlas_dynamic_cast<atlas::RegularGrid>(other);
        });

    py::class_<atlas::RegularLonLatGrid , atlas::RegularGrid>(m_grid, "RegularLonLatGrid")
        .def(py::init<const std::string&>())
        .def_property_readonly("valid", &atlas::RegularGrid::valid)
        .def("from_grid", [](const atlas::Grid& other){
            return atlas_dynamic_cast<atlas::RegularLonLatGrid>(other);
        });

    py::class_<atlas::FunctionSpace>(m, "FunctionSpace")
        .def_property_readonly("size", &atlas::FunctionSpace::size)
        .def_property_readonly("type", &atlas::FunctionSpace::type)
        .def_property_readonly("part", &atlas::FunctionSpace::part)
        .def_property_readonly("nb_parts", &atlas::FunctionSpace::nb_parts)
        .def_property_readonly("lonlat", &atlas::FunctionSpace::lonlat)
        .def_property_readonly("ghost", &atlas::FunctionSpace::ghost)
        .def_property_readonly("global_index", &atlas::FunctionSpace::global_index)
        .def_property_readonly("remote_index", &atlas::FunctionSpace::remote_index)
        .def_property_readonly("partition", &atlas::FunctionSpace::partition)
        .def(
            "create_field",
            [](const atlas::FunctionSpace& self, atlas::array::DataType dtype, const std::optional<std::string>& name, std::optional<int> levels,
                std::optional<int> variables) {
                atlas::util::Config config = atlas::option::datatype(dtype);
                if (name) {
                    config = config | atlas::option::name(*name);
                }
                if (levels) {
                    config = config | atlas::option::levels(*levels);
                }
                if (variables) {
                     config = config | atlas::option::variables(*variables);
                }
                return self.createField(config);
            },
            "dtype"_a, py::kw_only(), "name"_a = std::nullopt, "levels"_a = std::nullopt, "variables"_a = std::nullopt);

    auto m_functionspace = m.def_submodule( "functionspace" );

    py::class_<atlas::functionspace::StructuredColumns, atlas::FunctionSpace>( m_functionspace, "StructuredColumns" )
        .def(py::init( [](const atlas::Grid& grid, int halo ) {
                  return atlas::functionspace::StructuredColumns( grid, atlas::util::Config()( "halo", halo ) );
              } ),
              "grid"_a, py::kw_only(), "halo"_a = 0)
        .def_property_readonly("size", &atlas::functionspace::StructuredColumns::size)
        .def_property_readonly("levels", &atlas::functionspace::StructuredColumns::levels)
        .def_property_readonly("halo", &atlas::functionspace::StructuredColumns::halo)
        .def_property_readonly("valid", &atlas::functionspace::StructuredColumns::valid)
        .def_property_readonly("grid", &atlas::functionspace::StructuredColumns::grid)
        .def("from_functionspace", [](const atlas::FunctionSpace& other) {
            return atlas_dynamic_cast<atlas::functionspace::StructuredColumns>(other);
        });

    py::class_<atlas::array::ArrayShape>(m, "ArrayShape")
        .def("__str__", [](const atlas::array::ArrayShape& self) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < self.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << self[i];
            }
            oss << "]";
            return oss.str();
        })
        .def("__repr__", [](const atlas::array::ArrayShape& self) {
            std::ostringstream oss;
            oss << "ArrayShape(";
            for (size_t i = 0; i < self.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << self[i];
            }
            oss << ")";
            return oss.str();
        });

    m.def("make_array_shape", [](py::tuple& shape) {
        std::vector<int> shape_as_list(shape.size());
        for (std::size_t i = 0; i < shape.size(); ++i) {
            shape_as_list[i] = shape[i].cast<int>();
        }
        return atlas::array::ArrayShape(shape_as_list);
    });

    py::class_<atlas::array::ArrayStrides>(m, "ArrayStrides")
        .def("__str__", [](const atlas::array::ArrayStrides& self) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < self.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << self[i];
            }
            oss << "]";
            return oss.str();
        })
        .def("__repr__", [](const atlas::array::ArrayShape& self) {
            std::ostringstream oss;
            oss << "ArrayStrides(";
            for (size_t i = 0; i < self.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << self[i];
            }
            oss << ")";
            return oss.str();
        });

    py::class_<atlas::Field>(m, "Field")
        .def(py::init<const std::string&, atlas::array::DataType, const atlas::array::ArrayShape&>())
        .def("__str__", [](const atlas::Field& self) {
            std::ostringstream oss;
            if (!self) {
                oss << "<null Field>";
                return oss.str();
            }
            self.dump(oss);
            return oss.str();
        })
        .def("__repr__", [](const atlas::Field& self) {
            std::ostringstream oss;
            if (!self) {
                oss << "<null Field>";
                return oss.str();
            }
            self.dump(oss);
            return oss.str();
        })
        .def_property_readonly("name", &atlas::Field::name)
        .def_property_readonly("rank", &atlas::Field::rank)
        .def_property_readonly("datatype", &atlas::Field::datatype)
        .def_property_readonly("shape", py::overload_cast<>(&atlas::Field::shape, py::const_))
        .def_property_readonly("functionspace", &atlas::Field::functionspace)
        .def("clone", [](const atlas::Field& self) {
            return self.clone(atlas::util::Config());
        });


    // `ndarray_view`: returns a numpy array view of the field.
    m.def("ndarray_view", [](atlas::Field& field) {

        auto& atlas_array = field.array();

        // Check that the atlas array has C-style memory layout and is contiguous
        const auto atlas_spec = atlas_array.spec();
        if(!atlas_spec.hasDefaultLayout() || !atlas_spec.contiguous()) {
            throw std::runtime_error("Unsupported Atlas array layout.");
        }

        const auto atlas_dtype = atlas_array.datatype();

        std::vector<ssize_t> atlas_stride_in_bytes(atlas_array.rank());
        for (size_t i = 0; i < atlas_array.rank(); ++i) {
            atlas_stride_in_bytes[i] = atlas_array.stride(i) * atlas_dtype.size();
        }

        const auto py_format = to_py_buffer_format(atlas_dtype);

        return py::array(
            py::buffer_info(
                atlas_array.storage(), // Pointer to data
                atlas_dtype.size(),    // Size of one scalar
                py_format,             // Python struct-style format descriptor
                atlas_array.rank(),    // Number of dimensions
                atlas_array.shape(),   // Buffer dimensions
                atlas_stride_in_bytes  // Strides (in bytes)
            ),
            py::cast(nullptr)
        );
    });

    m.def("field_view", [](py::array& array, const std::optional<std::string>& name) {
        std::string field_name = "";
        if (name) {
            field_name = *name;
        }

        auto buffer_info = array.request();
        atlas::array::DataType dtype = to_atlas_dtype(buffer_info.format);
        atlas::array::ArrayShape shape;
        for (size_t i = 0; i < static_cast<size_t>(buffer_info.ndim); ++i) {
            shape.push_back(static_cast<int>(buffer_info.shape[i]));
        }
        atlas::array::ArrayStrides strides;
        for (size_t i = 0; i < static_cast<size_t>(buffer_info.ndim); ++i) {
            strides.push_back(static_cast<int>(buffer_info.strides[i] / dtype.size()));
        }
        atlas::array::ArraySpec spec(shape, strides);

        atlas::array::Array* wrapped_array = nullptr;
        if (dtype == atlas::array::DataType::int32()) {
            wrapped_array = atlas::array::Array::wrap(static_cast<int*>(buffer_info.ptr), spec);
        } else if (dtype == atlas::array::DataType::int64()) {
            wrapped_array = atlas::array::Array::wrap(static_cast<long*>(buffer_info.ptr), spec);
        } else if (dtype == atlas::array::DataType::real32()) {
            wrapped_array = atlas::array::Array::wrap(static_cast<float*>(buffer_info.ptr), spec);
        } else if (dtype == atlas::array::DataType::real64()) {
            wrapped_array = atlas::array::Array::wrap(static_cast<double*>(buffer_info.ptr), spec);
        } else if (dtype == atlas::array::DataType::uint64()) {
            wrapped_array = atlas::array::Array::wrap(static_cast<unsigned long*>(buffer_info.ptr), spec);
        } else {
            throw std::runtime_error("Unsupported data type for field view.");
        }
        return atlas::Field(field_name, wrapped_array);
    }, "array"_a, py::kw_only(), "name"_a = std::nullopt);


    auto fieldset_str = [](const atlas::FieldSet& self) {
        std::ostringstream oss;
        if (!self) {
            oss << "<null FieldSet>";
            return oss.str();
        }
        oss << "FieldSet(name=" << self.name() << ", size=" << self.size() << ")";
        for (atlas::idx_t i = 0; i < self.size(); ++i) {
            oss << "\n  " << self[i];
        }
        return oss.str();
    };

    py::class_<atlas::FieldSet>(m, "FieldSet")
        .def(py::init<>())
        .def("__str__", fieldset_str)
        .def("__repr__", fieldset_str)
        .def("size", &atlas::FieldSet::size)
        .def("name", py::overload_cast<>(&atlas::FieldSet::name, py::const_))
        .def("name", py::overload_cast<>(&atlas::FieldSet::name))
        .def("empty", &atlas::FieldSet::empty)
        .def("has", &atlas::FieldSet::has)
        .def("add", py::overload_cast<const atlas::Field&>(&atlas::FieldSet::add))
        .def("__getitem__", (const atlas::Field& (atlas::FieldSet::*)(long) const) &atlas::FieldSet::operator[])
        .def("__getitem__", (atlas::Field& (atlas::FieldSet::*)(long)) &atlas::FieldSet::operator[])
        .def("__getitem__", (const atlas::Field& (atlas::FieldSet::*)(const std::string&) const) &atlas::FieldSet::operator[])
        .def("__getitem__", (atlas::Field& (atlas::FieldSet::*)(const std::string&)) &atlas::FieldSet::operator[])
        .def("field", py::overload_cast<size_t>(&atlas::FieldSet::field<size_t>, py::const_))
        .def("field", py::overload_cast<size_t>(&atlas::FieldSet::field<size_t>))
        .def("field", (atlas::Field& (atlas::FieldSet::*)(const std::string&) const) &atlas::FieldSet::field)
        .def("clone", [](const atlas::FieldSet& self) {
            return self.clone(atlas::util::Config());
        })
        .def("field_names", &atlas::FieldSet::field_names)
        .def("keys", [](const atlas::FieldSet& self) {
            return self.field_names();
        })
        .def("values", [](const atlas::FieldSet& self) {
            std::vector<atlas::Field> fields;
            fields.reserve(self.size());
            for (auto& f : self) {
                fields.push_back(f);
            }
            return fields;
        })
        .def("items", [](const atlas::FieldSet& self) {
            std::vector<std::pair<std::string, atlas::Field>> names_fields;
            names_fields.reserve(self.size());
            for (size_t i = 0; i < self.size(); ++i) {
                auto& f = self[i];
                auto& f_name = f.name();
                names_fields.push_back(std::make_pair(f_name, f));
            }
            return names_fields;
        });

    py::class_<atlas::Interpolation>(m, "Interpolation")
        .def(py::init<const eckit::LocalConfiguration&, const atlas::FunctionSpace&, const atlas::FunctionSpace&>(),
             "config"_a,
             "source_fs"_a,
             "target_fs"_a)
        .def("execute", [](const atlas::Interpolation& self,
                           const atlas::Field& source,
                           atlas::Field& target) {
                return self.execute(source, target);
            },
            "source"_a,
            "target"_a);

    auto m_interpolation = m.def_submodule("interpolation");

    m_interpolation.def("assemble_global_matrix", &assemble_global_matrix
        , "interpolator"_a, "mpi_root"_a = 0);

#if ATLAS_VERSION >= 4100
    py::class_<atlas::linalg::SparseMatrixStorage>(m, "SparseMatrixStorage")
        .def("__str__", [](const atlas::linalg::SparseMatrixStorage& self) {
            std::ostringstream oss;
            auto view = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(self);
            oss << "SparseMatrixStorage(" << view.rows() << "x" << view.cols() << ", nnz=" << view.nnz();
            oss << ")";
            return oss.str();
        })
            .def("__repr__", [](const atlas::linalg::SparseMatrixStorage& self) {
            std::ostringstream oss;
            auto view = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(self);
            oss << "SparseMatrixStorage(" << view.rows() << "x" << view.cols() << ", nnz=" << view.nnz();
            oss << ")";
            return oss.str();
        })
        .def_property_readonly("nrows", &atlas::linalg::SparseMatrixStorage::rows)
        .def_property_readonly("ncols", py::overload_cast<>(&atlas::linalg::SparseMatrixStorage::cols, py::const_))
        .def_property_readonly("nnz", &atlas::linalg::SparseMatrixStorage::nnz)
        .def_property_readonly("outer", [](const atlas::linalg::SparseMatrixStorage& self) {
            auto view = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(self);
            std::vector<size_t> outer(view.rows() + 1);
            const auto* outer_ptr = view.outer();
            for (size_t i = 0; i < outer.size(); ++i) {
                outer[i] = outer_ptr[i];
            }
            return outer;
        })
        .def_property_readonly("inner", [](const atlas::linalg::SparseMatrixStorage& self) {
            auto view = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(self);
            std::vector<size_t> inner(view.nnz());
            const auto* inner_ptr = view.inner();
            for (size_t i = 0; i < inner.size(); ++i) {
                inner[i] = inner_ptr[i];
            }
            return inner;
        })
        .def_property_readonly("value", [](const atlas::linalg::SparseMatrixStorage& self) {
            auto view = atlas::linalg::make_host_view<eckit::linalg::Scalar,eckit::linalg::Index>(self);
            std::vector<double> data(view.nnz());
            const auto* data_ptr = view.value();
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = data_ptr[i];
            }
            return data;
        });
#else
    py::class_<eckit::linalg::SparseMatrix>(m, "SparseMatrixStorage")
        .def("__str__", [](const eckit::linalg::SparseMatrix& self) {
            std::ostringstream oss;
            self.dump(oss);
            return oss.str();
        })
        .def("__repr__", [](const eckit::linalg::SparseMatrix& self) {
            std::ostringstream oss;
            self.dump(oss);
            return oss.str();
        })
        .def_property_readonly("nrows", &eckit::linalg::SparseMatrix::rows)
        .def_property_readonly("ncols", py::overload_cast<>(&eckit::linalg::SparseMatrix::cols, py::const_))
        .def_property_readonly("nnz", &eckit::linalg::SparseMatrix::nonZeros)
        .def_property_readonly("outer", [](const eckit::linalg::SparseMatrix& self) {
            std::vector<size_t> outer(self.rows() + 1);
            const auto* outer_ptr = self.outer();
            for (size_t i = 0; i < outer.size(); ++i) {
                outer[i] = outer_ptr[i];
            }
            return outer;
        })
        .def_property_readonly("inner", [](const eckit::linalg::SparseMatrix& self) {
            std::vector<size_t> inner(self.nonZeros());
            const auto* inner_ptr = self.inner();
            for (size_t i = 0; i < inner.size(); ++i) {
                inner[i] = inner_ptr[i];
            }
            return inner;
        })
        .def_property_readonly("value", [](const eckit::linalg::SparseMatrix& self) {
            std::vector<double> data(self.nonZeros());
            const auto* data_ptr = self.data();
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = data_ptr[i];
            }
            return data;
        });
#endif

    py::class_<eckit::LocalConfiguration>(m, "LocalConfiguration")
        .def(py::init())
        .def( "__setitem__",
            [](eckit::LocalConfiguration& self, const std::string& key, py::object value) {
                if (py::isinstance<py::bool_>(value)) {
                    self.set(key, value.cast<bool>());
                } else if (py::isinstance<py::int_>(value)) {
                    self.set(key, value.cast<long long>());
                } else if (py::isinstance<py::float_>(value)) {
                    self.set(key, value.cast<double>());
                } else if (py::isinstance<py::str>(value)) {
                    self.set(key, value.cast<std::string>());
                } else {
                    throw std::out_of_range( "value argument with unsupported type" );
                }
            })
        .def("__getitem__",
            [](eckit::LocalConfiguration& self, const std::string& key) -> py::object {
                if (!self.has( key )) {
                    throw std::out_of_range("config does not contain key=" + key);
                }
                if (self.isBoolean(key)) {
                    return py::bool_(self.getBool(key));
                } else if (self.isIntegral(key)) {
                    return py::int_(self.getInt64(key));
                } else if (self.isFloatingPoint(key)) {
                    return py::float_(self.getDouble(key));
                } else if (self.isString(key)) {
                    return py::str(self.getString( key ));
                } else {
                    throw std::out_of_range("value stored with unsupported type");
                }
              })
        .def("__delitem__",
            [](eckit::LocalConfiguration& self, const std::string& key) {
                if (!self.has(key)) {
                    throw std::out_of_range("config does not contain key=" + key);
                }
                self.remove(key);
            })
        .def("__contains__", &eckit::LocalConfiguration::has)
        .def("__len__", [](const eckit::LocalConfiguration& self) {
            return self.keys().size();
        })
        .def("__str__", [](const eckit::LocalConfiguration& self) {
            std::ostringstream oss;
            oss << self;
            return oss.str();
        })
        .def("__repr__", [](const eckit::LocalConfiguration& self) {
            std::ostringstream oss;
            oss << self;
            return oss.str();
        })
        .def("keys", &eckit::LocalConfiguration::keys)
        .def("values", [](const eckit::LocalConfiguration& self) {
            py::list values;
            for (const auto& key : self.keys()) {
                py::object value;
                if (self.isBoolean(key)) {
                    value = py::bool_(self.getBool(key));
                } else if (self.isIntegral(key)) {
                    value = py::int_(self.getInt64(key));
                } else if (self.isFloatingPoint(key)) {
                    value = py::float_(self.getDouble(key));
                } else if (self.isString(key)) {
                    value = py::str(self.getString(key));
                } else {
                    throw std::out_of_range("value stored with unsupported type");
                }
                values.append(value);
            }
            return values;
        })
        .def("items", [](const eckit::LocalConfiguration& self) {
            py::list items;
            for (const auto& key : self.keys()) {
                py::object value;
                if (self.isBoolean(key)) {
                    value = py::bool_(self.getBool(key));
                } else if (self.isIntegral(key)) {
                    value = py::int_(self.getInt64(key));
                } else if (self.isFloatingPoint(key)) {
                    value = py::float_(self.getDouble(key));
                } else if (self.isString(key)) {
                    value = py::str(self.getString(key));
                } else {
                    throw std::out_of_range("value stored with unsupported type");
                }
                items.append(py::make_tuple(key, value));
            }
            return items;
        });

    py::class_<atlas::util::Config, eckit::LocalConfiguration>( m, "Config" )
        .def(py::init());

    py::class_<atlas::util::Metadata, eckit::LocalConfiguration>( m, "Metadata" )
        .def(py::init());


    // register_array_view<std::byte, 1>(m);
    register_array_view<int, 1>(m);
    register_array_view<long, 1>(m);
    register_array_view<float, 1>(m);
    register_array_view<double, 1>(m);
    register_array_view<unsigned long, 1>(m);


    // register_array_view<std::byte, 2>(m);
    register_array_view<int, 2>(m);
    register_array_view<long, 2>(m);
    register_array_view<float, 2>(m);
    register_array_view<double, 2>(m);
    register_array_view<unsigned long, 2>(m);


    // register_array_view<std::byte, 3>(m);
    register_array_view<int, 3>(m);
    register_array_view<long, 3>(m);
    register_array_view<float, 3>(m);
    register_array_view<double, 3>(m);
    register_array_view<unsigned long, 3>(m);


    // register_array_view<std::byte, 4>(m);
    register_array_view<int, 4>(m);
    register_array_view<long, 4>(m);
    register_array_view<float, 4>(m);
    register_array_view<double, 4>(m);
    register_array_view<unsigned long, 4>(m);
}
