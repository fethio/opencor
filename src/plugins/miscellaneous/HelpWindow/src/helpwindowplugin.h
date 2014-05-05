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
// HelpWindow plugin
//==============================================================================

#ifndef HELPWINDOWPLUGIN_H
#define HELPWINDOWPLUGIN_H

//==============================================================================

#include "i18ninterface.h"
#include "plugininterface.h"
#include "plugininfo.h"
#include "windowinterface.h"

//==============================================================================

namespace OpenCOR {
namespace HelpWindow {

//==============================================================================

PLUGININFO_FUNC HelpWindowPluginInfo();

//==============================================================================

class HelpWindowWindow;

//==============================================================================

class HelpWindowPlugin : public QObject, public I18nInterface,
                         public PluginInterface, public WindowInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.HelpWindowPlugin" FILE "helpwindowplugin.json")

    Q_INTERFACES(OpenCOR::I18nInterface)
    Q_INTERFACES(OpenCOR::PluginInterface)
    Q_INTERFACES(OpenCOR::WindowInterface)

public:
#include "i18ninterface.inl"
#include "plugininterface.inl"
#include "windowinterface.inl"

private:
    QAction *mHelpAction;

    HelpWindowWindow *mHelpWindow;
};

//==============================================================================

}   // namespace HelpWindow
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================