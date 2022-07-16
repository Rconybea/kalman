# process/CMakeLists.txt

# build shared library 'process'
add_library(simulator SHARED Simulator.cpp)

set_target_properties(simulator PROPERTIES
		      VERSION ${PROJECT_VERSION}
		      SOVERSION 1
		      PUBLIC_HEADER Simulator.hpp)

# ----------------------------------------------------------------
# all the errors+warnings!
#
target_compile_options(simulator PRIVATE -Werror -Wall -Wextra)

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
target_include_directories(simulator PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

target_link_libraries(simulator PUBLIC reactor)
target_link_libraries(simulator PUBLIC logutil)
target_link_libraries(simulator PUBLIC refcnt)

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

target_include_directories(simulator PUBLIC
 		           ${Boost_INCLUDE_DIR})

# end CMakeLists.txt

