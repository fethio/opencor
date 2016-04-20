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
// CLI application
//==============================================================================

#pragma once

//==============================================================================

#include "plugin.h"

//==============================================================================

#include <QStringList>

//==============================================================================

class QCoreApplication;
class QOpenGLContext;

//==============================================================================

namespace OpenCOR {

//==============================================================================

class PluginManager;

//==============================================================================

class CliApplication {
public:
    explicit CliApplication(int &pArgC, char **pArgV);
    ~CliApplication();

    bool run(int *pRes);

private:
    QCoreApplication *mCliApplication;

    PluginManager *mPluginManager;

    Plugins mLoadedCliPlugins;

    QOpenGLContext *mOpenglContext;

    void loadPlugins();

    QString pluginDescription(Plugin *pPlugin) const;

    void about() const;
    bool command(const QStringList &pArguments, int *pRes) const;
    void help() const;
    void plugins() const;
    void reset() const;
    void status() const;
    void version() const;
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
