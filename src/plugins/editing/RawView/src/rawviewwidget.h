//==============================================================================
// Raw view widget
//==============================================================================

#ifndef RAWVIEWWIDGET_H
#define RAWVIEWWIDGET_H

//==============================================================================

#include "widget.h"

//==============================================================================

namespace Ui {
    class RawViewWidget;
}

//==============================================================================

namespace OpenCOR {
namespace RawView {

//==============================================================================

class RawViewWidget : public Core::Widget
{
    Q_OBJECT

public:
    explicit RawViewWidget(QWidget *pParent, const QString &pFileName);

private:
    Ui::RawViewWidget *mUi;
};

//==============================================================================

}   // namespace RawView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
