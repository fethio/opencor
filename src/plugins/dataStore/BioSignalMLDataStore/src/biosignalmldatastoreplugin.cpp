/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// BioSignalML data store plugin
//==============================================================================

#include "biosignalmldatastoredata.h"
#include "biosignalmldatastoreexporter.h"
#include "biosignalmldatastoreplugin.h"
#include "biosignalmldatastoresavedialog.h"
#include "corecliutils.h"

//==============================================================================

#include <QMainWindow>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace BioSignalMLDataStore {

//==============================================================================

PLUGININFO_FUNC BioSignalMLDataStorePluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a BioSignalML specific data store plugin."));
    descriptions.insert("fr", QString::fromUtf8("une extension de magasin de données spécifique à BioSignalML."));

    return new PluginInfo("Data Store", true, false,
                          QStringList() << "BioSignalMLAPI" << "Core",
                          descriptions);
}

//==============================================================================
// I18n interface
//==============================================================================

void BioSignalMLDataStorePlugin::retranslateUi()
{
    // We don't handle this interface...
    // Note: even though we don't handle this interface, we still want to
    //       support it since some other aspects of our plugin are
    //       multilingual...
}

//==============================================================================
// Data store interface
//==============================================================================

QString BioSignalMLDataStorePlugin::dataStoreName() const
{
    // Return the name of the data store

    return "BioSignalML";
}

//==============================================================================

DataStore::DataStoreData * BioSignalMLDataStorePlugin::getData(const QString &pFileName,
                                                               DataStore::DataStore *pDataStore) const
{
    // Retrieve some data that will help us to export our data

    QString comment = QObject::tr("Generated by") + " " + Core::version()
                    + " " + QObject::tr("at") + " " + QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
                    + " " + QObject::tr("from") + " " + pDataStore->uri();
    BioSignalMLSaveDialog saveDialog;
    saveDialog.setComment(comment);
    saveDialog.setDefaultFileName(Core::newFileName(pFileName, "", false, "biosignalml"));
    saveDialog.setSelectedVariables(pDataStore->variables());

    if (saveDialog.run()) {
        return new BiosignalmlDataStoreData(saveDialog.fileName(),
                                            saveDialog.shortName(),
                                            saveDialog.author(),
                                            saveDialog.description(),
                                            saveDialog.selectedVariables(),
                                            comment);
    } else {
        return 0;
    }
}

//==============================================================================

DataStore::DataStoreExporter * BioSignalMLDataStorePlugin::dataStoreExporterInstance(const QString &pFileName,
                                                                                     DataStore::DataStore *pDataStore,
                                                                                     DataStore::DataStoreData *pDataStoreData) const
{
    // Return an instance of our BioSignalML data store exporter

    return new BiosignalmlDataStoreExporter(pFileName, pDataStore, pDataStoreData);
}

//==============================================================================

}   // namespace BioSignalMLDataStore
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
