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
// PMR window widget
//==============================================================================

#include "coreguiutils.h"
#include "i18ninterface.h"
#include "pmrwindowwidget.h"

//==============================================================================

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QIODevice>
#include <QLayout>
#include <QMenu>
#include <QRegularExpression>

//==============================================================================

namespace OpenCOR {
namespace PMRWindow {

//==============================================================================

PmrWindowWidget::PmrWindowWidget(QWidget *pParent) :
    WebViewerWidget::WebViewerWidget(pParent),
    Core::CommonWidget(),
    mExposureNames(QStringList()),
    mExposureDisplayed(QBoolList()),
    mExposureUrlId(QMap<QString, int>()),
    mInitialized(false),
    mErrorMessage(QString()),
    mInternetConnectionAvailable(true),
    mNumberOfFilteredExposures(0),
    mExposureUrls(QStringList()),
    mHaveExposureFiles(QMap<QString, bool>()),
    mHideExposureFiles(QMap<QString, bool>()),
    mExposureOrExposureFileUrl(QString())
{
    // Create and populate our context menu

    mContextMenu = new QMenu(this);

    mCopyAction = Core::newAction(QIcon(":/oxygen/actions/edit-copy.png"),
                                  this);

    connect(mCopyAction, SIGNAL(triggered(bool)),
            this, SLOT(copy()));

    mContextMenu->addAction(mCopyAction);

    // We want our own context menu

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu()));

    // Some connections to handle the clicking and hovering of a link

    connect(page(), SIGNAL(linkClicked(const QString &)),
            this, SLOT(linkClicked(const QString &)));
    connect(page(), SIGNAL(linkHovered(const QString &)),
            this, SLOT(linkHovered(const QString &)));

    // Retrieve the HTML template

    QByteArray fileContents;

    Core::readFileContentsFromFile(":/PMRWindow/output.html", fileContents);

    mTemplate = QString(fileContents).arg(Core::iconDataUri(":/oxygen/places/folder-downloads.png", 16, 16),
                                          Core::iconDataUri(":/oxygen/actions/document-open-remote.png", 16, 16),
                                          "%1", "%2");
}

//==============================================================================

void PmrWindowWidget::retranslateUi()
{
    // Retranslate our action

    I18nInterface::retranslateAction(mCopyAction, tr("Copy"),
                                     tr("Copy the URL to the clipboard"));

    // Retranslate our message, if we have been initialised

    if (mInitialized)
        page()->runJavaScript(QString("setMessage(\"%1\");").arg(message()));
}

//==============================================================================

QSize PmrWindowWidget::sizeHint() const
{
    // Suggest a default size for our PMR widget
    // Note: this is critical if we want a docked widget, with a PMR widget on
    //       it, to have a decent size when docked to the main window...

    return defaultSize(0.15);
}

//==============================================================================

QString PmrWindowWidget::message() const
{
    // Determine the message to be displayed, if any

    QString res = QString();

    if (mInternetConnectionAvailable && mErrorMessage.isEmpty()) {
        if (!mNumberOfFilteredExposures) {
            if (!mExposureNames.isEmpty())
                res = tr("No exposure matches your criteria.");
        } else if (mNumberOfFilteredExposures == 1) {
            res = tr("<strong>1</strong> exposure was found:");
        } else {
            res = tr("<strong>%1</strong> exposures were found:").arg(mNumberOfFilteredExposures);
        }
    } else {
        res = tr("<strong>Error:</strong> ")+Core::formatMessage(mInternetConnectionAvailable?
                                                                     mErrorMessage:
                                                                     Core::noInternetConnectionAvailableMessage(),
                                                                 true, true);
    }

    return res;
}

//==============================================================================

static const auto PmrScheme = QStringLiteral("pmr");
static const auto CloneWorkspaceAction = QStringLiteral("cloneWorkspace");
static const auto ShowExposureFilesAction = QStringLiteral("showExposureFiles");

//==============================================================================

