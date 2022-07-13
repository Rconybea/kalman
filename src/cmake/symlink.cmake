# create symlink in build directory
#
# first use case is to provide symlinks to canned regression baseline files
# used by unit tests.
#
# for example [filter/utest/utest.filter] writes a file [kalman-revert1.txt] to cwd.
# when invoking tests like this:
#   $ cd kalman/build
#   $ ctest
# cwd will be [kalman/build/filter/utest], so output will be in
#   [kalman/build/filter/utest/kalman-revert1.txt]
# want to compare this with [kalman/utestdata/filter/kalman-revert1-baseline.txt]
# and for convenience have build create symlink
#   kalman/build/filter/utest/data/ -symlink-> kalman/utestdata/filter/

macro(create_symlink target linkname)
    execute_process(
        COMMAND ln -sf "${target}" "${linkname}" 
        RESULT_VARIABLE LINK_STATUS
        ERROR_VARIABLE LINK_ERROR
    )
    if(NOT "${LINK_STATUS}" EQUAL 0)
        message(FATAL_ERROR "Create symlink failed:\n${LINK_ERROR}")
    endif()
endmacro()

