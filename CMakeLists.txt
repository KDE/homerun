project(homerun)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# Packaging
set(HOMERUN_VERSION "0.2.0")

set(HOMERUN_SOURCE_VERSION "")
set(HOMERUN_SOURCE_DATE "")

#if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
#    message(STATUS "Extracting source information using git log")
#    execute_process(COMMAND git show --pretty=%h HEAD
#        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#        OUTPUT_VARIABLE HOMERUN_SOURCE_VERSION
#        OUTPUT_STRIP_TRAILING_WHITESPACE)
#    execute_process(COMMAND git show --pretty=%ci HEAD
#        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#        OUTPUT_VARIABLE HOMERUN_SOURCE_DATE
#        OUTPUT_STRIP_TRAILING_WHITESPACE)
#endif()
message(STATUS "Source version: ${HOMERUN_SOURCE_VERSION}")
message(STATUS "Source date: ${HOMERUN_SOURCE_DATE}")

# Dependencies
find_package(KDE4 4.9.97 REQUIRED)
include(KDE4Defaults)
find_package(KDE4Workspace REQUIRED)
find_package(KDeclarative REQUIRED)

# Compile options
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-std=c++0x)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# configuration file
configure_file(homerun_config.h.in ${CMAKE_BINARY_DIR}/homerun_config.h @ONLY)

# config/
install(FILES config/homerunrc DESTINATION ${CONFIG_INSTALL_DIR})

add_subdirectory(lib)
add_subdirectory(applet)
add_subdirectory(shell)
add_subdirectory(components)
add_subdirectory(fixes)
add_subdirectory(tests)
add_subdirectory(icons)
add_subdirectory(package)
add_subdirectory(plugins)
