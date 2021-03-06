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
// COMBINE support tests
//==============================================================================

#pragma once

//==============================================================================

#include <QObject>

//==============================================================================

namespace OpenCOR {
namespace COMBINESupport {
    class CombineArchive;
}   // namespace COMBINESupport
}   // namespace OpenCOR

//==============================================================================

class Tests : public QObject
{
    Q_OBJECT

private:
    OpenCOR::COMBINESupport::CombineArchive *mCombineArchive;

    void doBasicTests(const QString &pFileName = QString());

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void basicTests();
    void loadingErrorTests();
};

//==============================================================================
// End of file
//==============================================================================
