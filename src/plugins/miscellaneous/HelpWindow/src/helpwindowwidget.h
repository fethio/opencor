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

#ifndef HELPWINDOWWIDGET_H
#define HELPWINDOWWIDGET_H

//==============================================================================

#include "commonwidget.h"
#include "webviewerwidget.h"

//==============================================================================

#include <QWebEngineUrlSchemeHandler>

//==============================================================================

class QHelpEngine;

//==============================================================================

namespace OpenCOR {
namespace HelpWindow {

//==============================================================================

class HelpWindowReply : public QIODevice
{
public:
    HelpWindowReply(const QByteArray &pData);

    virtual qint64 bytesAvailable() const;

    virtual void close();

protected:
    virtual qint64 readData(char *pData, qint64 pMaxLength);
    virtual qint64 writeData(const char *pData, qint64 pLength);

private:
    QByteArray mData;
    int mLength;
};

//==============================================================================

class HelpWindowUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    explicit HelpWindowUrlSchemeHandler(QHelpEngine *pHelpEngine,
                                        QObject *pParent);

protected:
    virtual void requestStarted(QWebEngineUrlRequestJob *pRequest);

private:
    QHelpEngine *mHelpEngine;

    QString mErrorMessageTemplate;
};

//==============================================================================

class HelpWindowWidget : public WebViewer::WebViewerWidget,
                         public Core::CommonWidget
{
    Q_OBJECT

public:
    explicit HelpWindowWidget(QHelpEngine *pHelpEngine, const QUrl &pHomePage,
                              QWidget *pParent);

    virtual void retranslateUi();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    void goToHomePage(const bool &pSynchronously = false);

    void resetZoom();

    void zoomIn();
    void zoomOut();

protected:
    virtual QSize sizeHint() const;

    virtual void mouseReleaseEvent(QMouseEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);

private:
    QHelpEngine *mHelpEngine;

    QUrl mHomePage;

    int mZoomLevel;

    void emitZoomRelatedSignals();

    void setZoomLevel(const int &pZoomLevel);

Q_SIGNALS:
    void notHomePage(const bool &pNotHomePage);

    void backEnabled(const bool &pEnabled);
    void forwardEnabled(const bool &pEnabled);

    void copyTextEnabled(const bool &pEnabled);

    void notDefaultZoomLevel(const bool &pEnabled);
    void zoomOutEnabled(const bool &pEnabled);

private Q_SLOTS:
    void linkClicked(const QString &pLink);

    void urlChanged(const QUrl &pUrl);

    void selectionChanged();

    void documentChanged();
};

//==============================================================================

}   // namespace HelpWindow
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
