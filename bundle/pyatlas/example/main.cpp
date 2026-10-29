#include <iostream>
#include <ostream>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <atlas/field.h>

namespace py = pybind11;

// Note: to run this you need to ensure that the directories
// containing the pyatlas lib and example.py in your PYTHONPATH.

int main() {
    // To setup the interpreter you can either:
    // A) create the `py::scoped_interpreter guard{};` object
    //    and ensure that its lifetime covers all uses of
    //    the interpreter.
    // B) orchestrate `py::initialize_interpreter()` and 
    //    `py::finalize_interpreter()` calls manually.
    // The second option can be better in cases where you might
    // choose to not call the finalise routine to avoind order
    // of destruction issues.

    py::scoped_interpreter guard{};

    // Build a fieldset with three small fields: A and B hold input
    // values, C is zeroed and will be filled in on the Python side.
    const atlas::idx_t n = 5;
    atlas::FieldSet fs{};
    for (const auto& name : {"A", "B", "C"}) {
        fs.add(atlas::Field(
            name,
            atlas::array::make_datatype<double>(),
            atlas::array::make_shape(n))
        );
    }

    auto a = atlas::array::make_view<double, 1>(fs["A"]);
    auto b = atlas::array::make_view<double, 1>(fs["B"]);
    auto c = atlas::array::make_view<double, 1>(fs["C"]);
    for (atlas::idx_t i = 0; i < a.shape(0); ++i) {
        a(i) = i;
        b(i) = 10 * i;
        c(i) = 0;
    }

    auto example_mod = pybind11::module_::import("example");

    // The Python FieldSet shares the same underlying fields, so
    // writes made in Python are visible here afterwards.
    example_mod.attr("add_fields")(fs);

    std::cout << "C (from C++):";
    for (atlas::idx_t i = 0; i < c.shape(0); ++i) {
        std::cout << " " << c(i);
    }
    std::cout << std::endl;
}