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
// CellML annotation view metadata details widget
//==============================================================================

#ifndef CELLMLANNOTATIONVIEWMETADATADETAILSWIDGET_H
#define CELLMLANNOTATIONVIEWMETADATADETAILSWIDGET_H

//==============================================================================

#include "cellmlfile.h"
#include "corecliutils.h"
#include "widget.h"

//==============================================================================

class QSplitter;
class QWebView;

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace Core {
    class BorderedWidget;
    class UserMessageWidget;
}   // namespace Core

//==============================================================================

namespace CellMLAnnotationView {

//==============================================================================

class CellmlAnnotationViewEditingWidget;
class CellmlAnnotationViewMetadataEditDetailsWidget;
class CellmlAnnotationViewMetadataViewDetailsWidget;
class CellMLAnnotationViewPlugin;
class CellmlAnnotationViewWidget;

//==============================================================================

class CellmlAnnotationViewMetadataDetailsWidget : public Core::Widget
{
    Q_OBJECT

public:
    explicit CellmlAnnotationViewMetadataDetailsWidget(CellMLAnnotationViewPlugin *pPlugin,
                                                       CellmlAnnotationViewWidget *pViewWidget,
                                                       CellmlAnnotationViewEditingWidget *pViewEditingWidget,
                                                       CellMLSupport::CellmlFile *pCellmlFile,
                                                       QWidget *pParent);

    virtual void retranslateUi();

    QSplitter * splitter() const;

    CellmlAnnotationViewMetadataEditDetailsWidget * metadataEditDetails() const;
    CellmlAnnotationViewMetadataViewDetailsWidget * metadataViewDetails() const;

    void filePermissionsChanged();

private:
    CellMLAnnotationViewPlugin *mPlugin;

    Core::BorderedWidget *mBorderedCategoryMessage;
    Core::UserMessageWidget *mCategoryMessage;

    Core::BorderedWidget *mBorderedUnsupportedMetadataMessage;
    Core::UserMessageWidget *mUnsupportedMetadataMessage;

    QSplitter *mSplitter;

    Core::BorderedWidget *mBorderedMetadataEditDetails;
    Core::BorderedWidget *mBorderedMetadataViewDetails;
    Core::BorderedWidget *mBorderedWebView;

    CellmlAnnotationViewMetadataEditDetailsWidget *mMetadataEditDetails;
    CellmlAnnotationViewMetadataViewDetailsWidget *mMetadataViewDetails;
    QWebView *mWebView;

    CellMLSupport::CellmlFile *mCellmlFile;

    ObjRef<iface::cellml_api::CellMLElement> mElement;

    void retranslateUnsupportedMetadataMessage();

Q_SIGNALS:
    void splitterMoved(const QIntList &pSizes);


    void qualifierDetailsRequested(QWebView *pWebView,
                                   const QString &pQualifier);
    void resourceDetailsRequested(QWebView *pWebView, const QString &pResource);
    void idDetailsRequested(QWebView *pWebView, const QString &pResource,
                            const QString &pId);

public Q_SLOTS:
    void updateGui(iface::cellml_api::CellMLElement *pElement);

private Q_SLOTS:
    void emitSplitterMoved();

    void lookUpQualifier(const QString &pQualifier);
    void lookUpResource(const QString &pResource);
    void lookUpId(const QString &pResource, const QString &pId);
    void lookUpNothing();

    void removeAllMetadata();

    void updateMetadataEditDetails();
};

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
