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
// CellML Model Repository window
//==============================================================================

#include "cellmlmodelrepositorywindow.h"
#include "cellmlmodelrepositorywidget.h"
#include "coreutils.h"

//==============================================================================

#include "ui_cellmlmodelrepositorywindow.h"

//==============================================================================

#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

//==============================================================================

namespace OpenCOR {
namespace CellMLModelRepository {

//==============================================================================

CellmlModelRepositoryWindow::CellmlModelRepositoryWindow(QWidget *pParent) :
    OrganisationWidget(pParent),
    mGui(new Ui::CellmlModelRepositoryWindow),
    mModelListRequested(false)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Make the name value our focus proxy

    setFocusProxy(mGui->filterValue);

    // Create and add the CellML Model Repository widget

    mCellmlModelRepositoryWidget = new CellmlModelRepositoryWidget(this);

    mGui->dockWidgetContents->layout()->addWidget(mCellmlModelRepositoryWidget);

    // Create and populate our custom context menu

    mCustomContextMenu = new QMenu(this);

    mCustomContextMenu->addAction(mGui->actionCopy);

    // We want our own context menu for the help widget (indeed, we don't want
    // the default one which has the reload menu item and not the other actions
    // that we have in our tool bar widget, so...)

    mCellmlModelRepositoryWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mCellmlModelRepositoryWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu(const QPoint &)));

    // Keep track of the window's visibility, so that we can request the list of
    // models, if necessary

    connect(this, SIGNAL(visibilityChanged(bool)),
            this, SLOT(retrieveModelList(const bool &)));

    // Create a network access manager so that we can then retrieve a list of
    // CellML models from the CellML Model Repository

    mNetworkAccessManager = new QNetworkAccessManager(this);

    // Make sure that we get told when the download of our Internet file is
    // complete, as well as if there are SSL errors (which would happen if the
    // website's certificate is invalid, e.g. it expired)

    connect(mNetworkAccessManager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(finished(QNetworkReply *)) );
    connect(mNetworkAccessManager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
            this, SLOT(sslErrors(QNetworkReply *, const QList<QSslError> &)) );

    // Connection to update the enabled state of our copy action

    connect(mCellmlModelRepositoryWidget, SIGNAL(copyTextEnabled(const bool &)),
            mGui->actionCopy, SLOT(setEnabled(bool)));
}

//==============================================================================

CellmlModelRepositoryWindow::~CellmlModelRepositoryWindow()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlModelRepositoryWindow::retranslateUi()
{
    // Retranslate the whole window

    mGui->retranslateUi(this);

    // Retranslate our list of models

    outputModelList(mModelList);
}

//==============================================================================

void CellmlModelRepositoryWindow::outputModelList(const QStringList &pModelList)
{
    // Output a given list of models

    mModelList = pModelList;

    QString contents = "";
    const QString leadingSpaces = "        ";

    if (mModelList.count()) {
        // We have models to list, so...

        if (mModelList.count() == 1)
            contents = tr("<strong>1</strong> CellML model was found:")+"\n";
        else
            contents = tr("<strong>%1</strong> CellML models were found:").arg(mModelList.count())+"\n";

        contents += leadingSpaces+"<ul>\n";

        foreach (const QString &model, mModelList)
            contents += leadingSpaces+"<li><a href=\""+mModelUrls[mModelNames.indexOf(model)]+"\">"+model+"</a></li>\n";

        contents += leadingSpaces+"</ul>";
    } else if (mModelNames.empty()) {
        if (mErrorMsg.count())
            // Something went wrong while trying to retrieve the list of models,
            // so...

            contents = leadingSpaces+tr("<strong>Error:</strong> ")+Core::formatErrorMsg(mErrorMsg);
        else if (mModelListRequested)
            // The list is still being loaded, so...

            contents = leadingSpaces+tr("Please wait while the list of CellML models is being loaded...");
        else
            // We have yet to request the list of models, so...

            contents = QString();
    } else {
        // No model could be found, so...

        contents = leadingSpaces+tr("No CellML model matches your criteria");
    }

    // Output the list matching the search criteria, or a message telling the
    // user what went wrong, if anything

    mCellmlModelRepositoryWidget->output(contents);
}

