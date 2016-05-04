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
// CellML annotation view metadata normal view details widget
//==============================================================================

#include "cellmlannotationvieweditingwidget.h"
#include "cellmlannotationviewmetadatanormalviewdetailswidget.h"
#include "corecliutils.h"
#include "coreguiutils.h"
#include "filemanager.h"
#include "usermessagewidget.h"
#include "webviewerwidget.h"

//==============================================================================

#include "ui_cellmlannotationviewmetadatanormalviewdetailswidget.h"

//==============================================================================

#include <Qt>

//==============================================================================

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QMenu>
#include <QRegularExpression>
#include <QScrollArea>
#include <QString>
#include <QTimer>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

CellmlAnnotationViewMetadataNormalViewDetailsWidget::CellmlAnnotationViewMetadataNormalViewDetailsWidget(CellMLSupport::CellmlFile *pCellmlFile,
                                                                                                         QWidget *pParent) :
    Core::Widget(pParent),
    mCellmlFile(pCellmlFile),
    mGui(new Ui::CellmlAnnotationViewMetadataNormalViewDetailsWidget),
    mItemsCount(0),
    mInitialized(false),
    mElement(0),
    mRdfTripleInformation(QString()),
    mInformationType(None),
    mLookUpRdfTripleInformation(First),
    mRdfTriplesMapping(QMap<QString, CellMLSupport::CellmlFileRdfTriple *>()),
    mResourceUrls(QMap<QString, QString>()),
    mIdUrls(QMap<QString, QString>()),
    mResourceOrIdUrl(QString()),
    mRdfTripleInformationSha1s(QStringList()),
    mRdfTripleInformationSha1(QString()),
    mFirstRdfTripleInformation(QString()),
    mLastRdfTripleInformation(QString()),
    mLink(QString()),
    mTextContent(QString())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create and populate our context menu

    mContextMenu = new QMenu(this);

    mContextMenu->addAction(mGui->actionCopy);

    // Create an output widget that will contain our output message and output
    // for ontological terms

    mOutput = new Core::Widget(this);

    QVBoxLayout *outputLayout = new QVBoxLayout(mOutput);

    outputLayout->setMargin(0);

    mOutput->setLayout(outputLayout);

    // Create our output message (within a scroll area, in case the label is too
    // wide)

    mOutputMessageScrollArea = new QScrollArea(mOutput);

    mOutputMessageScrollArea->setFrameShape(QFrame::NoFrame);
    mOutputMessageScrollArea->setWidgetResizable(true);

    mOutputMessage = new Core::UserMessageWidget(":/oxygen/actions/help-about.png",
                                                 mOutputMessageScrollArea);

    mOutputMessageScrollArea->setWidget(mOutputMessage);

    // Create our output for ontological terms

    QByteArray fileContents;

    Core::readFileContentsFromFile(":/ontologicalTerms.html", fileContents);

    mOutputOntologicalTermsTemplate = QString(fileContents).arg(Core::iconDataUri(":/oxygen/actions/list-remove.png", 16, 16),
                                                                Core::iconDataUri(":/oxygen/actions/list-remove.png", 16, 16, QIcon::Disabled),
                                                                "%1");

    mOutputOntologicalTerms = new WebViewer::WebViewerWidget(mOutput);

    mOutputOntologicalTerms->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mOutputOntologicalTerms, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showCustomContextMenu()));

    connect(mOutputOntologicalTerms->page(), SIGNAL(linkClicked(const QString &)),
            this, SLOT(linkClicked(const QString &)));
    connect(mOutputOntologicalTerms->page(), SIGNAL(linkHovered(const QString &)),
            this, SLOT(linkHovered(const QString &)));

    // Add our output message and output for ontological terms to our output
    // widget

    outputLayout->addWidget(mOutputMessageScrollArea);
    outputLayout->addWidget(mOutputOntologicalTerms);

    // Add our output widget to our main layout

    mGui->layout->addWidget(mOutput);
}

//==============================================================================

