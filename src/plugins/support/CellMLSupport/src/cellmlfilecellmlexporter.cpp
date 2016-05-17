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
// CellML file CellML exporter
//==============================================================================

#include "cellmlfilecellmlexporter.h"

//==============================================================================

#include "cellmlapidisablewarnings.h"
    #include "CellMLBootstrap.hpp"
#include "cellmlapienablewarnings.h"

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

CellmlFileCellmlExporter::CellmlFileCellmlExporter(iface::cellml_api::Model *pModel,
                                                   const std::wstring &pVersion) :
    CellmlFileExporter(),
    mModel(pModel)
{
    // Create an empty CellML model of the required version

    ObjRef<iface::cellml_api::CellMLBootstrap> cellmlBootstrap = CreateCellMLBootstrap();

    mExportedModel = cellmlBootstrap->createModel(pVersion);
}

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
