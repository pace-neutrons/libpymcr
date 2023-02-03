cmake_minimum_required (VERSION 3.13)
# Need 3.13 to find Matlab engine and data array libraries

project("libpymcr" VERSION "0.1.0")

set(CMAKE_MACOSX_RPATH TRUE)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED 11)

set(MINIMUM_PYBIND11_VERSION 2.5.0)
set(FETCH_PYBIND11_REPO https://github.com/pybind/pybind11)

if (PYTHON_EXECUTABLE)
  # Ensure the provided Python interpreter is used
  set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
endif()
find_package(Python3 COMPONENTS Interpreter Development)

# Use system pybind11 if it exists
find_package(pybind11 ${MINIMUM_PYBIND11_VERSION} QUIET)
if (pybind11_FOUND)
  message(STATUS "pybind11 >= ${MINIMUM_PYBIND11_VERSION} found")
else()
  include(FetchContent)
  if (EXISTS "${FETCHCONTENT_BASE_DIR}/pybind11-src")
    set(pybind11_POPULATED ON)
    set(pybind11_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/pybind11-src")
    set(pybind11_BINARY_DIR "${FETCHCONTENT_BASE_DIR}/pybind11-build")
    message("Found pre-fetched pybind11")
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
  else()
    message(STATUS "Fetching pybind11 v${MINIMUM_PYBIND11_VERSION} from ${FETCH_PYBIND11_REPO}")
    FetchContent_Declare(pybind11 GIT_REPOSITORY ${FETCH_PYBIND11_REPO} GIT_TAG v${MINIMUM_PYBIND11_VERSION} GIT_SHALLOW 1)
    FetchContent_GetProperties(pybind11)
    if(NOT pybind11_POPULATED)
      FetchContent_Populate(pybind11)
      add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
    endif()
  endif()
  set(pybind11_FOUND ON)
endif()

# You can set a specific Matlab version folder here
#set(Matlab_ROOT_DIR "c:/Program Files/MATLAB/R2020a")
set(MATLAB_FIND_DEBUG 1)
find_package(Matlab)

# On MacOS github runners it can sometimes fail to find the right libraries
if (NOT Matlab_HAS_CPP_API)
  if (NOT Matlab_ENGINE_LIBRARY)
    file(GLOB_RECURSE Matlab_ENGINE_LIBRARY "${Matlab_ROOT_DIR}/extern/bin/*/libMatlabEngine*")
  endif()
  if (NOT Matlab_DATAARRAY_LIBRARY)
    file(GLOB_RECURSE Matlab_DATAARRAY_LIBRARY "${Matlab_ROOT_DIR}/extern/bin/*/libMatlabDataArray*")
  endif()
  if (Matlab_ENGINE_LIBRARY AND Matlab_DATAARRAY_LIBRARY)
    set(Matlab_HAS_CPP_API 1)
    list(APPEND Matlab_LIBRARIES ${Matlab_ENGINE_LIBRARY})
    list(APPEND Matlab_LIBRARIES ${Matlab_DATAARRAY_LIBRARY})
  endif()
endif()

add_subdirectory(src)