CellmlAnnotationViewMetadataNormalViewDetailsWidget::~CellmlAnnotationViewMetadataNormalViewDetailsWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);

    // Retranslate our output message

    mOutputMessage->setMessage(tr("There is no metadata associated with the current CellML element..."));

    // Retranslate our output headers

    updateOutputHeaders();
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::updateOutputHeaders()
{
    // Update our output headers, if we have been initialised

    if (mInitialized) {
        mOutputOntologicalTerms->page()->runJavaScript(QString("setHeaders(\"%1\", \"%2\", \"%3\", \"%4\");").arg(tr("Qualifier"),
                                                                                                                  tr("Resource"),
                                                                                                                  tr("Id"),
                                                                                                                  (mItemsCount == 1)?
                                                                                                                      tr("(1 term)"):
                                                                                                                      tr("(%1 terms)").arg(QLocale().toString(mItemsCount))));
    }
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::additionalGuiUpdates(const QString &pRdfTripleInformation,
                                                                               const InformationType &pInformationType,
                                                                               const Information &pLookUpRdfTripleInformation)
{
    // Update our output headers

    updateOutputHeaders();

    // Show/hide our output message and output for ontological terms

    mOutputMessageScrollArea->setVisible(!mItemsCount);
    mOutputOntologicalTerms->setVisible(mItemsCount);

    // Request for something to be looked up, if needed

    if (pLookUpRdfTripleInformation != No) {
        if (mItemsCount) {
            // Request for the first resource id, the last resource id or an
            // 'old' qualifier, resource or resource id to be looked up

            if (pLookUpRdfTripleInformation == First) {
                // Look up the first resource id

                genericLookUp(mFirstRdfTripleInformation, Id);
            } else if (pLookUpRdfTripleInformation == Last) {
                // Look up the last resource id

                genericLookUp(mLastRdfTripleInformation, Id);
            } else {
                // Look up any 'old' qualifier, resource or resource id

                genericLookUp(pRdfTripleInformation, pInformationType);
            }
        } else {
            // No RDF triple left, so ask for 'nothing' to be looked up
            // Note: we do this to let people know that there is nothing to look
            //       up and that they can 'clean' whatever they use to show a
            //       look up to the user...

            genericLookUp();
        }
    }
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::updateGui(iface::cellml_api::CellMLElement *pElement,
                                                                    const QString &pRdfTripleInformation,
                                                                    const InformationType &pInformationType,
                                                                    const Information &pLookUpRdfTripleInformation)
{
    if (!pElement)
        return;

    // Keep track of the CellML element

    mElement = pElement;

    // Reset various properties
    // Note: we might only do that before adding new items, but then again there
    //       is no need to waste memory...

    mResourceUrls.clear();
    mIdUrls.clear();

    mRdfTripleInformationSha1s.clear();

    mRdfTripleInformationSha1 = QString();

    mRdfTriplesMapping.clear();

    mItemsCount = 0;

    mFirstRdfTripleInformation = QString();
    mLastRdfTripleInformation = QString();

    mOutputOntologicalTerms->setHtmlSynchronously(QString());

    mInitialized = false;

    // Populate our web view, but only if there is at least one RDF triple

    CellMLSupport::CellmlFileRdfTriples rdfTriples = mCellmlFile->rdfTriples(pElement);

    if (rdfTriples.count()) {
        // Add the RDF triples

        QString ontologicalTerms = QString();

        foreach (CellMLSupport::CellmlFileRdfTriple *rdfTriple, rdfTriples)
            ontologicalTerms += addRdfTriple(rdfTriple, false);

        mOutputOntologicalTerms->setHtmlSynchronously(mOutputOntologicalTermsTemplate.arg(ontologicalTerms));

        mInitialized = true;
    }

    // Do additional GUI updates

    additionalGuiUpdates(pRdfTripleInformation, pInformationType,
                         pLookUpRdfTripleInformation);
}

//==============================================================================

static const auto CaScheme = QStringLiteral("ca");
static const auto LookUpQualifierAction = QStringLiteral("lookUpQualifier");
static const auto LookUpResourceAction = QStringLiteral("lookUpResource");
static const auto LookUpIdAction = QStringLiteral("lookUpId");
static const auto RemoveTermAction = QStringLiteral("removeTerm");

//==============================================================================

QString CellmlAnnotationViewMetadataNormalViewDetailsWidget::addRdfTriple(CellMLSupport::CellmlFileRdfTriple *pRdfTriple,
                                                                          const bool &pDirectAddition)
{
    if (!pRdfTriple)
        return QString();

    // Add the item

    ++mItemsCount;

    QString qualifier = (pRdfTriple->modelQualifier() != CellMLSupport::CellmlFileRdfTriple::ModelUnknown)?
                            pRdfTriple->modelQualifierAsString():
                            pRdfTriple->bioQualifierAsString();
    QString rdfTripleInformation = qualifier+"|"+pRdfTriple->resource()+"|"+pRdfTriple->id();
    QString rdfTripleInformationSha1 = Core::sha1(rdfTripleInformation.toUtf8());

    QString ontologicalTerm = "<tr id=\"item_"+rdfTripleInformationSha1+"\">\n"
                              "    <td id=\"qualifier_"+rdfTripleInformationSha1+"\">\n"
                              "        <a href=\""+CaScheme+"://"+LookUpQualifierAction+"/"+rdfTripleInformation+"\">"+qualifier+"</a>\n"
                              "    </td>\n"
                              "    <td id=\"resource_"+rdfTripleInformationSha1+"\">\n"
                              "        <a href=\""+CaScheme+"://"+LookUpResourceAction+"/"+rdfTripleInformation+"\">"+pRdfTriple->resource()+"</a>\n"
                              "    </td>\n"
                              "    <td id=\"id_"+rdfTripleInformationSha1+"\">\n"
                              "        <a href=\""+CaScheme+"://"+LookUpIdAction+"/"+rdfTripleInformation+"\">"+pRdfTriple->id()+"</a>\n"
                              "    </td>\n"
                              "    <td id=\"button_"+rdfTripleInformationSha1+"\">\n"
                              "        <a class=\"noHover\" href=\""+CaScheme+"://"+RemoveTermAction+"/"+rdfTripleInformationSha1+"\"><img class=\"button\"/></a>\n"
                              "    </td>\n"
                              "    <td id=\"disabledButton_"+rdfTripleInformationSha1+"\" style=\"display: none;\">\n"
                              "        <img class=\"disabledButton\"/>\n"
                              "    </td>\n"
                              "</tr>\n";

    // Initialise our web view, if needed, or update it

    if (pDirectAddition) {
        if (mItemsCount == 1) {
            mOutputOntologicalTerms->setHtmlSynchronously(mOutputOntologicalTermsTemplate.arg(ontologicalTerm));

            mInitialized = true;
        } else {
/*---ISSUE908---
            if (mItemsCount == 1)
                mOutputOntologicalTerms->page()->mainFrame()->documentElement().findFirst("tbody").appendInside(ontologicalTerm);
            else
                mOutputOntologicalTerms->page()->mainFrame()->documentElement().findFirst(QString("tr[id=item_%1]").arg(mRdfTripleInformationSha1s.last())).appendOutside(ontologicalTerm);
*/
        }
    }

    // Keep track of some information

    QString resourceUrl = "http://identifiers.org/"+pRdfTriple->resource()+"/?redirect=true";
    QString idUrl = "http://identifiers.org/"+pRdfTriple->resource()+"/"+pRdfTriple->id()+"/?profile=most_reliable&redirect=true";

    mResourceUrls.insert(rdfTripleInformation, resourceUrl);
    mIdUrls.insert(rdfTripleInformation, idUrl);

    mRdfTripleInformationSha1s << rdfTripleInformationSha1;

    mRdfTriplesMapping.insert(rdfTripleInformationSha1, pRdfTriple);

    if (mFirstRdfTripleInformation.isEmpty())
        mFirstRdfTripleInformation = rdfTripleInformation;

    mLastRdfTripleInformation = rdfTripleInformation;

    // Do some additional GUI updates, if needed

    if (pDirectAddition) {
        mLookUpRdfTripleInformation = Last;

        additionalGuiUpdates(QString(), None, mLookUpRdfTripleInformation);
    }

    return pDirectAddition?QString():ontologicalTerm;
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::genericLookUp(const QString &pRdfTripleInformation,
                                                                        const InformationType &pInformationType)
{
    // Retrieve the RDF triple information

    QStringList rdfTripleInformation = pRdfTripleInformation.split("|");
    QString qualifier = pRdfTripleInformation.isEmpty()?QString():rdfTripleInformation[0];
    QString resource = pRdfTripleInformation.isEmpty()?QString():rdfTripleInformation[1];
    QString id = pRdfTripleInformation.isEmpty()?QString():rdfTripleInformation[2];

    // Keep track of the RDF triple information and type

    mRdfTripleInformation = pRdfTripleInformation;

    // (Un)highlight/(un)select our various RDF triple information

/*---ISSUE908---
    static const QString Highlighted = "highlighted";
    static const QString Selected = "selected";

    QWebElement documentElement = mOutputOntologicalTerms->page()->mainFrame()->documentElement();
    QString rdfTripleInformationSha1 = pRdfTripleInformation.isEmpty()?QString():Core::sha1(pRdfTripleInformation.toUtf8());

    if (rdfTripleInformationSha1.compare(mRdfTripleInformationSha1)) {
        if (!mRdfTripleInformationSha1.isEmpty()) {
            documentElement.findFirst(QString("tr[id=item_%1]").arg(mRdfTripleInformationSha1)).removeClass(Highlighted);

            if (mInformationType == Qualifier)
                documentElement.findFirst(QString("td[id=qualifier_%1]").arg(mRdfTripleInformationSha1)).removeClass(Selected);
            else if (mInformationType == Resource)
                documentElement.findFirst(QString("td[id=resource_%1]").arg(mRdfTripleInformationSha1)).removeClass(Selected);
            else if (mInformationType == Id)
                documentElement.findFirst(QString("td[id=id_%1]").arg(mRdfTripleInformationSha1)).removeClass(Selected);
        }

        if (!rdfTripleInformationSha1.isEmpty()) {
            documentElement.findFirst(QString("tr[id=item_%1]").arg(rdfTripleInformationSha1)).addClass(Highlighted);

            if (pInformationType == Qualifier)
                documentElement.findFirst(QString("td[id=qualifier_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
            else if (pInformationType == Resource)
                documentElement.findFirst(QString("td[id=resource_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
            else if (pInformationType == Id)
                documentElement.findFirst(QString("td[id=id_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
        }

        mRdfTripleInformationSha1 = rdfTripleInformationSha1;
    } else if (!rdfTripleInformationSha1.isEmpty()) {
        if (pInformationType == Qualifier) {
            documentElement.findFirst(QString("td[id=qualifier_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
            documentElement.findFirst(QString("td[id=resource_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
            documentElement.findFirst(QString("td[id=id_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
        } else if (pInformationType == Resource) {
            documentElement.findFirst(QString("td[id=qualifier_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
            documentElement.findFirst(QString("td[id=resource_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
            documentElement.findFirst(QString("td[id=id_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
        } else if (pInformationType == Id) {
            documentElement.findFirst(QString("td[id=qualifier_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
            documentElement.findFirst(QString("td[id=resource_%1]").arg(rdfTripleInformationSha1)).removeClass(Selected);
            documentElement.findFirst(QString("td[id=id_%1]").arg(rdfTripleInformationSha1)).addClass(Selected);
        }
    }
*/

    mInformationType = pInformationType;

    // Check whether we have something to look up
    // Note: there is nothing nothing do for Any...

/*---ISSUE908---
    if (mLookUpRdfTripleInformation == First) {
        mOutputOntologicalTerms->page()->triggerAction(QWebPage::MoveToStartOfDocument);
    } else if (mLookUpRdfTripleInformation == Last) {
        // Note #1: normally, we would use
        //             mOutputOntologicalTerms->page()->triggerAction(QWebPage::MoveToEndOfDocument);
        //          but this doesn't work...
        // Note #2: another option would be to use
        //              QWebFrame *outputFrame = mOutputOntologicalTerms->page()->mainFrame();
        //
        //              outputFrame->setScrollBarValue(Qt::Vertical, outputFrame->scrollBarMaximum(Qt::Vertical));
        //          but this doesnt' get us exactly to the bottom of the page...

        QTimer::singleShot(1, this, SLOT(showLastRdfTriple()));
    } else if (mLookUpRdfTripleInformation == No) {
        return;
    }
*/

    // Let people know that we want to look something up

    switch (pInformationType) {
    case None:
        emit noLookUpRequested();

        break;
    case Qualifier:
        emit qualifierLookUpRequested(qualifier);

        break;
    case Resource:
        emit resourceLookUpRequested(resource);

        break;
    case Id:
        emit idLookUpRequested(resource, id);

        break;
    }
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::disableLookUpInformation()
{
    // Disable the looking up of RDF triple information

    mLookUpRdfTripleInformation = No;

    // Update the GUI by pretending to be interested in looking something up

    genericLookUp();
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::showLastRdfTriple()
{
    // Show our last RDF triple by scrolling to the end of the page

/*---ISSUE908---
    QWebFrame *outputFrame = mOutputOntologicalTerms->page()->mainFrame();

    outputFrame->setScrollBarValue(Qt::Vertical, outputFrame->scrollBarMaximum(Qt::Vertical));
*/
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::linkClicked(const QString &pLink)
{
Q_UNUSED(pLink);
/*---ISSUE908---
    // Retrieve some information about the link

    mOutputOntologicalTerms->retrieveLinkInformation(mLink, mTextContent);

    // Check whether we have clicked a resource/id link or a button link

    if (mTextContent.isEmpty()) {
        // Update some information

        CellMLSupport::CellmlFileRdfTriple *rdfTriple = mRdfTriplesMapping.value(mLink);

        QString qualifier = (rdfTriple->modelQualifier() != CellMLSupport::CellmlFileRdfTriple::ModelUnknown)?
                                rdfTriple->modelQualifierAsString():
                                rdfTriple->bioQualifierAsString();
        QString rdfTripleInformation = qualifier+"|"+rdfTriple->resource()+"|"+rdfTriple->id();

        mResourceUrls.remove(rdfTripleInformation);
        mIdUrls.remove(rdfTripleInformation);

        mRdfTripleInformationSha1s.removeOne(mLink);

        mRdfTriplesMapping.remove(mLink);

        --mItemsCount;

        // Determine the 'new' RDF triple information to look up, based on
        // whether there are RDF triples left and whether the current RDF triple
        // is the one being highlighted

        QWebElement rdfTripleElement = mOutputOntologicalTerms->page()->mainFrame()->documentElement().findFirst(QString("tr[id=item_%1]").arg(mLink));

        if (!mItemsCount) {
            mRdfTripleInformation = QString();
            mInformationType = None;
        } else if (!mLink.compare(mRdfTripleInformationSha1)) {
            QWebElement newRdfTripleEment = rdfTripleElement.nextSibling();

            if (newRdfTripleEment.isNull())
                newRdfTripleEment = rdfTripleElement.previousSibling();

            static const QRegularExpression ItemRegEx = QRegularExpression("^item_");

            CellMLSupport::CellmlFileRdfTriple *newRdfTriple = mRdfTriplesMapping.value(newRdfTripleEment.attribute("id").remove(ItemRegEx));
            QString newQualifier = (newRdfTriple->modelQualifier() != CellMLSupport::CellmlFileRdfTriple::ModelUnknown)?
                                       newRdfTriple->modelQualifierAsString():
                                       newRdfTriple->bioQualifierAsString();

            mRdfTripleInformation = newQualifier+"|"+newRdfTriple->resource()+"|"+newRdfTriple->id();

            if (!rdfTripleInformation.compare(mFirstRdfTripleInformation))
                mFirstRdfTripleInformation = mRdfTripleInformation;

            if (!rdfTripleInformation.compare(mLastRdfTripleInformation))
                mLastRdfTripleInformation = mRdfTripleInformation;
        }

        // Remove the RDF triple from our GUI

        rdfTripleElement.removeFromDocument();

        // Do some additional GUI updates

        mLookUpRdfTripleInformation = Any;

        if (!mLink.compare(mRdfTripleInformationSha1)) {
            additionalGuiUpdates(mRdfTripleInformation, mInformationType, mLookUpRdfTripleInformation);
        } else {
            // The looked up information is the same, so no need to look it up
            // again
            // Note: indeed, to look it up again would result in the web view
            //       flashing (since a 'new' web page would be loaded)...

            additionalGuiUpdates(mRdfTripleInformation, mInformationType, No);
        }

        // Remove the RDF triple from the CellML file

        mCellmlFile->rdfTriples().remove(rdfTriple);

        // Let people know that an RDF triple has been removed

        emit rdfTripleRemoved(rdfTriple);
    } else {
        // We have clicked on a qualifier/resource/id link, so start by enabling
        // the looking up of any RDF triple information

        mLookUpRdfTripleInformation = Any;

        // Call our generic look up function

        QStringList rdfTripleInformation = mLink.split("|");

        genericLookUp(mLink,
                      (!rdfTripleInformation[0].compare(mTextContent))?
                          Qualifier:
                          !rdfTripleInformation[1].compare(mTextContent)?
                              Resource:
                              Id);
    }
*/
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::linkHovered(const QString &pLink)
{
    // Update mResourceOrIdUrl and our tool tip based on whether we are hovering
    // a text or button link

    QString linkToolTip;
    QUrl url = pLink;

    if (!url.scheme().compare(CaScheme)) {
        QString urlHost = url.host();

        if (!urlHost.compare(LookUpQualifierAction, Qt::CaseInsensitive)) {
            mResourceOrIdUrl = Core::urlArguments(url).split("|")[0];

            linkToolTip = tr("Look Up Qualifier");
        } else if (!urlHost.compare(LookUpResourceAction, Qt::CaseInsensitive)) {
            mResourceOrIdUrl = mResourceUrls.value(Core::urlArguments(url));

            linkToolTip = tr("Look Up Resource");
        } else if (!urlHost.compare(LookUpIdAction, Qt::CaseInsensitive)) {
            mResourceOrIdUrl = mIdUrls.value(Core::urlArguments(url));

            linkToolTip = tr("Look Up Id");
        } else {
            mResourceOrIdUrl = QString();

            linkToolTip = tr("Remove Term");
        }
    } else {
        mResourceOrIdUrl = QString();

        linkToolTip = QString();
    }

    mOutputOntologicalTerms->setLinkToolTip(linkToolTip);
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::showCustomContextMenu()
{
    // Show our context menu to allow the copying of the URL of the resource or
    // id, but only if we are over a link

    if (!mResourceOrIdUrl.isEmpty())
        mContextMenu->exec(QCursor::pos());
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::on_actionCopy_triggered()
{
    // Copy the qualifier or the URL of the resource or id to the clipboard

    QApplication::clipboard()->setText(mResourceOrIdUrl);
}

//==============================================================================

void CellmlAnnotationViewMetadataNormalViewDetailsWidget::filePermissionsChanged()
{
/*---ISSUE908---
    // Enable or disable the remove buttons for our RDF triples, depending on
    // whether the file is un/locked

    bool fileReadableAndWritable = Core::FileManager::instance()->isReadableAndWritable(mCellmlFile->fileName());
    QWebElement documentElement = mOutputOntologicalTerms->page()->mainFrame()->documentElement();

    foreach (const QString &rdfTripleInformationSha1, mRdfTripleInformationSha1s) {
        documentElement.findFirst(QString("td[id=button_%1]").arg(rdfTripleInformationSha1)).setStyleProperty("display", fileReadableAndWritable?"table-cell":"none");
        documentElement.findFirst(QString("td[id=disabledButton_%1]").arg(rdfTripleInformationSha1)).setStyleProperty("display", !fileReadableAndWritable?"table-cell":"none");
    }
*/
}

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
