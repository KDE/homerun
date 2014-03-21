project(homerunlauncher)

set(homerunlauncher_SRCS
    homerunlauncher.cpp
    )

kde4_add_ui_files(homerunlauncher_SRCS
    generalconfigpage.ui
    )

kde4_add_plugin(plasma_applet_homerunlauncher ${homerunlauncher_SRCS})

target_link_libraries(plasma_applet_homerunlauncher ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} )

configure_file(plasma-applet-homerunlauncher.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-homerunlauncher.desktop @ONLY)

install(TARGETS plasma_applet_homerunlauncher
    DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-homerunlauncher.desktop
    DESTINATION ${SERVICES_INSTALL_DIR})
