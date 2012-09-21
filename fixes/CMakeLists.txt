project(fixes)

set(fixes_SRCS
    datamodel.cpp
    fixesplugin.cpp
    )

qt4_automoc(${fixes_SRCS})

kde4_add_library(fixesplugin SHARED ${fixes_SRCS})

target_link_libraries(fixesplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        )

install(TARGETS fixesplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/fixes)
install(DIRECTORY qml/
    DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/fixes
    PATTERN "*.qml"
    PATTERN "qmldir"
    PATTERN "*.js"
    )
