project(salcomponents)

set(salcomponents_SRCS
    favoriteappsmodel.cpp
    pagemodel.cpp
    pathmodel.cpp
    placesmodel.cpp
    powermodel.cpp
    runnerinfoprovider.cpp
    sharedconfig.cpp
    salrunnermodel.cpp
    salservicemodel.cpp
    salcomponentsplugin.cpp
    sessionmodel.cpp
    sourcearguments.cpp
    )

qt4_automoc(${salcomponents_SRCS})

kde4_add_library(salcomponentsplugin SHARED ${salcomponents_SRCS})

target_link_libraries(salcomponentsplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4WORKSPACE_KWORKSPACE_LIBS}
        )

install(TARGETS salcomponentsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/sal/components)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/sal/components)
