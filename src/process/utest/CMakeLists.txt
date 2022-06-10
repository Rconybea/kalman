# build unittest 'simulator/unittest'

# These tests can use the Catch2-provided main
file(GLOB SOURCE_FILES "*.cpp")
add_executable(utest.process ${SOURCE_FILES})
#target_link_libraries(utest.process PRIVATE Catch2::Catch2WithMain)

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
target_include_directories(utest.process PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

target_link_libraries(utest.process PUBLIC process)
target_link_libraries(utest.process PUBLIC logutil)

# ----------------------------------------------------------------
# 3rd part dependency: catch2:

find_package(Catch2 2 REQUIRED)

# need this so that catch2/include appears in compile_commands.json,
# on which lsp integration relies.
#
# See also /nix/store/*-catch2-*/lib/cmake/Catch2/ParseAndAddCatchTests.cmake;
# commands here derived from ^ .cmake file
#
find_path(CATCH_INCLUDE_DIR "catch2/catch.hpp")
target_include_directories(utest.process PUBLIC ${CATCH_INCLUDE_DIR})

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
