project(components)

set(components_SRCS
    ${CMAKE_SOURCE_DIR}/internal/aboutdata.cpp
    abstractsourceregistry.cpp
    componentsplugin.cpp
    dirconfigurationwidget.cpp
    dirmodel.cpp
    favoriteappsmodel.cpp
    favoriteplacesmodel.cpp
    groupedinstalledappsmodel.cpp
    helpmenuactions.cpp
    icondialog.cpp
    installedappsconfigurationwidget.cpp
    installedappsmodel.cpp
    powermodel.cpp
    runnerconfigurationwidget.cpp
    runnermodel.cpp
    sessionmodel.cpp
    shadoweffect.cpp
    sourceconfigurationdialog.cpp
    sourceid.cpp
    sourcemodel.cpp
    sourceregistry.cpp
    tabmodel.cpp
    )

kde4_add_ui_files(components_SRCS
    dirconfigurationwidget.ui
    installedappsconfigurationwidget.ui
    runnerconfigurationwidget.ui
    )

qt4_automoc(${components_SRCS})

include_directories(
    ${lib_BINARY_DIR}
    ${lib_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/internal
    )

kde4_add_library(componentsplugin SHARED ${components_SRCS})

target_link_libraries(componentsplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4WORKSPACE_KWORKSPACE_LIBS}
        homerun
        )

install(TARGETS componentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components)
install(DIRECTORY qml/
    DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components
    PATTERN "*.qml"
    PATTERN "qmldir"
    PATTERN "*.js"
    )
