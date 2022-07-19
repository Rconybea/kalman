# filter_py/CMakeLists.txt

set(SELF_LIBRARY_NAME filter_py)

# build python module filter_py

# ----------------------------------------------------------------
# 3rd party dependency: pybind11:

# find_package(Python..) finds python in
#   /Library/Frameworks/Python.framework/...
# but we want to use python from nix
#
#find_package(Python COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11)
pybind11_add_module(${SELF_LIBRARY_NAME} MODULE FilterPy.cpp)

# ----------------------------------------------------------------
# all the errors+warnings!
#
target_compile_options(${SELF_LIBRARY_NAME} PRIVATE -Werror -Wall -Wextra)

# ----------------------------------------------------------------
# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since generated version file will be in build directory,
#   need that build directory to also appear in
#   compiler's include path
#
target_include_directories(${SELF_LIBRARY_NAME} PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: logutils, ...

target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC option)

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

# ----------------------------------------------------------------
# 3rd party dependency: boost:

# boost dependency -- specified by cmake
#
find_package(Boost REQUIRED)

target_include_directories(${SELF_LIBRARY_NAME} PUBLIC
 		           ${Boost_INCLUDE_DIR})

# end CMakeLists.txt

