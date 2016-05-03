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

HelpWindowReply::HelpWindowReply(const QByteArray &pData) :
    mData(pData),
    mLength(pData.length())
{
    setOpenMode(QIODevice::ReadOnly);

    QTimer::singleShot(0, this, &QIODevice::readyRead);
    QTimer::singleShot(0, this, &QIODevice::readChannelFinished);
}

//==============================================================================

qint64 HelpWindowReply::bytesAvailable() const
{
    // Return the amount of bytes available

    return mData.length()+QIODevice::bytesAvailable();
}

//==============================================================================

void HelpWindowReply::close()
{
    // Close ourselves

    QIODevice::close();

    deleteLater();
}

//==============================================================================

qint64 HelpWindowReply::readData(char *pData, qint64 pMaxLength)
{
    // Read the amount of data requested, if possible

    qint64 length = qMin(qint64(mData.length()), pMaxLength);

    if (length) {
        memcpy(pData, mData.constData(), length);

        mData.remove(0, length);
    }

    return length;
}

//==============================================================================

qint64 HelpWindowReply::writeData(const char *pData, qint64 pLength)
{
    Q_UNUSED(pData);
    Q_UNUSED(pLength);

    // No data can be written

    return 0;
}

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
}

//==============================================================================

void HelpWindowUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *pRequest)
{
    // Retrieve, if possible, the requested document and let the request know
    // about it

    QUrl url = pRequest->requestUrl();

    pRequest->reply(QMimeDatabase().mimeTypeForUrl(url).name().toUtf8(),
                    new HelpWindowReply(mHelpEngine->findFile(url).isValid()?
                                            mHelpEngine->fileData(url):
                                            mErrorMessageTemplate.arg(tr("Error"),
                                                                      tr("The following help file could not be found:")+" <strong>"+url.toString()+"</strong>.",
                                                                      tr("Please <a href=\"contactUs.html\">contact us</a> about this error."),
                                                                      Core::copyright()).toUtf8()));
}

//==============================================================================

enum {
    MinimumZoomLevel =  1,
    DefaultZoomLevel = 10
};

//==============================================================================

HelpWindowWidget::HelpWindowWidget(QHelpEngine *pHelpEngine,
                                   const QUrl &pHomePage, QWidget *pParent) :
    WebViewer::WebViewerWidget(pParent),
    Core::CommonWidget(),
    mHelpEngine(pHelpEngine),
    mHomePage(pHomePage),
    mZoomLevel(-1)   // This will ensure that mZoomLevel gets initialised by our
                     // first call to setZoomLevel
{
    // Specify and use our own URL scheme handler

    setSupportedUrlSchemes(QStringList() << "qthelp");

    page()->profile()->installUrlSchemeHandler("qthelp", new HelpWindowUrlSchemeHandler(pHelpEngine, this));

    // Prevent the widget from taking over the scrolling of other widgets

    setFocusPolicy(Qt::NoFocus);

    // Set our initial zoom level to the default value
    // Note: to set mZoomLevel directly is not good enough since one of the
    //       things setZoomLevel does is to set our zoom factor...

    setZoomLevel(DefaultZoomLevel);

    // Some connections

    connect(page(), SIGNAL(linkClicked(const QString &)),
            this, SLOT(linkClicked(const QString &)));

    connect(this, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(urlChanged(const QUrl &)));

    connect(page(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));

    connect(pageAction(QWebEnginePage::Back), SIGNAL(changed()),
            this, SLOT(documentChanged()));
    connect(pageAction(QWebEnginePage::Forward), SIGNAL(changed()),
            this, SLOT(documentChanged()));

    // Go to the home page (synchronously)

    goToHomePage(true);
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

void HelpWindowWidget::goToHomePage(const bool &pSynchronously)
{
    // Go to the home page
    // Note: we use setUrl() rather than load() since the former will ensure
    //       that url() becomes valid straightaway (which is important for
    //       retranslateUi())...

    if (pSynchronously)
        setUrlSynchronously(mHomePage);
    else
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

        WebViewer::WebViewerWidget::mouseReleaseEvent(pEvent);
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

        WebViewer::WebViewerWidget::wheelEvent(pEvent);
    }
}

//==============================================================================

void HelpWindowWidget::linkClicked(const QString &pLink)
{
    // Open the link the default way

    QDesktopServices::openUrl(pLink);
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
