set(homerun_plasmoid_dir ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.homerun)
set(metadata_dot_desktop ${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop)

configure_file(metadata.desktop.cmake
    ${metadata_dot_desktop} @ONLY)

install(DIRECTORY contents
    DESTINATION ${homerun_plasmoid_dir})

install(FILES ${metadata_dot_desktop}
    DESTINATION ${homerun_plasmoid_dir})

install(FILES ${metadata_dot_desktop}
    DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-homerun.desktop)
