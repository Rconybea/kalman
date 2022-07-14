# filter/CMakeLists.txt

set(SELF_LIBRARY_NAME filter)

# build shared liburary 'filter'
add_library(${SELF_LIBRARY_NAME} SHARED KalmanFilterSvc.cpp KalmanFilter.cpp)

set_target_properties(${SELF_LIBRARY_NAME}
		      PROPERTIES
		      VERSION ${PROJECT_VERSION}
		      SOVERSION 1
		      PUBLIC_HEADER BlackScholes.hpp)

# ----------------------------------------------------------------
# all the warnings!
#
target_compile_options(${SELF_LIBRARY_NAME} PRIVATE -Werror -Wall -Wextra)

# ----------------------------------------------------------------
# PROJECT_SOURCE_DIR:
#   so we can for example write
#     #include "logutil/scope.hpp"
#   from anywhere in the project
# PROJECT_BINARY_DIR:
#   since version file will be in build directory,  need that directory
#   to also be included in compiler's include path
#
target_include_directories(${SELF_LIBRARY_NAME}
                           PUBLIC
                           ${PROJECT_SOURCE_DIR}
			   ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

#target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC process)
target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC logutil)
target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC refcnt)

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

target_include_directories(${SELF_LIBRARY_NAME} PUBLIC
			   ${EIGEN_INCLUDE_DIRS})
target_compile_options(${SELF_LIBRARY_NAME} PUBLIC ${EIGEN_CFLAGS_OTHER})

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


