project(appswithfilters)

# Dependencies
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Homerun REQUIRED)

# Source
set(appswithfilters_SRCS
    changenotifier.cpp
    groupedinstalledappsmodel.cpp
    installedappsmodel.cpp
    )

qt4_automoc(${appswithfilters_SRCS})

# Build
kde4_add_plugin(homerun_source_appswithfilters SHARED ${appswithfilters_SRCS})

include_directories(
    ${HOMERUN_INCLUDES}
    ${KDE4_INCLUDES}
    )

# The library name must start with "homerun_source_". The
# HOMERUN_EXPORT_SOURCE C macro expects this.
target_link_libraries(homerun_source_appswithfilters
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${HOMERUN_LIBRARIES}
    )

# Install
install(TARGETS homerun_source_appswithfilters DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES homerun-source-appswithfilters.desktop DESTINATION ${SERVICES_INSTALL_DIR})
