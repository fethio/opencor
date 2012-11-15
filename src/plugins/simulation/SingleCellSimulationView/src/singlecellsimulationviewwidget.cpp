//==============================================================================
// Single cell simulation view widget
//==============================================================================

#include "cellmlfilemanager.h"
#include "coreutils.h"
#include "singlecellsimulationviewcontentswidget.h"
#include "singlecellsimulationviewinformationwidget.h"
#include "singlecellsimulationviewsimulation.h"
#include "singlecellsimulationviewsimulationinformationwidget.h"
#include "singlecellsimulationviewwidget.h"
#include "toolbarwidget.h"

//==============================================================================

#include "ui_singlecellsimulationviewwidget.h"

//==============================================================================

#include <QDesktopWidget>
#include <QProgressBar>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>

//==============================================================================

namespace OpenCOR {
namespace SingleCellSimulationView {

//==============================================================================

static const QString OutputTab  = "&nbsp;&nbsp;&nbsp;&nbsp;";
static const QString OutputGood = " style=\"color: green;\"";
static const QString OutputInfo = " style=\"color: navy;\"";
static const QString OutputBad  = " style=\"color: maroon;\"";
static const QString OutputBrLn = "<br/>\n";

//==============================================================================

SingleCellSimulationViewWidget::SingleCellSimulationViewWidget(QWidget *pParent) :
    Widget(pParent),
    mGui(new Ui::SingleCellSimulationViewWidget),
    mCanSaveSettings(false),
    mSolverInterfaces(SolverInterfaces()),
    mCellmlFileRuntime(0),
    mSimulation(0),
    mSimulations(QMap<QString, SingleCellSimulationViewSimulation *>())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create a tool bar widget with different buttons

    Core::ToolBarWidget *toolBarWidget = new Core::ToolBarWidget(this);

    toolBarWidget->addAction(mGui->actionRun);
    toolBarWidget->addAction(mGui->actionPause);
    toolBarWidget->addAction(mGui->actionStop);
    toolBarWidget->addSeparator();
    toolBarWidget->addAction(mGui->actionDebugMode);
    toolBarWidget->addSeparator();
    toolBarWidget->addAction(mGui->actionAdd);
    toolBarWidget->addAction(mGui->actionRemove);
    toolBarWidget->addSeparator();
    toolBarWidget->addAction(mGui->actionCsvExport);

    mGui->layout->addWidget(toolBarWidget);
    mGui->layout->addWidget(Core::newLineWidget(this));

    // Create our splitter

    mSplitter = new QSplitter(Qt::Vertical, this);

    // Create our contents widget and create a connection to keep track of
    // whether we can remove graph panels

    mContentsWidget = new SingleCellSimulationViewContentsWidget(this);

    mContentsWidget->setObjectName("Contents");

    connect(mContentsWidget, SIGNAL(removeGraphPanelsEnabled(const bool &)),
            mGui->actionRemove, SLOT(setEnabled(bool)));

    // Create a simulation output widget with a layout on which we put a
    // separating line and our simulation output list view
    // Note: the separating line is because we remove, for aesthetical reasons,
    //       the border of our simulation output list view...

    QWidget *simulationOutputWidget = new QWidget(this);
    QVBoxLayout *simulationOutputLayout= new QVBoxLayout(simulationOutputWidget);

    simulationOutputLayout->setMargin(0);
    simulationOutputLayout->setSpacing(0);

    simulationOutputWidget->setLayout(simulationOutputLayout);

    mOutput = new QTextEdit(this);

    mOutput->setFrameStyle(QFrame::NoFrame);

    simulationOutputLayout->addWidget(Core::newLineWidget(this));
    simulationOutputLayout->addWidget(mOutput);

    // Populate our splitter and use as much space as possible for it by asking
    // for its height to be that of the desktop's, and then add our splitter to
    // our single cell simulation view widget

    mSplitter->addWidget(mContentsWidget);
    mSplitter->addWidget(simulationOutputWidget);

    mSplitter->setSizes(QList<int>() << qApp->desktop()->screenGeometry().height() << 1);

    mGui->layout->addWidget(mSplitter);

    // Create our (thin) simulation progress widget

    mProgressBar = new QProgressBar(this);

