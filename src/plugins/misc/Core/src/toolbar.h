//==============================================================================
// Toolbar
//==============================================================================

#ifndef TOOLBAR_H
#define TOOLBAR_H

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include <QToolBar>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT ToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget *pParent);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
