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

#ifndef WEBENGINEVIEWWIDGET_H
#define WEBENGINEVIEWWIDGET_H

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include <QString>
#include <QWebEngineView>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT WebEngineViewWidget : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebEngineViewWidget(QWidget *pParent);

    void setLinkToolTip(const QString &pLinkToolTip);

    void setUrlSynchronously(const QUrl &pUrl);
    void setHtmlSynchronously(const QString &pHtml,
                              const QUrl &pBaseUrl = QUrl());

    QString hoveredLink() const;

protected:
    virtual bool event(QEvent *pEvent);

private:
    bool mResettingCursor;

    QString mLinkToolTip;

    QString mHoveredLink;

private Q_SLOTS:
    void linkHovered(const QString &pLink);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
