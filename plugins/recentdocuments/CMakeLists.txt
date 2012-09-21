project(recentdocuments)

# Source
set(recentdocuments_SRCS
    recentdocuments.cpp
    )

qt4_automoc(${recentdocuments_SRCS})

# Build
kde4_add_plugin(homerun_source_recentdocuments SHARED ${recentdocuments_SRCS})

include_directories(
    ${lib_SOURCE_DIR}
    ${KDE4_INCLUDES}
    )

# The library name must start with "homerun_source_". The
# HOMERUN_EXPORT_SOURCE C macro expects this.
target_link_libraries(homerun_source_recentdocuments
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KFILE_LIBS}
    homerun
    )

# Install
install(TARGETS homerun_source_recentdocuments DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES homerun-source-recentdocuments.desktop DESTINATION ${SERVICES_INSTALL_DIR})
