//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_APPSHELL_APPSHELL_H
#define MU_APPSHELL_APPSHELL_H

#include <QList>
#include <QMap>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "framework/commandline/icommandlineregister.h"

class QCommandLineParser;

namespace mu {
namespace appshell {
class AppShell
{
    INJECT(appshell, commandline::ICommandLineRegister, commandlineRegister)
public:
    AppShell();

    void addModule(mu::framework::IModuleSetup* module);

    int run(int argc, char** argv);

private:

    enum class RunMode {
        Gui,
        Concole
    };

    void parseCommandLineArguments(QCommandLineParser& parser);
    void applyCommandLineArguments(QCommandLineParser& parser);

    RunMode runMode(const QStringList &options) const;

    QList<mu::framework::IModuleSetup*> m_modules;

    QList<QString> m_consoleRunModeOptions;

};
}
}

#endif // MU_APPSHELL_APPSHELL_H
