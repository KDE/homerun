include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell)

set(plasma-sal_SRCS
    main.cpp
    plasmaapp.cpp
    salcorona.cpp
    salview.cpp
)

kde4_add_ui_files(plasma-sal_SRCS BackgroundDialog.ui)

set(plasmaapp_dbusXML org.kde.plasma-sal.App.xml)

qt4_add_dbus_adaptor(plasma-sal_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

kde4_add_executable(plasma-sal ${plasma-sal_SRCS})

target_link_libraries(plasma-sal ${KDE4_PLASMA_LIBS} kworkspace  ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS}
                                     ${X11_X11_LIB} plasmagenericshell)
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-sal ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(plasma-sal PROPERTIES OUTPUT_NAME plasma-sal)

install(TARGETS plasma-sal ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-salrc DESTINATION ${CONFIG_INSTALL_DIR})
