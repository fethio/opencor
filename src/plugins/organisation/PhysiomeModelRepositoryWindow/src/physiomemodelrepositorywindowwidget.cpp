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
// Physiome Model Repository widget
//==============================================================================

#include "coreguiutils.h"
#include "physiomemodelrepositorywindowwidget.h"

//==============================================================================

#include "ui_physiomemodelrepositorywindowwidget.h"

//==============================================================================

#include <QClipboard>
#include <QDesktopServices>
#include <QIODevice>
#include <QLayout>
#include <QMenu>
#include <QRegularExpression>

//==============================================================================

namespace OpenCOR {
namespace PhysiomeModelRepositoryWindow {

//==============================================================================

PhysiomeModelRepositoryWindowExposure::PhysiomeModelRepositoryWindowExposure(const QString &pUrl,
                                                                             const QString &pName) :
    mUrl(pUrl),
    mName(pName)
{
}

//==============================================================================

bool PhysiomeModelRepositoryWindowExposure::operator<(const PhysiomeModelRepositoryWindowExposure &pExposure) const
{
    // Return whether the current exposure is lower than the given one (without
    // worrying about casing)

    return mName.compare(pExposure.name(), Qt::CaseInsensitive) < 0;
}

//==============================================================================

QString PhysiomeModelRepositoryWindowExposure::url() const
{
    // Return our URL

    return mUrl;
}

//==============================================================================

QString PhysiomeModelRepositoryWindowExposure::name() const
{
    // Return our name

    return mName;
}

//==============================================================================

PhysiomeModelRepositoryWindowWidget::PhysiomeModelRepositoryWindowWidget(QWidget *pParent) :
    Core::WebEngineViewWidget(pParent),
    Core::CommonWidget(),
    mGui(new Ui::PhysiomeModelRepositoryWindowWidget),
    mExposureNames(QStringList()),
    mExposureDisplayed(QBoolList()),
    mExposureUrlId(QMap<QString, int>()),
    mInitialized(false),
    mErrorMessage(QString()),
    mInternetConnectionAvailable(true),
    mNumberOfFilteredExposures(0),
    mExposureUrls(QStringList()),
    mHideExposureFiles(QMap<QString, bool>()),
    mExposureUrl(QString())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Customise our layout

    layout()->setMargin(0);
    layout()->setSpacing(0);

    // Create and populate our context menu

    mContextMenu = new QMenu(this);

    mContextMenu->addAction(mGui->actionCopy);

    // We want our own context menu

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu()));

    // Some connections to handle the clicking and hovering of a link

/*---ISSUE908---
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
*/

    connect(page(), SIGNAL(linkClicked(const QUrl &)),
            this, SLOT(linkClicked()));
    connect(page(), SIGNAL(linkHovered(const QString &)),
            this, SLOT(linkHovered(const QString &)));

    // Retrieve the output template

    QByteArray fileContents;

    Core::readFileContentsFromFile(":/output.html", fileContents);

    mTemplate = QString(fileContents).arg(Core::iconDataUri(":/oxygen/places/folder-downloads.png", 16, 16),
                                          Core::iconDataUri(":/oxygen/actions/document-open-remote.png", 16, 16),
                                          "%1", "%2");
}

//==============================================================================

PhysiomeModelRepositoryWindowWidget::~PhysiomeModelRepositoryWindowWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::retranslateUi()
{
    // Retranslate our message, if we have been initialised

    if (mInitialized)
        page()->runJavaScript(QString("setMessage(\"%1\");").arg(message()));
}

//==============================================================================

QSize PhysiomeModelRepositoryWindowWidget::sizeHint() const
{
    // Suggest a default size for our PMR widget
    // Note: this is critical if we want a docked widget, with a PMR widget on
    //       it, to have a decent size when docked to the main window...

    return defaultSize(0.15);
}

//==============================================================================

QString PhysiomeModelRepositoryWindowWidget::message() const
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

