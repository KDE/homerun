project(sources)

set(sources_SRCS
    dir/dirconfigurationwidget.cpp
    dir/dirmodel.cpp
    favorites/favoriteappsmodel.cpp
    favorites/favoriteplacesmodel.cpp
    favorites/favoriteutils.cpp
    favorites/fileplacesmodel.cpp
    favorites/kfileplacesitem.cpp
    favorites/kfileplacessharedbookmarks.cpp
    installedapps/changenotifier.cpp
    installedapps/filterableinstalledappsmodel.cpp
    installedapps/groupedinstalledappsmodel.cpp
    installedapps/installedappsconfigurationwidget.cpp
    installedapps/installedappsmodel.cpp
    power/powermodel.cpp
    recentapps/recentappsmodel.cpp
    runners/querymatchmodel.cpp
    runners/singlerunnermodel.cpp
    runners/runnerconfigurationwidget.cpp
    runners/runnermodel.cpp
    session/openedsessionsmodel.cpp
    session/sessionmodel.cpp
    session/sessionswatcher.cpp
    )

kde4_add_ui_files(sources_SRCS
    dir/dirconfigurationwidget.ui
    installedapps/installedappsconfigurationwidget.ui
    runners/runnerconfigurationwidget.ui
    )

qt4_automoc(${sources_SRCS})

include_directories(
    ${lib_BINARY_DIR}
    ${lib_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dir
    ${CMAKE_CURRENT_SOURCE_DIR}/favorites
    ${CMAKE_CURRENT_SOURCE_DIR}/installedapps
    ${CMAKE_CURRENT_SOURCE_DIR}/power
    ${CMAKE_CURRENT_SOURCE_DIR}/recentapps
    ${CMAKE_CURRENT_SOURCE_DIR}/runners
    ${CMAKE_CURRENT_SOURCE_DIR}/session
    )

kde4_add_library(componentsplugin SHARED ${sources_SRCS})

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
