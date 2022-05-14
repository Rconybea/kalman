cmake_minimum_required(VERSION 3.10)

# introduce a variable to hold c++ compiler flags
SET(PROJECT_CXX_FLAGS "--std=c++20")

# adds to {c,c++} compiler flags
add_definitions(${PROJECT_CXX_FLAGS})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ability to configure dependencies given in pkg-config (.pc) files
include(FindPkgConfig)

# set the project name
project(kalman VERSION 0.1)

# propagate version info to c++
configure_file(KalmanConfig.h.in KalmanConfig.h)

# add process/, logutil/ subdirectory
add_subdirectory(process)
add_subdirectory(distribution)
add_subdirectory(random)
add_subdirectory(logutil)

# add the executable
add_executable(example junk.cpp)

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
# internal dependency: logutil
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
