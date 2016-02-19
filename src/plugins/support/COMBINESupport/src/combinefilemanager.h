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
// COMBINE file manager
//==============================================================================

#ifndef COMBINEFILEMANAGER_H
#define COMBINEFILEMANAGER_H

//==============================================================================

#include "combinearchive.h"
#include "combinesupportglobal.h"
#include "standardfilemanager.h"

//==============================================================================

namespace OpenCOR {
namespace COMBINESupport {

//==============================================================================

class COMBINESUPPORT_EXPORT CombineFileManager : public StandardSupport::StandardFileManager
{
    Q_OBJECT

public:
    static CombineFileManager * instance();

    bool isCombineArchive(const QString &pFileName) const;

    CombineArchive * combineArchive(const QString &pFileName);

protected:
    virtual bool canLoadFile(const QString &pFileName) const;

    virtual QObject * newFile(const QString &pFileName) const;
};

//==============================================================================

}   // namespace COMBINESupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
