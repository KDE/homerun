macro(sal_add_unit_test _test)
    set(_src ${_test}.cpp ${ARGN})
    qt4_automoc(${_src})
    kde4_add_unit_test(${_test} ${_src})
    target_link_libraries(${_test}
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

sal_add_unit_test(pagemodeltest ${salcomponents_SOURCE_DIR}/pagemodel.cpp)
