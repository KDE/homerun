set(homerunviewer_SRCS
    ${CMAKE_SOURCE_DIR}/internal/aboutdata.cpp
    fullview.cpp
    main.cpp
    )

include_directories(
    ${CMAKE_SOURCE_DIR}/internal
    )

qt4_add_dbus_adaptor(homerunviewer_SRCS org.kde.homerunviewer.xml
   fullview.h FullView)

kde4_add_executable(homerunviewer ${homerunviewer_SRCS})

target_link_libraries(homerunviewer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDECLARATIVE_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARY}
    )


install(TARGETS homerunviewer ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES homerunviewer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
