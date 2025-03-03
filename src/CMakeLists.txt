pybind11_add_module(_libpymcr libpymcr.cpp type_converter.cpp load_matlab.cpp)
target_include_directories(_libpymcr PRIVATE ${Matlab_INCLUDE_DIRS})

mcr_add_mex(
    NAME call_python
    SRC call_python.cpp load_matlab.cpp
)
# Explicitly does not link with Matlab libraries (we will call them with dlopen etc)
set_property(TARGET call_python PROPERTY LINK_LIBRARIES "")
# This is set automatically by setup.py but we don't want call_python.mex in the wheel
set_property(TARGET call_python PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
