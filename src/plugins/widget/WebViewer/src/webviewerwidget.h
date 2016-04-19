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

#pragma once

//==============================================================================

#include "webviewerglobal.h"

//==============================================================================

#include <QString>
#include <QWebEngineView>

//==============================================================================

namespace OpenCOR {
namespace WebViewer {

//==============================================================================

class WebViewerPage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit WebViewerPage(QObject *pParent);

    void setSupportedUrlSchemes(const QStringList &pUrlSupportedSchemes);

protected:
    virtual bool acceptNavigationRequest(const QUrl &pUrl, NavigationType pType,
                                         bool pIsMainFrame);

private:
    QStringList mSupportedUrlSchemes;

Q_SIGNALS:
    void linkClicked(const QString &pLink);
};

//==============================================================================

class WEBVIEWER_EXPORT WebViewerWidget : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebViewerWidget(QWidget *pParent);

    void setSupportedUrlSchemes(const QStringList &pUrlSupportedSchemes);

    void setLinkToolTip(const QString &pLinkToolTip);

    void setUrlSynchronously(const QUrl &pUrl);
    void setHtmlSynchronously(const QString &pHtml,
                              const QUrl &pBaseUrl = QUrl());

protected:
    virtual bool event(QEvent *pEvent);

private:
    WebViewerPage *mPage;

    bool mResettingCursor;

    QString mLinkToolTip;
};

//==============================================================================

}   // namespace WebViewer
}   // namespace OpenCOR

//==============================================================================


//==============================================================================
// End of file
//==============================================================================
