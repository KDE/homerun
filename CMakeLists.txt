project(homerun)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# Dependencies
find_package(KDE4 ${KDE_MIN_VERSION} REQUIRED)
include(KDE4Defaults)
find_package(KDE4Workspace REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-std=c++0x)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# package/
install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.homerun)

install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-homerun.desktop)

# config/
install(FILES config/homerunrc DESTINATION ${CONFIG_INSTALL_DIR})

add_subdirectory(lib)
add_subdirectory(applet)
add_subdirectory(shell)
add_subdirectory(components)
add_subdirectory(fixes)
add_subdirectory(tests)
add_subdirectory(icons)
add_subdirectory(plugins/helloworld)
