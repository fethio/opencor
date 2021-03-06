PROJECT(SingleCellViewPlugin)

# Add the plugin

ADD_PLUGIN(SingleCellView
    SOURCES
        ../../datastoreinterface.cpp
        ../../filetypeinterface.cpp
        ../../i18ninterface.cpp
        ../../plugin.cpp
        ../../plugininfo.cpp
        ../../pluginmanager.cpp
        ../../solverinterface.cpp
        ../../viewinterface.cpp

        src/singlecellviewcontentswidget.cpp
        src/singlecellviewinformationgraphswidget.cpp
        src/singlecellviewinformationparameterswidget.cpp
        src/singlecellviewinformationsimulationwidget.cpp
        src/singlecellviewinformationsolverswidget.cpp
        src/singlecellviewinformationwidget.cpp
        src/singlecellviewplugin.cpp
        src/singlecellviewsimulation.cpp
        src/singlecellviewsimulationworker.cpp
        src/singlecellviewsimulationwidget.cpp
        src/singlecellviewwidget.cpp
    HEADERS_MOC
        ../../datastoreinterface.h
        ../../plugin.h
        ../../pluginmanager.h
        ../../solverinterface.h

        src/singlecellviewcontentswidget.h
        src/singlecellviewinformationgraphswidget.h
        src/singlecellviewinformationparameterswidget.h
        src/singlecellviewinformationsimulationwidget.h
        src/singlecellviewinformationsolverswidget.h
        src/singlecellviewinformationwidget.h
        src/singlecellviewplugin.h
        src/singlecellviewsimulation.h
        src/singlecellviewsimulationworker.h
        src/singlecellviewsimulationwidget.h
        src/singlecellviewwidget.h
    INCLUDE_DIRS
        src
    DEFINITIONS
        ${QWT_DEFINITIONS}
    PLUGINS
        CellMLAPI
        CellMLSupport
        COMBINESupport
        Compiler
        Core
        GraphPanelWidget
        ${LLVM_PLUGIN}
        ${QWT_PLUGIN}
        SBMLAPI
        SEDMLAPI
        SEDMLSupport
        ${ZLIB_PLUGIN}
    PLUGIN_BINARIES
        ${LLVM_PLUGIN_BINARY}
        ${QWT_PLUGIN_BINARY}
        ${ZLIB_PLUGIN_BINARY}
    EXTERNAL_BINARIES
        ${CELLML_API_EXTERNAL_BINARIES}
        ${SBML_API_EXTERNAL_BINARIES}
        ${SEDML_API_EXTERNAL_BINARIES}
)
