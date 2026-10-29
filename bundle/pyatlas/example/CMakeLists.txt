ecbuild_add_executable(
    TARGET example
    SOURCES main.cpp
    LIBS pybind11::embed atlas
)