    mProgressBar->setAlignment(Qt::AlignCenter);
    mProgressBar->setFixedHeight(3);
    mProgressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    mProgressBar->setTextVisible(false);

    setProgressBarStyleSheet();

    mGui->layout->addWidget(Core::newLineWidget(this));
    mGui->layout->addWidget(mProgressBar);

    // Make our contents widget our focus proxy

    setFocusProxy(mContentsWidget);
}

//==============================================================================

SingleCellSimulationViewWidget::~SingleCellSimulationViewWidget()
{
    // Delete our model settings

    foreach (SingleCellSimulationViewSimulation *simulation, mSimulations)
        delete simulation;

    // Delete the GUI

    delete mGui;
}

//==============================================================================

void SingleCellSimulationViewWidget::retranslateUi()
{
    // Retranslate the whole view

    mGui->retranslateUi(this);

    // Retranslate our contents widget

    mContentsWidget->retranslateUi();
}

//==============================================================================

static const QString SettingsSizesCount = "SizesCount";
static const QString SettingsSize       = "Size";

//==============================================================================

void SingleCellSimulationViewWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve and set the sizes of our splitter

    int sizesCount = pSettings->value(SettingsSizesCount, 0).toInt();

    if (sizesCount) {
        QList<int> newSizes = QList<int>();

        for (int i = 0; i < sizesCount; ++i)
            newSizes << pSettings->value(SettingsSize+QString::number(i)).toInt();

        mSplitter->setSizes(newSizes);
    }

