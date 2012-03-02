project(salservicemodel)

set(salservicemodel_SRCS
    salservicemodel.cpp
    salservicemodelplugin.cpp
    )

qt4_automoc(${salservicemodel_SRCS})

kde4_add_library(salservicemodelplugin SHARED ${salservicemodel_SRCS})

target_link_libraries(salservicemodelplugin
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_PLASMA_LIBS}
        )

install(TARGETS salservicemodelplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/salservicemodel)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/salservicemodel)
