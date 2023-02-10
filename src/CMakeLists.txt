#add_library(type_converter OBJECT type_converter.cpp)
#target_include_directories(type_converter PRIVATE ${Matlab_INCLUDE_DIRS})
#target_include_directories(type_converter PRIVATE ${PYTHON_INCLUDE_DIRS})
#target_include_directories(type_converter PRIVATE "${pybind11_SOURCE_DIR}/include")
#set_property(TARGET type_converter PROPERTY POSITION_INDEPENDENT_CODE ON)

pybind11_add_module(_libpymcr libpymcr.cpp type_converter.cpp load_matlab.cpp)
target_include_directories(_libpymcr PRIVATE ${Matlab_INCLUDE_DIRS})
#target_link_libraries(_libpymcr PUBLIC type_converter)

matlab_add_mex(
    NAME call_python
    SRC call_python.cpp type_converter.cpp
)
target_include_directories(call_python PUBLIC ${PYTHON_INCLUDE_DIRS})
target_include_directories(call_python PUBLIC "${pybind11_SOURCE_DIR}/include")
#target_link_libraries(call_python type_converter)
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(call_python ${PYTHON_LIBRARIES})
endif()
# This is set automatically by setup.py but we don't want call_python.mex in the wheel
set_property(TARGET call_python PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
