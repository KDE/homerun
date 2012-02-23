project(tasks)

set(tasks_SRCS tasks.cpp)

kde4_add_plugin(plasma_applet_org_kde_tasks ${tasks_SRCS})

target_link_libraries(plasma_applet_org_kde_tasks ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTDECLARATIVE_LIBRARY} taskmanager)

install(TARGETS plasma_applet_org_kde_tasks
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-tasks.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.tasks)
