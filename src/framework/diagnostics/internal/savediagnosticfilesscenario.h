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
#ifndef MUSE_DIAGNOSTICS_SAVEDIAGNOSTICFILESSCENARIO_H
#define MUSE_DIAGNOSTICS_SAVEDIAGNOSTICFILESSCENARIO_H

#include "isavediagnosticfilesscenario.h"

#include "modularity/ioc.h"
#include "idiagnosticsconfiguration.h"
#include "iinteractive.h"

namespace muse::diagnostics {
class SaveDiagnosticFilesScenario : public ISaveDiagnosticFilesScenario, public Injectable
{
    Inject<diagnostics::IDiagnosticsConfiguration> configuration = { this };
    Inject<muse::IInteractive> interactive = { this };

public:
    SaveDiagnosticFilesScenario(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    muse::Ret saveDiagnosticFiles() override;
};
}

#endif // MUSE_DIAGNOSTICS_SAVEDIAGNOSTICFILESSCENARIO_H
