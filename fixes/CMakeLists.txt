project(salfixes)

set(salfixes_SRCS
    datamodel.cpp
    runnermodel.cpp
    salfixesplugin.cpp
    )

qt4_automoc(${salfixes_SRCS})

kde4_add_library(salfixesplugin SHARED ${salfixes_SRCS})

target_link_libraries(salfixesplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        )

install(TARGETS salfixesplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/sal/fixes)
install(FILES qmldir
    Label.qml
    DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/sal/fixes)