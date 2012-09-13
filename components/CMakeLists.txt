project(components)

set(components_SRCS
    abstractsource.cpp
    componentsplugin.cpp
    favoriteappsmodel.cpp
    groupedservicemodel.cpp
    pagemodel.cpp
    pathmodel.cpp
    placesmodel.cpp
    powermodel.cpp
    runnerinfoprovider.cpp
    runnermodel.cpp
    servicemodel.cpp
    sessionmodel.cpp
    sharedconfig.cpp
    sourceid.cpp
    sourceregistry.cpp
    )

qt4_automoc(${components_SRCS})

kde4_add_library(componentsplugin SHARED ${components_SRCS})

target_link_libraries(componentsplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4WORKSPACE_KWORKSPACE_LIBS}
        )

install(TARGETS componentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/components)

install(FILES homerun-source.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
