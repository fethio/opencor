PROJECT(CVODESolverPlugin)

# Add the plugin

ADD_PLUGIN(CVODESolver
    SOURCES
        ../../i18ninterface.cpp
        ../../plugininfo.cpp
        ../../solverinterface.cpp

        src/cvodesolver.cpp
        src/cvodesolverplugin.cpp
    HEADERS_MOC
        ../../solverinterface.h

        src/cvodesolverplugin.h
    INCLUDE_DIRS
        src
    PLUGINS
        ${SUNDIALS_PLUGIN}
    PLUGIN_BINARIES
        ${SUNDIALS_PLUGIN_BINARY}
    QT_MODULES
        Widgets
)
