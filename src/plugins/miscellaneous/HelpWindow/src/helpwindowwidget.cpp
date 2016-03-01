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
// Help widget
//==============================================================================

#include "corecliutils.h"
#include "helpwindowwidget.h"

//==============================================================================

#include <Qt>

//==============================================================================

#include <QAction>
#include <QDesktopServices>
#include <QHelpEngine>
#include <QIODevice>
#include <QMimeDatabase>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>

//==============================================================================

namespace OpenCOR {
namespace HelpWindow {

//==============================================================================

HelpWindowUrlSchemeHandler::HelpWindowUrlSchemeHandler(QHelpEngine *pHelpEngine,
                                                       QObject *pParent) :
    QWebEngineUrlSchemeHandler(pParent),
    mHelpEngine(pHelpEngine)
{
    // Retrieve the error message template

    QByteArray fileContents;

    Core::readFileContentsFromFile(":/helpWindowWidgetError.html", fileContents);

    mErrorMessageTemplate = fileContents;

    // Customise and open our buffer

    mBuffer.setBuffer(&mBufferData);

    mBuffer.open(QIODevice::ReadOnly);
}

//==============================================================================

void HelpWindowUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *pRequest)
{
    // Retrieve, if possible, the requested document

    QUrl url = pRequest->requestUrl();
    mBufferData = mHelpEngine->findFile(url).isValid()?
                      mHelpEngine->fileData(url):
                      mErrorMessageTemplate.arg(tr("Error"),
                                                tr("The following help file could not be found:")+" <strong>"+url.toString()+"</strong>.",
                                                tr("Please <a href=\"contactUs.html\">contact us</a> about this error."),
                                                Core::copyright()).toUtf8();

    // Let the request know about our reply

    pRequest->reply(QMimeDatabase().mimeTypeForUrl(url).name().toUtf8(), &mBuffer);
}

//==============================================================================

HelpWindowPage::HelpWindowPage(QObject *pParent) :
    QWebEnginePage(pParent)
{
}

//==============================================================================

bool HelpWindowPage::acceptNavigationRequest(const QUrl &pUrl,
                                             NavigationType pType,
                                             bool pIsMainFrame)
{
    Q_UNUSED(pType);
    Q_UNUSED(pIsMainFrame);

    // Determine whether the URL refers to an OpenCOR document (qthelp://...) or
    // an external resource of sorts (e.g. http[s]://... and opencor://...), and
    // if it is the latter then just open the URL the default way

    QString urlScheme = pUrl.scheme();

    if (!urlScheme.compare("qthelp")) {
        return true;
    } else {
        QDesktopServices::openUrl(pUrl);

        return false;
    }
}

//==============================================================================

enum {
    MinimumZoomLevel =  1,
    DefaultZoomLevel = 10
};

//==============================================================================

HelpWindowWidget::HelpWindowWidget(QHelpEngine *pHelpEngine,
                                   const QUrl &pHomePage, QWidget *pParent) :
    QWebEngineView(pParent),
    Core::CommonWidget(pParent),
    mHelpEngine(pHelpEngine),
    mHomePage(pHomePage),
    mZoomLevel(-1)   // This will ensure that mZoomLevel gets initialised by our
                     // first call to setZoomLevel
{
    // Use our own help page and URL scheme handler

    setPage(new HelpWindowPage(this));

    page()->profile()->installUrlSchemeHandler("qthelp", new HelpWindowUrlSchemeHandler(pHelpEngine, this));

    // Prevent objects from being dropped on us
    // Note: by default, QWebEngineView allows for objects to be dropped on
    //       itself, while we don't want that...

    setAcceptDrops(false);

    // Prevent the widget from taking over the scrolling of other widgets

    setFocusPolicy(Qt::NoFocus);

    // Set our initial zoom level to the default value
    // Note: to set mZoomLevel directly is not good enough since one of the
    //       things setZoomLevel does is to set our zoom factor...

    setZoomLevel(DefaultZoomLevel);

    // Some connections

    connect(this, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(urlChanged(const QUrl &)));

    connect(page(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));

    connect(pageAction(QWebEnginePage::Back), SIGNAL(changed()),
            this, SLOT(documentChanged()));
    connect(pageAction(QWebEnginePage::Forward), SIGNAL(changed()),
            this, SLOT(documentChanged()));

    // Go to the home page

    goToHomePage();
}

//==============================================================================

void HelpWindowWidget::retranslateUi()
{
    // Retranslate the current document, but only if it is an error document
    // since a valid document is hard-coded and therefore cannot be translated
    // Note: we use setUrl() rather than reload() since the latter won't work
    //       upon starting OpenCOR with a non-system locale...

    if (!mHelpEngine->findFile(url()).isValid())
        setUrl(url());
}

//==============================================================================

static const auto SettingsZoomLevel = QStringLiteral("ZoomLevel");

//==============================================================================

void HelpWindowWidget::loadSettings(QSettings *pSettings)
{
    // Let the user know of a few default things about ourselves by emitting a
    // few signals

    emit notHomePage(false);

    emit backEnabled(false);
    emit forwardEnabled(false);

    emit copyTextEnabled(false);

    emitZoomRelatedSignals();

    // Retrieve the zoom level

    setZoomLevel(pSettings->value(SettingsZoomLevel, DefaultZoomLevel).toInt());
}

//==============================================================================

void HelpWindowWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the text size multiplier