//==============================================================================

void CellmlModelRepositoryWindow::on_filterValue_textChanged(const QString &text)
{
    // Generate a Web page that contains all the models which match our search
    // criteria

    outputModelList(mModelNames.filter(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption)));
}

//==============================================================================

void CellmlModelRepositoryWindow::on_actionCopy_triggered()
{
    // Copy the current slection to the clipboard

    QApplication::clipboard()->setText(mCellmlModelRepositoryWidget->selectedText());
}

//==============================================================================

void CellmlModelRepositoryWindow::on_refreshButton_clicked()
{
    // Output the message telling the user that the list is being downloaded
    // Note: to clear mModelNames ensures that we get the correct message from
    //       outputModelList...

    mModelListRequested = true;

    mModelNames.clear();

    outputModelList(QStringList());

    // Disable the GUI side, so that the user doesn't get confused and ask to
    // refresh over and over again while he should just be patient

    setEnabled(false);

    // Get the list of CellML models

    mNetworkAccessManager->get(QNetworkRequest(QUrl("https://models.physiomeproject.org/cellml/workspace/rest/contents.json")));
}

//==============================================================================

void CellmlModelRepositoryWindow::finished(QNetworkReply *pNetworkReply)
{
    // Clear some properties

    mModelNames.clear();
    mModelUrls.clear();

    // Output the list of models, should we have retrieved it without any
    // problem

    if (pNetworkReply->error() == QNetworkReply::NoError) {
        // Parse the JSON code

        QJsonParseError jsonParseError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(pNetworkReply->readAll(), &jsonParseError);

        if (jsonParseError.error == QJsonParseError::NoError) {
            // Retrieve the list of CellML models

            QVariantMap resultMap = jsonDocument.object().toVariantMap();

            foreach (const QVariant &modelVariant, resultMap["values"].toList()) {
                QVariantList modelDetailsVariant = modelVariant.toList();

                mModelNames << modelDetailsVariant[0].toString();
                mModelUrls << modelDetailsVariant[1].toString();
            }

            // Everything went fine, so...

            mErrorMsg = QString();
        } else {
            // Something went wrong, so...

            mErrorMsg = jsonParseError.errorString();
        }
    } else {
        // Something went wrong, so...

        mErrorMsg = pNetworkReply->errorString();
    }

    // Initialise the output using whatever search criteria is present

    on_filterValue_textChanged(mGui->filterValue->text());

    // Re-enable the GUI side

    setEnabled(true);

    // Give, within the current window, the focus to mGui->filterValue, but
    // only if the current window already has the focus

    Core::setFocusTo(mGui->filterValue);

    // Delete (later) the network reply

    pNetworkReply->deleteLater();
}

//==============================================================================

void CellmlModelRepositoryWindow::sslErrors(QNetworkReply *pNetworkReply,
                                            const QList<QSslError> &pSslErrors)
{
    // Ignore the SSL errors since we trust the website and therefore its
    // certificate (even if it is invalid, e.g. it has expired)

    pNetworkReply->ignoreSslErrors(pSslErrors);
}

//==============================================================================

void CellmlModelRepositoryWindow::showCustomContextMenu(const QPoint &pPosition) const
{
    Q_UNUSED(pPosition);

    // Show our custom context menu for our CellML Models Repository widget

    mCustomContextMenu->exec(QCursor::pos());
}

//==============================================================================

void CellmlModelRepositoryWindow::retrieveModelList(const bool &pVisible)
{
    // Retrieve the list of models, if we are becoming visible and the list of
    // models has never been requested before

    if (pVisible && !mModelListRequested)
        on_refreshButton_clicked();
}

//==============================================================================

}   // namespace CellMLModelRepository
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
