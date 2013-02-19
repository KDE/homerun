project(components)

set(components_SRCS
    ${CMAKE_SOURCE_DIR}/internal/aboutdata.cpp
    abstractsourceregistry.cpp
    componentsplugin.cpp
    dirconfigurationwidget.cpp
    dirmodel.cpp
    favoriteappsmodel.cpp
    favoriteplacesmodel.cpp
    helpmenuactions.cpp
    icondialog.cpp
    powermodel.cpp
    sessionmodel.cpp
    shadoweffect.cpp
    sourceconfigurationdialog.cpp
    sourceid.cpp
    sourcemodel.cpp
    sourceregistry.cpp
    sources/installedapps/groupedinstalledappsmodel.cpp
    sources/installedapps/installedappsconfigurationwidget.cpp
    sources/installedapps/installedappsmodel.cpp
    sources/runners/querymatchmodel.cpp
    sources/runners/singlerunnermodel.cpp
    sources/runners/runnerconfigurationwidget.cpp
    sources/runners/runnermodel.cpp
    tabmodel.cpp
    )

kde4_add_ui_files(components_SRCS
    dirconfigurationwidget.ui
    sources/installedapps/installedappsconfigurationwidget.ui
    sources/runners/runnerconfigurationwidget.ui
    )

qt4_automoc(${components_SRCS})

include_directories(
    ${lib_BINARY_DIR}
    ${lib_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/installedapps
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/runners
    )

kde4_add_library(componentsplugin SHARED ${components_SRCS})

target_link_libraries(componentsplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_SOLID_LIBS}
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
