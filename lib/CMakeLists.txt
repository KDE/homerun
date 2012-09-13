project(lib)

## Lib version
### Bump this one when a binary-incompatible change is introduced
set(lib_SO_VERSION 0)

### Bump this one when the API is extended in a binary-compatible way
set(lib_MINOR_VERSION 0)

### Bump this one when changes do not extend the API
set(lib_PATCH_VERSION 0)

set(lib_VERSION ${lib_SO_VERSION}.${lib_MINOR_VERSION}.${lib_PATCH_VERSION})

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
set_target_properties(homerun PROPERTIES
    VERSION ${lib_VERSION}
    SOVERSION ${lib_SO_VERSION}
    )

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
