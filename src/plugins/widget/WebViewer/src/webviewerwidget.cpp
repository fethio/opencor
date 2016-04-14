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
// Web viewer widget
//==============================================================================

#include "webviewerwidget.h"

//==============================================================================

#include <QCursor>
#include <QEvent>
#include <QEventLoop>
#include <QHelpEvent>
#include <QToolTip>

//==============================================================================

namespace OpenCOR {
namespace WebViewer {

//==============================================================================

WebViewerPage::WebViewerPage(QObject *pParent) :
    QWebEnginePage(pParent),
    mSupportedUrlSchemes(QStringList())
{
}

//==============================================================================

void WebViewerPage::setSupportedUrlSchemes(const QStringList &pUrlSupportedSchemes)
{
    // Set our supported URL schemes

    mSupportedUrlSchemes = pUrlSupportedSchemes;
}

//==============================================================================

bool WebViewerPage::acceptNavigationRequest(const QUrl &pUrl,
                                            NavigationType pType,
                                            bool pIsMainFrame)
{
    Q_UNUSED(pType);
    Q_UNUSED(pIsMainFrame);

    // Accept the navigation request if mSupportedUrlSchemes contains the given
    // URL's scheme, otherwise let the user know that a link has been 'clicked'

    QString urlScheme = pUrl.scheme();

    if (mSupportedUrlSchemes.contains(urlScheme)) {
        return true;
    } else {
        emit linkClicked(pUrl.toString());

        return false;
    }
}

//==============================================================================

WebViewerWidget::WebViewerWidget(QWidget *pParent) :
    QWebEngineView(pParent),
    mPage(new WebViewerPage(this)),
    mResettingCursor(false),
    mLinkToolTip(QString())
{
    // Customise ourselves

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Use our own Web engine page

    setPage(mPage);
}

//==============================================================================

void WebViewerWidget::setSupportedUrlSchemes(const QStringList &pUrlSupportedSchemes)
{
    // Set our supported URL schemes

    mPage->setSupportedUrlSchemes(pUrlSupportedSchemes);
}

//==============================================================================

void WebViewerWidget::setLinkToolTip(const QString &pLinkToolTip)
{
    // Set our link tool tip

    mLinkToolTip = pLinkToolTip;
}

//==============================================================================

void WebViewerWidget::setUrlSynchronously(const QUrl &pUrl)
{
    // Set the given URL synchronously

    setUrl(pUrl);

    QEventLoop eventLoop;

    connect(this, SIGNAL(loadFinished(bool)),
            &eventLoop, SLOT(quit()));

    eventLoop.exec();

    disconnect(this, SIGNAL(loadFinished(bool)),
               &eventLoop, SLOT(quit()));
}

//==============================================================================

void WebViewerWidget::setHtmlSynchronously(const QString &pHtml,
                                           const QUrl &pBaseUrl)
{
    // Set the given HTML code synchronously

    setHtml(pHtml, pBaseUrl);

    QEventLoop eventLoop;

    connect(this, SIGNAL(loadFinished(bool)),
            &eventLoop, SLOT(quit()));

    eventLoop.exec();

    disconnect(this, SIGNAL(loadFinished(bool)),
               &eventLoop, SLOT(quit()));
}

//==============================================================================

bool WebViewerWidget::event(QEvent *pEvent)
{
    // Override the change of the cursor and tool tip when hovering a link

    if (mResettingCursor) {
        return true;
    } else if (    (pEvent->type() == QEvent::CursorChange)
               &&  (cursor().shape() == Qt::IBeamCursor)
               && !mResettingCursor) {
        mResettingCursor = true;

        setCursor(Qt::ArrowCursor);

        mResettingCursor = false;

        return true;
    } else if (pEvent->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(pEvent);

        if (!mLinkToolTip.isEmpty()) {
            QToolTip::showText(helpEvent->globalPos(), mLinkToolTip);
        } else {
            QToolTip::hideText();

            pEvent->ignore();
        }

        return true;
    } else {
        return QWebEngineView::event(pEvent);
    }
}

//==============================================================================

}   // namespace WebViewer
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
