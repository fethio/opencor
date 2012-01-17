//==============================================================================
// Computer engine class
//==============================================================================

#ifndef COMPUTERENGINE_H
#define COMPUTERENGINE_H

//==============================================================================

#include "computerexternalfunction.h"
#include "computerfunction.h"
#include "computerglobal.h"
#include "computerissue.h"

//==============================================================================

#include <QStringList>

//==============================================================================

namespace llvm {
    class ExecutionEngine;
    class Function;
    class Module;
}

//==============================================================================

namespace OpenCOR {
namespace Computer {

//==============================================================================

class ComputerParser;

//==============================================================================

class COMPUTER_EXPORT ComputerEngine : public QObject
{
    Q_OBJECT

public:
    explicit ComputerEngine();
    ~ComputerEngine();

    llvm::Module * module();
    llvm::ExecutionEngine * executionEngine();

    ComputerIssue error();
    ComputerIssues parserErrors();

    llvm::Function * addFunction(const QString &pFunction);

private:
    llvm::Module *mModule;
    llvm::ExecutionEngine *mExecutionEngine;

    ComputerIssue mError;

    ComputerParser *mParser;

    ComputerExternalFunctions mExternalFunctions;

    bool compileFunction(ComputerFunction &pFunction);
};

//==============================================================================

}   // namespace Computer
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
