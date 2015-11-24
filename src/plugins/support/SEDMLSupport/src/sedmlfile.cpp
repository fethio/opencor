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
// SED-ML file class
//==============================================================================

#include "corecliutils.h"
#include "sedmlfile.h"

//==============================================================================

#include <QRegularExpression>
#include <QTemporaryFile>

//==============================================================================

#include "sedmlapidisablewarnings.h"
    #include "sedml/SedDocument.h"
    #include "sedml/SedReader.h"
#include "sedmlapienablewarnings.h"

//==============================================================================

namespace OpenCOR {
namespace SEDMLSupport {

//==============================================================================

SedmlFile::SedmlFile(const QString &pFileName) :
    StandardSupport::StandardFile(pFileName)
{
}

//==============================================================================

bool SedmlFile::load()
{
    // Consider the file loaded

    return true;
}

//==============================================================================

bool SedmlFile::save(const QString &pNewFileName)
{
    Q_UNUSED(pNewFileName);

    // Consider the file saved

    return true;
}

//==============================================================================

bool SedmlFile::isValid(const QString &pFileContents, SedmlFileIssues &pIssues)
{
    // Check whether the given file contents is SED-ML valid and, if not,
    // populate pIssues with the problems found (after having emptied its
    // contents)
    // Note: normally, we would create a temporary SED-ML document using
    //       libsedml::readSedMLFromString(), but if the given file contents
    //       doesn't start with:
    //           <?xml version='1.0' encoding='UTF-8'?>
    //       then libsedml::readSedMLFromString() will prepend it to our given
    //       file contents, which is not what we want. So, instead, we create a
    //       temporary file which contents is that of our given file contents,
    //       and simply call libsedml::readSedML()...

    pIssues.clear();

    QTemporaryFile file;
    QByteArray fileContentsByteArray = pFileContents.toUtf8();

    file.open();

    file.write(fileContentsByteArray);
    file.flush();

    QByteArray fileNameByteArray = file.fileName().toUtf8();
    libsedml::SedDocument *sedmlDocument = libsedml::readSedML(fileNameByteArray.constData());
    libsedml::SedErrorLog *errorLog = sedmlDocument->getErrorLog();

    for (unsigned int i = 0, iMax = errorLog->getNumErrors(); i < iMax; ++i) {
        const libsedml::SedError *error = errorLog->getError(i);
        SedmlFileIssue::Type issueType;

        switch (error->getSeverity()) {
        case LIBSBML_SEV_INFO:
            issueType = SedmlFileIssue::Information;

            break;
        case LIBSBML_SEV_ERROR:
            issueType = SedmlFileIssue::Error;

            break;
        case LIBSBML_SEV_WARNING:
            issueType = SedmlFileIssue::Warning;

            break;
        case LIBSBML_SEV_FATAL:
            issueType = SedmlFileIssue::Fatal;

            break;
        }

        static const QRegularExpression TrailingEmptyLinesRegEx = QRegularExpression("[\\n]*$");

        QString errorMessage = QString::fromStdString(error->getMessage()).remove(TrailingEmptyLinesRegEx);

        pIssues << SedmlFileIssue(issueType, error->getLine(), error->getColumn(), errorMessage);
    }

    file.close();

    // Only consider the given file contents SED-ML valid if it has no errors

    return sedmlDocument->getNumErrors(libsedml::LIBSEDML_SEV_ERROR) == 0;
}

//==============================================================================

}   // namespace SEDMLSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
