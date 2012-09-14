project(multisources)

set(multisources_SRCS
    multisources.cpp
    )

qt4_automoc(${multisources_SRCS})

kde4_add_plugin(homerun_source_multisources SHARED ${multisources_SRCS})

# FIXME
set(HOMERUN_INCLUDE_DIRS ${lib_SOURCE_DIR})
set(HOMERUN_LIBRARIES homerun)

include_directories(
    ${HOMERUN_INCLUDE_DIRS}
    )

# The library name must start with "homerun_source_". The
# HOMERUN_EXPORT_SOURCE C macro expects this.
target_link_libraries(homerun_source_multisources
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${HOMERUN_LIBRARIES}
    )

install(TARGETS homerun_source_multisources DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES homerun-source-multisources.desktop DESTINATION ${SERVICES_INSTALL_DIR})
