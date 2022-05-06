cmake_minimum_required(VERSION 3.10)

# ability to configure dependencies gives in pkg-config (.pc) files
include(FindPkgConfig)

# set the project name
project(kalman VERSION 0.1)

# propagate version info to c++
configure_file(KalmanConfig.h.in KalmanConfig.h)

# add the executable
add_executable(example matrix/junk.cpp)

# since version file will be in build directory,  need that directory
# to also be included in compiler's include path
#
target_include_directories(example PUBLIC ${PROJECT_BINARY_DIR} ${EIGEN_INCLUDE_DIRS})
target_compile_options(example PUBLIC ${EIGEN_CFLAGS_OTHER})

#target_link_libraries(testapp ${SDL2_LIBRARIES})
#target_compile_options(testapp PUBLIC ${SDL2_CFLAGS_OTHE})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# make standard directories for std:: includes explicit
# so that
# (1) they appear in compile_commands.json.
# (2) clangd (run from emacs lsp-mode) can find them
#
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

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

# end CMakeLists.txt
