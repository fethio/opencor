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
// Busy support widget
//==============================================================================

#include "busysupportwidget.h"
#include "busywidget.h"
#include "centralwidget.h"
#include "coreguiutils.h"

//==============================================================================

#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QRect>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

BusySupportWidget::BusySupportWidget() :
    mBusyWidget(0)
{
}

//==============================================================================

bool BusySupportWidget::isBusyWidgetVisible() const
{
    // Return whether our busy widget is visible

    return mBusyWidget?mBusyWidget->isVisible():false;
}

//==============================================================================

void BusySupportWidget::showBusyWidget(QWidget *pParent, const bool &pGlobal,
                                       const double &pProgress)
{
    // Make sure that our previous busy widget, if any, is hidden (and deleted)

    hideBusyWidget();

    // Create and show our new busy widget centered

    mBusyWidget = new BusyWidget(pParent, pGlobal, pProgress);

    centerBusyWidget();

    // Disable our parent widget (or OpenCOR itself in case our parent is the
    // central widget), if any
    // Note: if our parent is the central widget, we really want to disable the
    //       whole application otherwise we could, for example, be loading a
    //       remote file and still be able to switch files, which would mess up
    //       our GUI...

    QWidget *effectiveParentWidget = mBusyWidget->effectiveParentWidget();

    if (effectiveParentWidget) {
        if (effectiveParentWidget == static_cast<QWidget *>(centralWidget()))
            mainWindow()->setEnabled(false);
        else
            effectiveParentWidget->setEnabled(false);
    }

    // Make sure that our busy widget is shown straightaway
    // Note: indeed, depending on the operating system (e.g. OS X) and on what
    //       we do next (e.g. retrieving a remote file), our busy widget may or
    //       not show straightaway...

    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();
}

//==============================================================================

void BusySupportWidget::hideBusyWidget()
{
    // Determine our parent widget, if any, and enable it (or OpenCOR itself in
    // case our parent is the central widget)

    QWidget *effectiveParentWidget = mBusyWidget?mBusyWidget->effectiveParentWidget():0;

    if (effectiveParentWidget) {
        if (effectiveParentWidget == static_cast<QWidget *>(centralWidget()))
            mainWindow()->setEnabled(true);
        else
            effectiveParentWidget->setEnabled(true);
    }

    // Hide our busy widget by deleting it

    delete mBusyWidget;

    mBusyWidget = 0;
}

//==============================================================================

void BusySupportWidget::centerBusyWidget()
{
    // Make sure that we have a busy widget

    if (!mBusyWidget)
        return;

    // Center our busy widget

    QRect desktopGeometry = qApp->desktop()->availableGeometry();
    int parentWidth = mBusyWidget->parentWidget()?mBusyWidget->parentWidget()->width():desktopGeometry.width();
    int parentHeight = mBusyWidget->parentWidget()?mBusyWidget->parentWidget()->height():desktopGeometry.height();

    mBusyWidget->move(0.5*(parentWidth-mBusyWidget->width()),
                      0.5*(parentHeight-mBusyWidget->height()));
}

//==============================================================================

void BusySupportWidget::setBusyWidgetProgress(const double &pProgress)
{
    // Make sure that we have a busy widget

    if (!mBusyWidget)
        return;

    // Set the progress of our busy widget

    mBusyWidget->setProgress(pProgress);
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
