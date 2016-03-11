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
// Web view widget
//==============================================================================

#include "webengineviewwidget.h"

//==============================================================================

#include <QCursor>
#include <QEvent>
#include <QEventLoop>
#include <QHelpEvent>
#include <QToolTip>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

WebEngineViewWidget::WebEngineViewWidget(QWidget *pParent) :
    QWebEngineView(pParent),
    mResettingCursor(false),
    mLinkToolTip(QString())
{
    // Customise ourselves

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Keep track of the currently hovered link, if any

    connect(page(), SIGNAL(linkHovered(const QString &)),
            this, SLOT(linkHovered(const QString &)));
}

//==============================================================================

void WebEngineViewWidget::setLinkToolTip(const QString &pLinkToolTip)
{
    // Set our link tool tip

    mLinkToolTip = pLinkToolTip;
}

//==============================================================================

void WebEngineViewWidget::setUrlSynchronously(const QUrl &pUrl)
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

void WebEngineViewWidget::setHtmlSynchronously(const QString &pHtml,
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

QString WebEngineViewWidget::hoveredLink() const
{
    // Return the currently hovered link, if any

    return mHoveredLink;
}

//==============================================================================

bool WebEngineViewWidget::event(QEvent *pEvent)
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

void WebEngineViewWidget::linkHovered(const QString &pLink)
{
    // Keep track of the currently hovered link, if any

    mHoveredLink = pLink;
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
