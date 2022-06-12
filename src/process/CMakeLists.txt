# process/CMakeLists.txt

# build shared library 'process'
add_library(process SHARED BrownianMotion.cpp Realization.cpp)

set_target_properties(process PROPERTIES
 			      VERSION ${PROJECT_VERSION}
 			      SOVERSION 1
 			      PUBLIC_HEADER BrownianMotion.hpp)

# ----------------------------------------------------------------
# all the warnings!
#
target_compile_options(process PRIVATE -Werror -Wall -Wextra)

# ----------------------------------------------------------------
# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since version file will be in build directory,  need that directory
#   to also be included in compiler's include path
#
target_include_directories(process PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

target_link_libraries(process PUBLIC simulator)

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

target_include_directories(process PUBLIC
 		           ${Boost_INCLUDE_DIR})

# end CMakeLists.txt

