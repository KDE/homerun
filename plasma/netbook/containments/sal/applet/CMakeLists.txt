project(sallauncher)

set(sallauncher_SRCS sallauncher.cpp)

kde4_add_plugin(plasma_applet_sallauncher ${sallauncher_SRCS})

target_link_libraries(plasma_applet_sallauncher ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} )

install(TARGETS plasma_applet_sallauncher
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-sallauncher.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