void PhysiomeModelRepositoryWindowWidget::initialize(const PhysiomeModelRepositoryWindowExposures &pExposures,
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
    mHideExposureFiles.clear();

    // Initialise our list of exposures

    QString exposures = QString();
    QRegularExpression filterRegEx = QRegularExpression(pFilter, QRegularExpression::CaseInsensitiveOption);

    mNumberOfFilteredExposures = 0;

    for (int i = 0, iMax = pExposures.count(); i < iMax; ++i) {
        QString exposureUrl = pExposures[i].url();
        QString exposureName = pExposures[i].name();
        bool exposureDisplayed = exposureName.contains(filterRegEx);

        exposures += "<tr id=\"exposure_"+QString::number(i)+"\" style=\"display: "+(exposureDisplayed?"table-row":"none")+";\">\n"
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
                     "                        <a class=\"noHover\" href=\""+PmrScheme+"://"+ShowExposureFilesAction+"/"+exposureUrl+"\"><img id=\"exposure_"+QString::number(i)+"\" class=\"button open\"/></a>\n"
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

void PhysiomeModelRepositoryWindowWidget::filter(const QString &pFilter)
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
                                  "showHideExposures(%2);").arg(message(), "["+exposuresInfo+"]"));
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::addExposureFiles(const QString &pUrl,
                                                           const QStringList &pExposureFiles)
{
    // Add the given exposure files to the exposure

Q_UNUSED(pUrl);
Q_UNUSED(pExposureFiles);
//    static const QRegularExpression FilePathRegEx = QRegularExpression("^.*/");
/*---ISSUE908---

    QWebElement ulElement = page()->mainFrame()->documentElement().findFirst(QString("ul[id=exposureFiles_%1]").arg(mExposureUrlId.value(pUrl)));

    foreach (const QString &exposureFile, pExposureFiles) {
        ulElement.appendInside(QString("<li class=\"exposureFile\">"
                                       "    <a href=\"%1\">%2</a>"
                                       "</li>").arg(exposureFile, QString(exposureFile).remove(FilePathRegEx)));
    }
*/
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::showExposureFiles(const QString &pUrl,
                                                            const bool &pShow)
{
Q_UNUSED(pUrl);
Q_UNUSED(pShow);
/*---ISSUE908---
    // Show the exposure files for the given exposure

    int id = mExposureUrlId.value(pUrl);
    QWebElement documentElement = page()->mainFrame()->documentElement();
    QWebElement imgElement = documentElement.findFirst(QString("img[id=exposure_%1]").arg(id));
    QWebElement ulElement = documentElement.findFirst(QString("ul[id=exposureFiles_%1]").arg(id));

    if (pShow) {
        imgElement.removeClass("button");
        imgElement.addClass("downButton");

        ulElement.addClass("visible");
        ulElement.setStyleProperty("display", "table-row");
    } else {
        imgElement.addClass("button");
        imgElement.removeClass("downButton");

        ulElement.removeClass("visible");
        ulElement.setStyleProperty("display", "none");
    }
*/
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::on_actionCopy_triggered()
{
    // Copy the URL of the exposure to the clipboard

    QApplication::clipboard()->setText(mExposureUrl);
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::linkClicked()
{
/*---ISSUE908---
    // Retrieve some information about the link

    QString link;
    QString textContent;

    QWebElement element = retrieveLinkInformation(link, textContent);

    // Check whether we have clicked a text or button link

    if (textContent.isEmpty()) {
        // We have clicked on a button link, so let people know whether we want
        // to clone a workspace or whether we want to show/hide exposure files

        QStringList linkList = link.split("|");

        if (!linkList[0].compare("cloneWorkspace")) {
            emit cloneWorkspace(linkList[1], linkList[2]);
        } else {
            // Show/hide exposure files, if we have them, or let people know
            // that we want to show them

            int id = mExposureUrlId.value(linkList[1]);

            QWebElement documentElement = page()->mainFrame()->documentElement();
            QWebElement ulElement = documentElement.findFirst(QString("ul[id=exposureFiles_%1]").arg(id));

            if (ulElement.firstChild().isNull()) {
                emit showExposureFiles(linkList[1], linkList[2]);
            } else {
                showExposureFiles(linkList[1],
                                  documentElement.findFirst(QString("img[id=exposure_%1]").arg(id)).hasClass("button"));
            }
        }
    } else {
        // Open an exposure link in the user's browser or ask for an exposure
        // file to be opened in OpenCOR

        if (element.parent().hasClass("exposureFile"))
            emit exposureFileOpenRequested(link);
        else
            QDesktopServices::openUrl(link);
    }
*/
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::linkHovered(const QString &pLink)
{
    // Update mExposureUrl and our tool tip based on whether we are hovering a
    // text or button link

    QString linkToolTip = QString();
    QUrl url = pLink;

    if (!url.scheme().compare(PmrScheme)) {
        mExposureUrl = QString();

        if (!url.host().compare(CloneWorkspaceAction, Qt::CaseInsensitive)) {
            linkToolTip = tr("Clone Workspace");
        } else {
            if (mHideExposureFiles.value(url.path()))
                linkToolTip = tr("Show Exposure Files");
            else
                linkToolTip = tr("Hide Exposure Files");
        }
    } else {
        mExposureUrl = pLink;

        if (mExposureUrls.contains(pLink))
            linkToolTip = tr("Browse Exposure");
        else
            linkToolTip = tr("Open Exposure File");
    }

    setLinkToolTip(linkToolTip);
}

//==============================================================================

void PhysiomeModelRepositoryWindowWidget::showCustomContextMenu()
{
    // Show our context menu to allow the copying of the exposure URL

    if (!mExposureUrl.isEmpty())
        mContextMenu->exec(QCursor::pos());
}

//==============================================================================

}   // namespace PhysiomeModelRepositoryWindow
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
