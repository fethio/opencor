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
// MathML viewer plugin
//==============================================================================

#pragma once

//==============================================================================

#include "i18ninterface.h"
#include "plugininfo.h"

//==============================================================================

namespace OpenCOR {
namespace MathMLViewer {

//==============================================================================

PLUGININFO_FUNC MathMLViewerPluginInfo();

//==============================================================================

class MathMLViewerPlugin : public QObject, public I18nInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.MathMLViewerPlugin" FILE "mathmlviewerplugin.json")

    Q_INTERFACES(OpenCOR::I18nInterface)

public:
#include "i18ninterface.inl"
};

//==============================================================================

}   // namespace MathMLViewer
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
