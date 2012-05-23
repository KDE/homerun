add_subdirectory(auto)

set(_check_command ${CMAKE_CTEST_COMMAND} --verbose)
add_custom_target(check COMMAND
    if [ -n \"$$DISPLAY\" ] \; then ${_check_command} \; else ${_check_command} --exclude-regex '_x11' \; fi
    )
add_dependencies(check buildtests)
