//==============================================================================
// Organisation widget
//==============================================================================

#ifndef ORGANISATIONWIDGET_H
#define ORGANISATIONWIDGET_H

//==============================================================================

#include "dockwidget.h"
#include "coreglobal.h"

//==============================================================================

#include <QObject>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT OrganisationWidget : public DockWidget
{
    Q_OBJECT

public:
    explicit OrganisationWidget(QWidget *pParent);

Q_SIGNALS:
    void filesOpened(const QStringList &pFiles);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
