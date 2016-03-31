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
// File
//==============================================================================

#ifndef FILE_H
#define FILE_H

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include <QStringList>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT File
{
public:
    enum Type {
        New,
        Local,
        Remote
    };

    enum Status {
        // As a result of checking a file

        Changed,
        DependenciesChanged,
        AllChanged,
        Unchanged,
        Deleted,

        // As a result of setting the locked status of a file

        LockedNotNeeded,
        LockedSet,
        LockedNotSet
    };

    explicit File(const QString &pFileName, const Type &pType,
                  const QString &pUrl);
    ~File();

    QString fileName() const;
    bool setFileName(const QString &pFileName);

    Status check();

    QString sha1(const QString &pFileName = QString()) const;

    void reset(const bool &pResetDependencies = true);

    bool isDifferent() const;
    bool isDifferent(const QByteArray &pFileContents) const;

    bool isNew() const;
    bool makeNew(const QString &pFileName);

    int newIndex() const;

    bool isLocal() const;
    bool isRemote() const;

    QString url() const;

    bool isModified() const;
    bool setModified(const bool &pModified);

    bool isReadable() const;
    bool isWritable() const;
    bool isReadableAndWritable() const;

    bool isLocked() const;
    Status setLocked(const bool &pLocked);

    QStringList dependencies() const;
    bool setDependencies(const QStringList &pDependencies);

private:
    QString mFileName;
    QString mUrl;
    QString mSha1;

    int mNewIndex;

    bool mModified;

    QStringList mDependencies;
    QStringList mDependenciesSha1;
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
