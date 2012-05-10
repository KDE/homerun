set(salviewer_SRCS
    fullview.cpp
    main.cpp
)

qt4_add_dbus_adaptor(salviewer_SRCS org.kde.salviewer.xml
   fullview.h FullView)

find_package(KDeclarative REQUIRED)

kde4_add_executable(salviewer ${salviewer_SRCS})

target_link_libraries(salviewer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDECLARATIVE_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARY}
    )


install(TARGETS salviewer ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES salviewer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
