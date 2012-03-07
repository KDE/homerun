set(salviewer_SRCS
    fullview.cpp
    main.cpp
)

kde4_add_executable(salviewer ${salviewer_SRCS})

target_link_libraries(salviewer ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS salviewer ${INSTALL_TARGETS_DEFAULT_ARGS})

