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
// SED-ML editing interface
//==============================================================================

#ifndef SEDMLEDITINGINTERFACE_H
#define SEDMLEDITINGINTERFACE_H

//==============================================================================

#include <QObject>

//==============================================================================

namespace OpenCOR {

//==============================================================================

class SedmlEditingInterface
{
public:
#define INTERFACE_DEFINITION
    #include "sedmleditinginterface.inl"
#undef INTERFACE_DEFINITION
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

Q_DECLARE_INTERFACE(OpenCOR::SedmlEditingInterface, "OpenCOR::SedmlEditingInterface")

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
