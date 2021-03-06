﻿/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// Busy support widget
//==============================================================================

#pragma once

//==============================================================================

#include "commonwidget.h"
#include "coreglobal.h"

//==============================================================================

#include <QDockWidget>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class BusyWidget;

//==============================================================================

class CORE_EXPORT BusySupportWidget
{
public:
    explicit BusySupportWidget();

    bool isBusyWidgetVisible() const;

    void showBusyWidget(QWidget *pParent);
    void showProgressBusyWidget(QWidget *pParent);

    void showGlobalBusyWidget(QWidget *pParent);
    void showGlobalProgressBusyWidget(QWidget *pParent);

    void hideBusyWidget();

    void resizeBusyWidget();

    void setBusyWidgetProgress(const double &pProgress);

private:
    BusyWidget *mBusyWidget;

    void doShowBusyWidget(QWidget *pParent, const bool &pGlobal = false,
                          const double &pProgress = -1.0);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
