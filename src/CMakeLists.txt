cmake_minimum_required(VERSION 3.10)

# ----------------------------------------------------------------
# set the project name.

# NOTE: this needs to come before c++ compiler settings

project(kalman VERSION 0.1)
enable_language(CXX)
enable_testing()

# ----------------------------------------------------------------
# c++ compiler

# introduce a variable to hold c++ compiler flags
SET(PROJECT_CXX_FLAGS "--std=c++20")

# adds to {c,c++} compiler flags
add_definitions(${PROJECT_CXX_FLAGS})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

#message ("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
#message ("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
#message ("CMAKE_COMPILER_IS_GNUCXX=${CMAKE_COMPILER_IS_GNUCXX}")

# ----------------------------------------------------------------

# ability to configure dependencies given in pkg-config (.pc) files
include(FindPkgConfig)
include(cmake/code-coverage.cmake)
include(cmake/symlink.cmake)

# propagate version info to c++
configure_file(KalmanConfig.h.in KalmanConfig.h)

# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON ?)
add_code_coverage()

# add process/, logutil/, .. subdirectory
add_subdirectory(simulator)
add_subdirectory(simulator_py)
add_subdirectory(filter)
add_subdirectory(process)
add_subdirectory(option)
add_subdirectory(option_util)
add_subdirectory(distribution)
add_subdirectory(distribution_py)
add_subdirectory(random)
add_subdirectory(queue)
add_subdirectory(refcnt)
add_subdirectory(logutil)

# add unittest subdirectories
add_subdirectory(filter/utest)
add_subdirectory(process/utest)
add_subdirectory(option/utest)
add_subdirectory(tree/utest)
add_subdirectory(refcnt/utest)

# add the executable
add_executable(example junk.cpp)

# ----------------------------------------------------------------
# generic project dependency

# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since version file will be in build directory,  need that directory
#   to also be included in compiler's include path
#
target_include_directories(example PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: process, logutil, ..

target_link_libraries(example PUBLIC process)
target_link_libraries(example PUBLIC logutil)

# ----------------------------------------------------------------
# 3rd party dependency: eigen:

# eigen dependency -- specified by .pc file
#   pkg_check_modules(<prefix>
#                     [REQUIRED] [QUIET]
#                     [NO_CMAKE_PATH]
#                     [NO_CMAKE_ENVIRONMENT_PATH]
#                     [IMPORTED_TARGET [GLOBAL]]
#                     <moduleSpec> [<moduleSpec>...])
# see:
#   https://cmake.org/cmake/help/latest/module/FindPkgConfig.html
#
pkg_check_modules(EIGEN REQUIRED eigen3)

target_include_directories(example PUBLIC
			   ${EIGEN_INCLUDE_DIRS})
target_compile_options(example PUBLIC ${EIGEN_CFLAGS_OTHER})

# ----------------------------------------------------------------
# 3rd party dependency: boost:

# boost dependency -- specified by cmake
#
find_package(Boost REQUIRED)

target_include_directories(example PUBLIC
 		           ${Boost_INCLUDE_DIR})

# ----------------------------------------------------------------
# 3rd party dependency: catch2:
# (adding this here temporarily so we can observe effect on kalman/build/CmakeCache.txt)
#
# find_package(Catch2 2 REQUIRED)

# ----------------------------------------------------------------
# make standard directories for std:: includes explicit
# so that
# (1) they appear in compile_commands.json.
# (2) clangd (run from emacs lsp-mode) can find them
#
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# end CMakeLists.txt
