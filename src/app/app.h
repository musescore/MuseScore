/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_APP_APP_H
#define MU_APP_APP_H

#include <QList>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "converter/iconvertercontroller.h"
#include "diagnostics/idiagnosticdrawprovider.h"
#include "autobot/iautobot.h"

#include "commandlinecontroller.h"

namespace mu::app {
class App
{
    INJECT(app, framework::IApplication, muapplication)
    INJECT(app, converter::IConverterController, converter)
    INJECT(app, diagnostics::IDiagnosticDrawProvider, diagnosticDrawProvider)
    INJECT(app, autobot::IAutobot, autobot)

public:
    App();

    void addModule(modularity::IModuleSetup* module);

    int run(int argc, char** argv);

private:

    int processConverter(const CommandLineController::ConverterTask& task);
    int processDiagnostic(const CommandLineController::Diagnostic& task);
    void processAutobot(const CommandLineController::Autobot& task);

    QList<modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APP_APP_H
