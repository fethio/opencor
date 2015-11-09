/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// SingleCellView plugin
//==============================================================================

#include "cellmlfilemanager.h"
#include "cellmlsupportplugin.h"
#include "coreguiutils.h"
#include "datastoreinterface.h"
#include "singlecellviewplugin.h"
#include "singlecellviewwidget.h"
#include "solverinterface.h"

//==============================================================================

#include <QMainWindow>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

PLUGININFO_FUNC SingleCellViewPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to run single cell simulations."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour exécuter des simulations unicellulaires."));

    return new PluginInfo("Simulation", true, false,
                          QStringList() << "CellMLSupport" << "COMBINESupport"<< "Qwt" << "SEDMLSupport",
                          descriptions);
}

//==============================================================================

SingleCellViewPlugin::SingleCellViewPlugin() :
    mSedmlFileTypes(FileTypes()),
    mCombineFileTypes(FileTypes())
{
}

//==============================================================================
// File handling interface
//==============================================================================

bool SingleCellViewPlugin::saveFile(const QString &pOldFileName,
                                    const QString &pNewFileName)
{
    // Let our view widget know that we want to save a file

    return mViewWidget->saveFile(pOldFileName, pNewFileName);
}

//==============================================================================

void SingleCellViewPlugin::fileOpened(const QString &pFileName)
{
    // Let our view widget know that a file has been opened

    mViewWidget->fileOpened(pFileName);
}

//==============================================================================

void SingleCellViewPlugin::filePermissionsChanged(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void SingleCellViewPlugin::fileModified(const QString &pFileName)
{
    // Let our view widget know that a file has been modified

    mViewWidget->fileModified(pFileName);
}

//==============================================================================

void SingleCellViewPlugin::fileReloaded(const QString &pFileName)
{
    // The given file has been reloaded, so let its corresponding view widget
    // know about it

    if (mViewWidget->contains(pFileName))
        mViewWidget->fileReloaded(pFileName);
}

//==============================================================================

void SingleCellViewPlugin::fileRenamed(const QString &pOldFileName,
                                       const QString &pNewFileName)
{
    // Let our view widget know that a file has been renamed

    mViewWidget->fileRenamed(pOldFileName, pNewFileName);
}

//==============================================================================

void SingleCellViewPlugin::fileSaved(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void SingleCellViewPlugin::fileClosed(const QString &pFileName)
{
    // Let our view widget know that a file has been closed

    mViewWidget->fileClosed(pFileName);
}

//==============================================================================
// I18n interface
//==============================================================================

void SingleCellViewPlugin::retranslateUi()
{
    // Retranslate our single cell view widget

    mViewWidget->retranslateUi();
}

//==============================================================================
// Plugin interface
//==============================================================================

void SingleCellViewPlugin::initializePlugin()
{
    // Create our single view widget

    mViewWidget = new SingleCellViewWidget(this, Core::mainWindow());

    // Hide our single view widget since it may not initially be shown in our
    // central widget

    mViewWidget->setVisible(false);
}

//==============================================================================

void SingleCellViewPlugin::finalizePlugin()
{
    // We don't handle this interface...
}

//==============================================================================

void SingleCellViewPlugin::pluginsInitialized(const Plugins &pLoadedPlugins)
{
    // Retrieve the different solvers and data stores that are available to us,
    // as well as the file types supported by the SEDMLSupport plugin

    SolverInterfaces solverInterfaces = SolverInterfaces();
    DataStoreInterfaces dataStoreInterfaces = DataStoreInterfaces();

    foreach (Plugin *plugin, pLoadedPlugins) {
        // Look for a solver

        SolverInterface *solverInterface = qobject_cast<SolverInterface *>(plugin->instance());

        if (solverInterface)
            solverInterfaces << solverInterface;

        // Look for a data store

        DataStoreInterface *dataStoreInterface = qobject_cast<DataStoreInterface *>(plugin->instance());

        if (dataStoreInterface)
            dataStoreInterfaces << dataStoreInterface;

        // File types supported by the SEDMLSupport and COMBINESupport plugins

        FileTypeInterface *fileTypeInterface = qobject_cast<FileTypeInterface *>(plugin->instance());

        if (fileTypeInterface) {
            if (!plugin->name().compare("SEDMLSupport"))
                mSedmlFileTypes << fileTypeInterface->fileTypes();
            else if (!plugin->name().compare("COMBINESupport"))
                mCombineFileTypes << fileTypeInterface->fileTypes();
        }
    }

    // Initialise our view widget with the different solvers and data stores
    // that are available to us

    mViewWidget->setSolverInterfaces(solverInterfaces);
    mViewWidget->setDataStoreInterfaces(dataStoreInterfaces);
}

//==============================================================================

void SingleCellViewPlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve our single cell view settings

    pSettings->beginGroup(mViewWidget->objectName());
        mViewWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellViewPlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of our single cell view settings

    pSettings->beginGroup(mViewWidget->objectName());
        mViewWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellViewPlugin::handleAction(const QUrl &pUrl)
{
    Q_UNUSED(pUrl);

    // We don't handle this interface...
}

//==============================================================================
// View interface
//==============================================================================

ViewInterface::Mode SingleCellViewPlugin::viewMode() const
{
    // Return our mode

    return ViewInterface::Simulation;
}

//==============================================================================

QStringList SingleCellViewPlugin::viewMimeTypes() const
{
    // Return the MIME types we support

    return QStringList() << CellMLSupport::CellmlMimeType;
}

//==============================================================================

QString SingleCellViewPlugin::viewDefaultFileExtension() const
{
    // Return the default file extension we support

    return CellMLSupport::CellmlFileExtension;
}

//==============================================================================

bool SingleCellViewPlugin::hasViewWidget(const QString &pFileName)
{
    // Make sure that we are dealing with a CellML file

    if (!CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName))
        return false;

    // Return whether we have a view widget for the given CellML file

    return mViewWidget->contains(pFileName);
}

//==============================================================================

QWidget * SingleCellViewPlugin::viewWidget(const QString &pFileName)
{
    // Make sure that we are dealing with a CellML file

    if (!CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName))
        return 0;

    // Update and return our simulation view widget using the given CellML file
    // Note: we temporarily disable updates for our simulation view widget, so
    //       as to avoid any risk of known/unknown/potential flickering...

    mViewWidget->setUpdatesEnabled(false);
        mViewWidget->initialize(pFileName);
    mViewWidget->setUpdatesEnabled(true);

    return mViewWidget;
}

//==============================================================================

void SingleCellViewPlugin::removeViewWidget(const QString &pFileName)
{
    // Ask our view widget to finalise the given CellML file

    mViewWidget->finalize(pFileName);
}

//==============================================================================

QString SingleCellViewPlugin::viewName() const
{
    // Return our single cell view's name

    return tr("Single Cell");
}

//==============================================================================

QIcon SingleCellViewPlugin::fileTabIcon(const QString &pFileName) const
{
    // Return the requested file tab icon

    return mViewWidget->fileTabIcon(pFileName);
}

//==============================================================================
// Plugin specific
//==============================================================================

FileTypes SingleCellViewPlugin::sedmlFileTypes() const
{
    // Return our SED-ML file types

    return mSedmlFileTypes;
}

//==============================================================================

FileTypes SingleCellViewPlugin::combineFileTypes() const
{
    // Return our COMBINE file types

    return mCombineFileTypes;
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