    pSettings->setValue(SettingsZoomLevel, mZoomLevel);
}

//==============================================================================

void HelpWindowWidget::goToHomePage()
{
    // Go to the home page
    // Note: we use setUrl() rather than load() since the former will ensure
    //       that url() becomes valid straightaway (which is important for
    //       retranslateUi()) and that the document gets loaded immediately...

    setUrl(mHomePage);
}

//==============================================================================

void HelpWindowWidget::resetZoom()
{
    // Reset the zoom level

    setZoomLevel(DefaultZoomLevel);
}

//==============================================================================

void HelpWindowWidget::zoomIn()
{
    // Zoom in the help document contents

    setZoomLevel(mZoomLevel+1);
}

//==============================================================================

void HelpWindowWidget::zoomOut()
{
    // Zoom out the help document contents

    setZoomLevel(qMax(int(MinimumZoomLevel), mZoomLevel-1));
}

//==============================================================================

void HelpWindowWidget::emitZoomRelatedSignals()
{
    // Let the user know whether we are not at the default zoom level and
    // whether we can still zoom out

    emit notDefaultZoomLevel(mZoomLevel != DefaultZoomLevel);
    emit zoomOutEnabled(mZoomLevel != MinimumZoomLevel);
}

//==============================================================================

void HelpWindowWidget::setZoomLevel(const int &pZoomLevel)
{
    if (pZoomLevel == mZoomLevel)
        return;

    // Set the zoom level of the help document contents to a particular value

    mZoomLevel = pZoomLevel;

    setZoomFactor(0.1*mZoomLevel);

    // Emit a few zoom-related signals

    emitZoomRelatedSignals();
}

//==============================================================================

QSize HelpWindowWidget::sizeHint() const
{
    // Suggest a default size for the help widget
    // Note: this is critical if we want a docked widget, with a help widget on
    //       it, to have a decent size when docked to the main window...

    return defaultSize(0.2);
}

//==============================================================================

void HelpWindowWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    // Handle some special mouse buttons for navigating the help

    if (pEvent->button() == Qt::XButton1) {
        // Special mouse button #1 which is used to go to the previous help
        // document

        triggerPageAction(QWebEnginePage::Back);

        // Accept the event

        pEvent->accept();
    } else if (pEvent->button() == Qt::XButton2) {
        // Special mouse button #2 which is used to go to the next help document

        triggerPageAction(QWebEnginePage::Forward);

        // Accept the event

        pEvent->accept();
    } else {
        // Something else, so use the default handling of the event

        QWebEngineView::mouseReleaseEvent(pEvent);
    }
}

//==============================================================================

void HelpWindowWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Handle the wheel mouse button for zooming in/out the help document
    // contents

    if (pEvent->modifiers() == Qt::ControlModifier) {
        int delta = pEvent->delta();

        if (delta > 0)
            zoomIn();
        else if (delta < 0)
            zoomOut();

        pEvent->accept();
    } else {
        // Not the modifier we were expecting, so call the default handling of
        // the event

        QWebEngineView::wheelEvent(pEvent);
    }
}

//==============================================================================

void HelpWindowWidget::paintEvent(QPaintEvent *pEvent)
{
    // Default handling of the event

    QWebEngineView::paintEvent(pEvent);

    // Draw a border

    drawBorder(
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
               true, true, true, true,
#elif defined(Q_OS_MAC)
               true, false, false, false,
#else
    #error Unsupported platform
#endif
               true, false, false, false
              );
}

//==============================================================================

void HelpWindowWidget::urlChanged(const QUrl &pUrl)
{
    // The URL has changed, so let the user know whether it's the home page

    emit notHomePage(pUrl != mHomePage);
}

//==============================================================================

void HelpWindowWidget::selectionChanged()
{
    // The text selection has changed, so let the user know whether some text is
    // now selected

    emit copyTextEnabled(!selectedText().isEmpty());
}

//==============================================================================

void HelpWindowWidget::documentChanged()
{
    // A new help document has been selected, resulting in the previous or next
    // help document becoming either available or not

    QAction *action = qobject_cast<QAction *>(sender());

    if (action == pageAction(QWebEnginePage::Back))
        emit backEnabled(action->isEnabled());
    else if (action == pageAction(QWebEnginePage::Forward))
        emit forwardEnabled(action->isEnabled());
}

//==============================================================================

}   // namespace HelpWindow
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
