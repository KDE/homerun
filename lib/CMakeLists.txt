project(lib)

## Lib version
### Bump this one when a binary-incompatible change is introduced
set(lib_VERSION_MAJOR 0)

### Bump this one when the API is extended in a binary-compatible way
set(lib_VERSION_MINOR 0)

### Bump this one when changes do not extend the API
set(lib_VERSION_PATCH 0)

set(lib_VERSION ${lib_VERSION_MAJOR}.${lib_VERSION_MINOR}.${lib_VERSION_PATCH})

set(lib_SRCS
    abstractsource.cpp
    actionlist.cpp
    pathmodel.cpp
    sourceconfigurationwidget.cpp
    )

# Build
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBKONQ_INCLUDE_DIR}
    )

qt4_automoc(${lib_SRCS})

configure_file(libhomerun_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/libhomerun_config.h @ONLY)

kde4_add_library(homerun SHARED ${lib_SRCS})
set_target_properties(homerun PROPERTIES
    VERSION ${lib_VERSION}
    SOVERSION ${lib_VERSION_MAJOR}
    )

target_link_libraries(homerun
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${LIBKONQ_LIBRARY}
    )

# Doxygen
configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(dox COMMAND
    doxygen
    )

# Install
## Headers
install(DIRECTORY .
    DESTINATION include/homerun
    FILES_MATCHING PATTERN "*.h"
    )

## .so
install(TARGETS homerun
    EXPORT HomerunTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
    )

## CMakeConfig
install(EXPORT HomerunTargets
    DESTINATION ${LIB_INSTALL_DIR}/cmake/Homerun
    FILE HomerunTargets.cmake
    )

configure_file(
    HomerunConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HomerunConfig.cmake
    @ONLY)

macro_write_basic_cmake_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HomerunConfigVersion.cmake
    ${lib_VERSION_MAJOR} ${lib_VERSION_MINOR} ${lib_VERSION_PATCH}
    )

INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HomerunConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HomerunConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/Homerun
    )

## Desktop file
install(FILES homerun-source.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
