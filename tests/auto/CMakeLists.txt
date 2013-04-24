macro(homerun_add_unit_test _test)
    string(REGEX REPLACE "_x11$" "" _cpp "${_test}") # ${cpp} is ${_test} without the "_x11" prefix, if present

    # Put all .ui files in _ui, put the rest in _src
    set(_src ${_cpp}.cpp)
    set(_ui "")
    foreach(_name ${ARGN})
        if(${_name} MATCHES "\\.ui$")
            set(_ui ${_ui} ${_name})
        else()
            set(_src ${_src} ${_name})
        endif()
    endforeach()

    if(_ui)
        kde4_add_ui_files(_src ${_ui})
    endif()
    qt4_automoc(${_src})
    kde4_add_unit_test(${_test} ${_src})
    target_link_libraries(${_test}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDECORE_LIBS}
        ${KDE4_SOLID_LIBS}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${LIBKONQ_LIBRARY}
        )
endmacro(homerun_add_unit_test)

include_directories(
    ${components_SOURCE_DIR}
    ${components_SOURCE_DIR}/sources/favorites
    ${components_SOURCE_DIR}/sources/dir
    ${CMAKE_SOURCE_DIR}/internal
    ${lib_SOURCE_DIR}
    ${lib_BINARY_DIR}
    ${LIBKONQ_INCLUDE_DIR}
    )

# Tests which depend on X11 cannot be run on headless builders. If X11 is not
# running, "make check" will not run X11-dependent tests. 
# For "make check" to distinguish X11-dependent tests from X11-independent
# tests, binaries of X11-dependent tests must be suffixed with "_x11".

# X11-independent tests
homerun_add_unit_test(tabmodeltest
    ${components_SOURCE_DIR}/tabmodel.cpp
    ${components_SOURCE_DIR}/sourcemodel.cpp
    ${components_SOURCE_DIR}/abstractsourceregistry.cpp
    )

homerun_add_unit_test(sourcemodeltest
    ${components_SOURCE_DIR}/sourcemodel.cpp
    ${components_SOURCE_DIR}/abstractsourceregistry.cpp
    )

homerun_add_unit_test(i18nconfigtest)

# X11-dependent tests
homerun_add_unit_test(favoriteappsmodeltest_x11
    ${components_SOURCE_DIR}/sources/favorites/favoriteappsmodel.cpp
    )

homerun_add_unit_test(favoriteplacesmodeltest_x11
    ${components_SOURCE_DIR}/sources/dir/dirconfigurationwidget.cpp
    ${components_SOURCE_DIR}/sources/dir/dirconfigurationwidget.ui
    ${components_SOURCE_DIR}/sources/dir/dirmodel.cpp
    ${components_SOURCE_DIR}/sources/favorites/favoriteplacesmodel.cpp
    ${components_SOURCE_DIR}/sources/favorites/favoriteutils.cpp
    ${components_SOURCE_DIR}/sources/favorites/fileplacesmodel.cpp
    ${components_SOURCE_DIR}/sources/favorites/kfileplacesitem.cpp
    ${components_SOURCE_DIR}/sources/favorites/kfileplacessharedbookmarks.cpp
    ${lib_SOURCE_DIR}/abstractsource.cpp
    ${lib_SOURCE_DIR}/actionlist.cpp
    ${lib_SOURCE_DIR}/pathmodel.cpp
    ${lib_SOURCE_DIR}/sourceconfigurationwidget.cpp
    )
homerun_add_unit_test(dirmodeltest_x11
    ${components_SOURCE_DIR}/sources/dir/dirconfigurationwidget.cpp
    ${components_SOURCE_DIR}/sources/dir/dirconfigurationwidget.ui
    ${components_SOURCE_DIR}/sources/dir/dirmodel.cpp
    ${components_SOURCE_DIR}/sources/favorites/favoriteutils.cpp
    ${lib_SOURCE_DIR}/abstractsource.cpp
    ${lib_SOURCE_DIR}/actionlist.cpp
    ${lib_SOURCE_DIR}/pathmodel.cpp
    ${lib_SOURCE_DIR}/sourceconfigurationwidget.cpp
    )

# Definition of the "check" target
set(_check_command ${CMAKE_CTEST_COMMAND} --verbose)
add_custom_target(check COMMAND
    if [ -n \"$$DISPLAY\" ] \; then ${_check_command} \; else ${_check_command} --exclude-regex '_x11' \; fi
    )
add_dependencies(check buildtests)