void PmrWindowWidget::initialize(const PMRSupport::PmrExposures &pExposures,
                                 const QString &pErrorMessage,
                                 const QString &pFilter,
                                 const bool &pInternetConnectionAvailable)
{
    // Initialise / keep track of some properties

    mExposureNames.clear();
    mExposureDisplayed.clear();
    mExposureUrlId.clear();

    mErrorMessage = pErrorMessage;

    mInternetConnectionAvailable = pInternetConnectionAvailable;

    mExposureUrls.clear();
    mHaveExposureFiles.clear();
    mHideExposureFiles.clear();

    // Initialise our list of exposures

    QString exposures = QString();
    QRegularExpression filterRegEx = QRegularExpression(pFilter, QRegularExpression::CaseInsensitiveOption);

    mNumberOfFilteredExposures = 0;

    for (int i = 0, iMax = pExposures.count(); i < iMax; ++i) {
        QString exposureUrl = pExposures[i]->url();
        QString exposureName = pExposures[i]->name();
        bool exposureDisplayed = exposureName.contains(filterRegEx);

        exposures += "<tr style=\"display: "+QString(exposureDisplayed?"table-row":"none")+";\">\n"
                     "    <td class=\"exposure\">\n"
                     "        <table class=\"fullWidth\">\n"
                     "            <tbody>\n"
                     "                <tr>\n"
                     "                    <td class=\"fullWidth\">\n"
                     "                        <ul>\n"
                     "                            <li class=\"exposure\">\n"
                     "                                <a href=\""+exposureUrl+"\">"+exposureName+"</a>\n"
                     "                            </li>\n"
                     "                        </ul>\n"
                     "                    </td>\n"
                     "                    <td class=\"button\">\n"
                     "                        <a class=\"noHover\" href=\""+PmrScheme+"://"+CloneWorkspaceAction+"/"+exposureUrl+"\"><img class=\"button clone\"/></a>\n"
                     "                    </td>\n"
                     "                    <td class=\"button\">\n"
                     "                        <a class=\"noHover\" href=\""+PmrScheme+"://"+ShowExposureFilesAction+"/"+exposureUrl+"\"><img id=\"showExposureFilesButton_"+QString::number(i)+"\" class=\"button open\" onclick=\"updateShowExposureFilesButton("+QString::number(i)+", false);\"/></a>\n"
                     "                    </td>\n"
                     "                </tr>\n"
                     "            </tbody>\n"
                     "        </table>\n"
                     "        <ul id=\"exposureFiles_"+QString::number(i)+"\" style=\"display: none;\">\n"
                     "        </ul>\n"
                     "    </td>\n"
                     "</tr>\n";

        mExposureNames << exposureName;
        mExposureDisplayed << exposureDisplayed;
        mExposureUrlId.insert(exposureUrl, i);

        mExposureUrls << exposureUrl;

        mNumberOfFilteredExposures += exposureDisplayed;
    }

    setHtmlSynchronously(mTemplate.arg(message(), exposures));

    mInitialized = true;
}

//==============================================================================

void PmrWindowWidget::filter(const QString &pFilter)
{
    // Filter our list of exposures and remove any duplicates (they will be
    // 'reintroduced' in the next step)

    QStringList filteredExposureNames = mExposureNames.filter(QRegularExpression(pFilter, QRegularExpression::CaseInsensitiveOption));

    mNumberOfFilteredExposures = filteredExposureNames.count();

    filteredExposureNames.removeDuplicates();

    // Update our message and show/hide the relevant exposures

    QString exposuresInfo = QString();

    for (int i = 0, iMax = mExposureNames.count(); i < iMax; ++i) {
        if (mExposureDisplayed[i] != filteredExposureNames.contains(mExposureNames[i])) {
            mExposureDisplayed[i] = !mExposureDisplayed[i];

            exposuresInfo += QString("%1[%2, %3]").arg(exposuresInfo.isEmpty()?QString():", ",
                                                       QString::number(i),
                                                       QString::number(mExposureDisplayed[i]));
        }
    }

    page()->runJavaScript(QString("setMessage(\"%1\");"
                                  "showHideExposures([%2]);").arg(message(), exposuresInfo));
}

//==============================================================================

static const auto OpenExposureFileCommand = "opencor://openFile/";

//==============================================================================

