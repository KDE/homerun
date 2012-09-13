project(lib)

set(lib_SRCS
    abstractsource.cpp
    favoriteappsmodel.cpp
    groupedservicemodel.cpp
    pathmodel.cpp
    placesmodel.cpp
    powermodel.cpp
    runnermodel.cpp
    servicemodel.cpp
    sessionmodel.cpp
    sourceid.cpp
    sourceregistry.cpp
    )

qt4_automoc(${lib_SRCS})

kde4_add_library(homerun SHARED ${lib_SRCS})

target_link_libraries(homerun
    ${QT_QTCORE_LIBRARY}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4WORKSPACE_KWORKSPACE_LIBS}
    )

install(TARGETS homerun DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES homerun-source.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
