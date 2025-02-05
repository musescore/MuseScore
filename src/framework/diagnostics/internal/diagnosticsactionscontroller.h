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
#ifndef MUSE_DIAGNOSTICS_DIAGNOSTICSACTIONSCONTROLLER_H
#define MUSE_DIAGNOSTICS_DIAGNOSTICSACTIONSCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "iinteractive.h"
#include "accessibility/iaccessibilitycontroller.h"
#include "isavediagnosticfilesscenario.h"

namespace muse::diagnostics {
class DiagnosticsActionsController : public Injectable, public actions::Actionable
{
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<IInteractive> interactive = { this };
    Inject<diagnostics::ISaveDiagnosticFilesScenario> saveDiagnosticsScenario = { this };

public:
    DiagnosticsActionsController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

private:
    void openUri(const muse::UriQuery& uri, bool isSingle = true);
    void saveDiagnosticFiles();

    void onActionQuery(const actions::ActionQuery& q);
};
}

#endif // MUSE_DIAGNOSTICS_DIAGNOSTICSACTIONSCONTROLLER_H
