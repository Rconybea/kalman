# refcnt/CMakeLists.txt

# ----------------------------------------------------------------
# 3rd party dependency: boost:

# boost dependency -- specified by cmake
#
find_package(Boost REQUIRED)

target_include_directories(process PUBLIC
 		           ${Boost_INCLUDE_DIR})

# end CMakeLists.txt

