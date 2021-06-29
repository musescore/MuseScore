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

#ifndef MU_APPSHELL_APPSHELL_H
#define MU_APPSHELL_APPSHELL_H

#include <QList>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "converter/iconvertercontroller.h"

#include "commandlinecontroller.h"

namespace mu::appshell {
class AppShell
{
    INJECT(appshell, framework::IApplication, muapplication)
    INJECT(appshell, converter::IConverterController, converter)

public:
    AppShell();

    void addModule(modularity::IModuleSetup* module);

    int run(int argc, char** argv);

private:

    int processConverter(const CommandLineController::ConverterTask& task);

    QList<modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APPSHELL_APPSHELL_H