void PmrWindowWidget::addExposureFiles(const QString &pUrl,
                                       const QStringList &pExposureFiles)
{
    // Add the given exposure files to the exposure which URL is given

    static const QRegularExpression FilePathRegEx = QRegularExpression("^.*/");

    QString exposureFiles = QString();

    foreach (const QString &exposureFile, pExposureFiles) {
        exposureFiles += QString("%1[\"%2\", \"%3\"]").arg(exposureFiles.isEmpty()?QString():", ",
                                                           OpenExposureFileCommand+exposureFile,
                                                           QString(exposureFile).remove(FilePathRegEx));
    }

    page()->runJavaScript(QString("addExposureFiles(%1, [%2]);").arg(mExposureUrlId.value(pUrl))
                                                                .arg(exposureFiles));
}

//==============================================================================

void PmrWindowWidget::showExposureFiles(const QString &pUrl, const bool &pShow)
{
    // Show/hide our exposure files

    mHideExposureFiles.insert(pUrl, pShow);

    page()->runJavaScript(QString("showExposureFiles(%1, %2);").arg(mExposureUrlId.value(pUrl))
                                                               .arg(pShow));

    // Update our tool tip (in case the user move the mouse but still remains
    // above the show/hide button)

    linkHovered(PmrScheme+"://"+ShowExposureFilesAction+"/"+pUrl);
}

//==============================================================================

void PmrWindowWidget::copy()
{
    // Copy the URL of the exposure or exposure file to the clipboard

    QApplication::clipboard()->setText(mExposureOrExposureFileUrl);
}

//==============================================================================

void PmrWindowWidget::linkClicked(const QString &pLink)
{
    // Check whether we have clicked a text or button link

    QUrl url = pLink;

    if (!url.scheme().compare(PmrScheme)) {
        // We have clicked on a button link, so let people know whether we want
        // to clone a workspace or whether we want to show/hide exposure files

        QString action = url.authority();
        QString pmrUrl = Core::urlArguments(url);

        if (!action.compare(CloneWorkspaceAction, Qt::CaseInsensitive)) {
            emit cloneWorkspaceRequested(pmrUrl);
        } else if (!action.compare(ShowExposureFilesAction, Qt::CaseInsensitive)) {
            // Show/hide exposure files, if we have them, or let people know
            // that we want to show them

            if (!mHaveExposureFiles.value(pmrUrl)) {
                mHaveExposureFiles.insert(pmrUrl, true);

                emit showExposureFilesRequested(pmrUrl);
            } else {
                showExposureFiles(pmrUrl, !mHideExposureFiles.value(pmrUrl));
            }
        }
    } else {
        // Open an exposure link in the user's browser or ask for an exposure
        // file to be opened in OpenCOR (this will be done automatically thanks
        // to our OpenCOR URL scheme)

        QDesktopServices::openUrl(pLink);
    }
}

//==============================================================================

void PmrWindowWidget::linkHovered(const QString &pLink)
{
    // Update mExposureOrExposureFileUrl and our tool tip based on whether we
    // are hovering a text or button link

    QUrl url = pLink;
    QString linkToolTip;

    if (!url.scheme().compare(PmrScheme)) {
        mExposureOrExposureFileUrl = QString();

        if (!url.host().compare(CloneWorkspaceAction, Qt::CaseInsensitive)) {
            linkToolTip = tr("Clone Workspace");
        } else {
            if (!mHideExposureFiles.value(Core::urlArguments(url)))
                linkToolTip = tr("Show Exposure Files");
            else
                linkToolTip = tr("Hide Exposure Files");
        }
    } else if (!pLink.isEmpty()) {
        mExposureOrExposureFileUrl = pLink;

        if (mExposureUrls.contains(pLink)) {
            linkToolTip = tr("Browse Exposure");
        } else {
            mExposureOrExposureFileUrl.remove(OpenExposureFileCommand, Qt::CaseInsensitive);

            linkToolTip = tr("Open Exposure File");
        }
    } else {
        mExposureOrExposureFileUrl = QString();

        linkToolTip = QString();
    }

    setLinkToolTip(linkToolTip);
}

//==============================================================================

void PmrWindowWidget::showCustomContextMenu()
{
    // Show our context menu to allow the copying of the exposure URL or
    // exposure file URL

    if (!mExposureOrExposureFileUrl.isEmpty())
        mContextMenu->exec(QCursor::pos());
}

//==============================================================================

}   // namespace PMRWindow
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