    // Retrieve the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellSimulationViewWidget::saveSettings(QSettings *pSettings) const
{
    if (!mCanSaveSettings)
        return;

    // Keep track of our splitter sizes

    QList<int> crtSizes = mSplitter->sizes();

    pSettings->setValue(SettingsSizesCount, crtSizes.count());

    for (int i = 0, iMax = crtSizes.count(); i < iMax; ++i)
        pSettings->setValue(SettingsSize+QString::number(i), crtSizes[i]);

    // Keep track of the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellSimulationViewWidget::addSolverInterface(SolverInterface *pSolverInterface)
{
    // Add the solver interface to our list

    if (!mSolverInterfaces.contains(pSolverInterface)) {
        // The solver interface is not yet in our list, so...

        mSolverInterfaces << pSolverInterface;

#ifdef QT_DEBUG
        // Display the solver's properties

        qDebug("---------------------------------------");
        qDebug("'%s' solver:", qPrintable(pSolverInterface->name()));
        qDebug(" - Type: %s", qPrintable(pSolverInterface->typeAsString()));

        Solver::Properties properties = pSolverInterface->properties();

        if (properties.count()) {
            qDebug(" - Properties:");

            Solver::Properties::const_iterator iter = properties.constBegin();
            Solver::Properties::const_iterator iterEnd = properties.constEnd();

            while (iter != iterEnd) {
                QString type;

                switch (iter.value()) {
                case Solver::Double:
                    type = "Double";

                    break;
                case Solver::Integer:
                    type = "Integer";

                    break;
                default:
                    type = "???";
                }

                qDebug("    - %s: %s", qPrintable(iter.key()), qPrintable(type));

                ++iter;
            }
        } else {
            qDebug(" - Properties: none");
        }
#endif
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::clearGraphPanels()
{
    // Ask our contents widget to clear all the graph panels

    mContentsWidget->clearGraphPanels();
}

//==============================================================================

void SingleCellSimulationViewWidget::clearActiveGraphPanel()
{
    // Ask our contents widget to clear the current graph panel

    mContentsWidget->clearActiveGraphPanel();
}

//==============================================================================

void SingleCellSimulationViewWidget::outputStatus(const QString &pStatus)
{
    // Move to the end of the output (just in case...)

    mOutput->moveCursor(QTextCursor::End);

    // Make sure that the output ends as expected and if not then add BrLn to it

    static const QString EndOfOutput = "<br /></p></body></html>";

    if (mOutput->toHtml().right(EndOfOutput.size()).compare(EndOfOutput))
        mOutput->insertHtml(OutputBrLn);

    // Output the status and make sure it's visible

    mOutput->insertHtml(pStatus);
    mOutput->moveCursor(QTextCursor::End);
}

//==============================================================================

void SingleCellSimulationViewWidget::outputStatusError(const QString &pStatusError)
{
    // Output the status error

    outputStatus(OutputTab+"<span"+OutputBad+"><strong>"+tr("Error:")+"</strong> "+pStatusError+".</span>"+OutputBrLn);
}

//==============================================================================

void SingleCellSimulationViewWidget::outputStatusSimulationError(const QString &pStatusSimulationError)
{
    // Output the status simulation error

    outputStatusError(pStatusSimulationError);

    // Leave the simulation mode

    setSimulationMode(false);
}

//==============================================================================

void SingleCellSimulationViewWidget::setRunPauseMode(const bool &pRunEnabled)
{
    // Show/hide our run and pause actions

    mGui->actionRun->setVisible(pRunEnabled);
    mGui->actionPause->setVisible(!pRunEnabled);
}

//==============================================================================

void SingleCellSimulationViewWidget::setSimulationMode(const bool &pEnabled)
{
    // Set our run/pause mode

    setRunPauseMode(!pEnabled);

    // Enable/disable our stop action

    mGui->actionStop->setEnabled(pEnabled);

    // Enable or disable the simulation widget

    mContentsWidget->informationWidget()->simulationWidget()->setEnabled(!pEnabled);
}

//==============================================================================

void SingleCellSimulationViewWidget::initialize(const QString &pFileName)
{
    // Do a few things for the previous model, if needed

    SingleCellSimulationViewSimulationInformationWidget *simulationSettings = mContentsWidget->informationWidget()->simulationWidget();

    if (mSimulation) {
        // Keep track of the simulation settings for the previous model

        mSimulation->fromGui(simulationSettings);

        // Stop keeping track of the simulation's progress

        disconnect(mSimulation, SIGNAL(progress(const double &)),
                   this, SLOT(simulationWorkerProgress(const double &)));
    }

    // Retrieve our simulation settings for the current model, if any

    mSimulation = mSimulations.value(pFileName);

    if (!mSimulation) {
        // No simulation settings currently exist for the model, so create some

        mSimulation = new SingleCellSimulationViewSimulation();

        // Create some connections to keep track of the state of our simulation
        // worker

        connect(mSimulation, SIGNAL(running()),
                this, SLOT(simulationWorkerRunning()));
        connect(mSimulation, SIGNAL(pausing()),
                this, SLOT(simulationWorkerPausing()));
        connect(mSimulation, SIGNAL(stopped()),
                this, SLOT(simulationWorkerStopped()));

        // Create a connection to keep track the simulation's progress

        connect(mSimulation, SIGNAL(progress(const double &)),
                this, SLOT(simulationWorkerProgress(const double &)));

        // Keep track of our simulation settings

        mSimulations.insert(pFileName, mSimulation);
    }

    // (Re-)initialise our GUI with our simulation settings for the current
    // model

    mSimulation->toGui(simulationSettings);

    // Get a runtime for the CellML file

    CellMLSupport::CellmlFile *cellmlFile = CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName);

    mCellmlFileRuntime = cellmlFile->runtime();

    QString status = QString();

    if (!mOutput->document()->isEmpty())
        status += "<hr/>\n";

    status += "<strong>"+pFileName+"</strong>"+OutputBrLn;
    status += OutputTab+"<strong>"+tr("Runtime:")+"</strong> ";

    if (mCellmlFileRuntime->isValid()) {
        QString additionalInformation = QString();

        if (mCellmlFileRuntime->needNlaSolver())
            additionalInformation = " + "+tr("Non-linear algebraic system(s)");

        status += "<span"+OutputGood+">"+tr("valid")+"</span>.<br/>\n";
        status += QString(OutputTab+"<strong>"+tr("Model type:")+"</strong> <span"+OutputInfo+">%1%2</span>."+OutputBrLn).arg((mCellmlFileRuntime->modelType() == CellMLSupport::CellmlFileRuntime::Ode)?tr("ODE"):tr("DAE"),
                                                                                                                              additionalInformation);
    } else {
        status += "<span"+OutputBad+">"+tr("invalid")+"</span>."+OutputBrLn;

        foreach (const CellMLSupport::CellmlFileIssue &issue,
                 mCellmlFileRuntime->issues())
            status += QString(OutputTab+"<span"+OutputBad+"><strong>%1</strong> %2.</span>"+OutputBrLn).arg((issue.type() == CellMLSupport::CellmlFileIssue::Error)?tr("Error:"):tr("Warning:"),
                                                                                                            issue.message());
    }

    outputStatus(status);

    // Enable or disable the run action depending on whether the runtime is
    // valid

    CellMLSupport::CellmlFileVariable *variableOfIntegration = mCellmlFileRuntime->isValid()?mCellmlFileRuntime->variableOfIntegration():0;

    mGui->actionRun->setEnabled(variableOfIntegration);

    // By default, we assume that the runtime isn't valid or that there is no
    // variable of integration, meaning that that we can't show the unit of the
    // variable of integration

    simulationSettings->setUnit("???");

    // Check if we have an invalid runtime and, if so, set the unit to an
    // unknown one and leave

    if (!mCellmlFileRuntime->isValid())
        // Note: no need to output a status error since one will have already
        //       been generated while trying to get the runtime (see above)...

        return;

    // Retrieve the unit of the variable of integration, if any

    if (variableOfIntegration)
        // We have a variable of integration, so we can retrieve its unit

        simulationSettings->setUnit(mCellmlFileRuntime->variableOfIntegration()->unit());
    else
        // We don't have a variable of integration, so...

        outputStatusError(tr("the model must have at least one ODE or DAE"));
}

//==============================================================================

void SingleCellSimulationViewWidget::setProgressBarStyleSheet()
{
    // Customise our progress bar to be a very simple (and fast) one

    QPalette progressBarPalette = qApp->palette();
    QColor progressBarBackground = progressBarPalette.color(QPalette::Window);
    QColor progressBarChunkBackground = progressBarPalette.color(QPalette::Highlight);

    mProgressBar->setStyleSheet(QString("QProgressBar {"
                                        "    background: rgb(%1, %2, %3);"
                                        "    border: 0px;"
                                        "}"
                                        ""
                                        "QProgressBar::chunk {"
                                        "    background: rgb(%4, %5, %6);"
                                        "    border: 0px;"
                                        "}").arg(QString::number(progressBarBackground.red()),
                                                 QString::number(progressBarBackground.green()),
                                                 QString::number(progressBarBackground.blue()),
                                                 QString::number(progressBarChunkBackground.red()),
                                                 QString::number(progressBarChunkBackground.green()),
                                                 QString::number(progressBarChunkBackground.blue())));
}

//==============================================================================

void SingleCellSimulationViewWidget::changeEvent(QEvent *pEvent)
{
    // Default handling of the event

    Widget::changeEvent(pEvent);

    // Check whether the palette has changed and if so then update the colours
    // to be used by our progress bar

    if (pEvent->type() == QEvent::PaletteChange)
        setProgressBarStyleSheet();
}

//==============================================================================

void SingleCellSimulationViewWidget::paintEvent(QPaintEvent *pEvent)
{
    // Default handling of the event

    Widget::paintEvent(pEvent);

    // The view has been painted at least once which means that the sizes of
    // mSplitter are meaningful and, as a consequence, we can save our settings
    // upon leaving OpenCOR

    mCanSaveSettings = true;
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionRun_triggered()
{
    // Start the simulation

    mSimulation->run();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionPause_triggered()
{
    // Pause the simulation

    mSimulation->pause();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionStop_triggered()
{
    // Stop the simulation

    mSimulation->stop();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionDebugMode_triggered()
{
//---GRY--- TO BE DONE...
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionAdd_triggered()
{
    // Ask our contents widget to add a new graph panel

    mContentsWidget->addGraphPanel();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionRemove_triggered()
{
    // Ask our contents widget to remove the current graph panel

    mContentsWidget->removeGraphPanel();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionCsvExport_triggered()
{
//---GRY--- TO BE DONE...
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationWorkerRunning()
{
    // Our simulation worker is running, so reflect it at the GUI level

    setSimulationMode(true);
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationWorkerPausing()
{
    // Our simulation worker is pausing, so reflect it at the GUI level

    setRunPauseMode(true);
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationWorkerStopped()
{
    // Our simulation worker has stopped, so reflect it at the GUI level

    setSimulationMode(false);
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationWorkerProgress(const double &pProgress)
{
    // Our simulation has progressed, so update our progress bar

    mProgressBar->setValue(pProgress);
}

//==============================================================================

}   // namespace SingleCellSimulationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
