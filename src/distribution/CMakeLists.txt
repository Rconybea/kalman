# distribution/CMakeLists.txt

# build library 'process'
add_library(distribution Normal.cpp)

set_target_properties(distribution PROPERTIES
				   VERSION ${PROJECT_VERSION}
				   SOVERSION 1)

# ----------------------------------------------------------------
# all the warnings!
#
target_compile_options(distribution PRIVATE -Werror -Wall -Wextra)

# ----------------------------------------------------------------
# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since version file will be in build directory,  need that directory
#   to also be included in compiler's include path
#
target_include_directories(distribution PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

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

