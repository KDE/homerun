project(kicker)

set(kicker_SRCS
    appletproxy.cpp
    fadeouteffect.cpp
    fadeoutitem.cpp
    kickerplugin.cpp
    processrunner.cpp
    sourcelistmodel.cpp
    urlconverter.cpp
    windowsystem.cpp
    )

qt4_automoc(${kicker_SRCS})

kde4_add_library(kickerplugin SHARED ${kicker_SRCS})

target_link_libraries(kickerplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        )

install(TARGETS kickerplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/kicker)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/homerun/kicker)
