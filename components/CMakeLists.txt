project(components)

set(components_SRCS
    componentsplugin.cpp
    tabmodel.cpp
    runnerinfoprovider.cpp
    sharedconfig.cpp
    )

qt4_automoc(${components_SRCS})

include_directories(${lib_SOURCE_DIR})

kde4_add_library(componentsplugin SHARED ${components_SRCS})

target_link_libraries(componentsplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        homerun
        )

install(TARGETS componentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components)
