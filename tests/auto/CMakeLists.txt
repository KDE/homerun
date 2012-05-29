macro(sal_add_unit_test _test)
    string(REGEX REPLACE "_x11$" "" _cpp "${_test}") # ${cpp} is ${_test} without the "_x11" prefix, if present
    set(_src ${_cpp}.cpp ${ARGN})
    qt4_automoc(${_src})
    kde4_add_unit_test(${_test} ${_src})
    target_link_libraries(${_test}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDECORE_LIBS}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        )
endmacro(sal_add_unit_test)

include_directories(
    ${salcomponents_SOURCE_DIR}
    )

# Tests which depend on X11 cannot be run on headless builders. If X11 is not
# running, "make check" will not run X11-dependent tests. 
# For "make check" to distinguish X11-dependent tests from X11-independent
# tests, binaries of X11-dependent tests must be suffixed with "_x11".

# X11-independent tests
sal_add_unit_test(pagemodeltest ${salcomponents_SOURCE_DIR}/pagemodel.cpp)

# X11-dependent tests
sal_add_unit_test(favoritemodeltest_x11 ${salcomponents_SOURCE_DIR}/favoritemodel.cpp)
sal_add_unit_test(runnerinfoprovidertest_x11 ${salcomponents_SOURCE_DIR}/runnerinfoprovider.cpp) # Depends on having a running klauncher, which needs $DISPLAY

# Definition of the "check" target
set(_check_command ${CMAKE_CTEST_COMMAND} --verbose)
add_custom_target(check COMMAND
    if [ -n \"$$DISPLAY\" ] \; then ${_check_command} \; else ${_check_command} --exclude-regex '_x11' \; fi
    )
add_dependencies(check buildtests)
