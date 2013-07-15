project(homerun)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# Packaging
set(HOMERUN_VERSION "1.1.0")

# Dependencies
find_package(KDE4 4.9.97 REQUIRED)
include(KDE4Defaults)
find_package(KDE4Workspace REQUIRED)
find_package(KDeclarative REQUIRED)
find_package(LibKonq REQUIRED)
macro_log_feature(LIBKONQ_FOUND "libkonq" "Standard file management features" "http://www.kde.org" TRUE)

# Compile options
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-std=c++0x)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# configuration file
configure_file(homerun_config.h.in ${CMAKE_BINARY_DIR}/homerun_config.h @ONLY)

# config/
install(FILES config/homerunrc DESTINATION ${CONFIG_INSTALL_DIR})

install(FILES data/favoriteapps.xml DESTINATION ${DATA_INSTALL_DIR}/homerun/)

add_subdirectory(lib)
add_subdirectory(applet)
add_subdirectory(shell)
add_subdirectory(components)
add_subdirectory(fixes)
add_subdirectory(tests)
add_subdirectory(icons)
add_subdirectory(package)
add_subdirectory(plugins)

get_filename_component(_po_path po ABSOLUTE)

if (EXISTS ${_po_path}/CMakeLists.txt)
    find_package(Msgfmt REQUIRED)
    find_package(Gettext REQUIRED)
endif()

if (EXISTS ${_po_path}/CMakeLists.txt)
    add_subdirectory(po)
endif()

macro_display_feature_log()
