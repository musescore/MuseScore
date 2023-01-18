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
#include "diagnosticsactionscontroller.h"

#include "types/uri.h"

#include "view/diagnosticaccessiblemodel.h"

#include "log.h"

using namespace mu::diagnostics;
using namespace mu::accessibility;
using namespace mu::framework;

static const mu::UriQuery SYSTEM_PATHS_URI("musescore://diagnostics/system/paths?sync=false&modal=false&floating=true");
static const mu::UriQuery PROFILER_URI("musescore://diagnostics/system/profiler?sync=false&modal=false&floating=true");
static const mu::UriQuery NAVIGATION_TREE_URI("musescore://diagnostics/navigation/tree?sync=false&modal=false&floating=true");
static const mu::UriQuery ACCESSIBLE_TREE_URI("musescore://diagnostics/accessible/tree?sync=false&modal=false&floating=true");
static const mu::UriQuery ENGRAVING_ELEMENTS_URI("musescore://diagnostics/engraving/elements?sync=false&modal=false&floating=true");

void DiagnosticsActionsController::init()
{
    dispatcher()->reg(this, "diagnostic-show-paths", [this]() { openUri(SYSTEM_PATHS_URI); });
    dispatcher()->reg(this, "diagnostic-show-profiler", [this]() { openUri(PROFILER_URI); });
    dispatcher()->reg(this, "diagnostic-show-navigation-tree", [this]() { openUri(NAVIGATION_TREE_URI); });
    dispatcher()->reg(this, "diagnostic-show-accessible-tree", [this]() { openUri(ACCESSIBLE_TREE_URI); });
    dispatcher()->reg(this, "diagnostic-accessible-tree-dump", []() { DiagnosticAccessibleModel::dumpTree(); });
    dispatcher()->reg(this, "diagnostic-show-engraving-elements", [this]() { openUri(ENGRAVING_ELEMENTS_URI, false); });
    dispatcher()->reg(this, "diagnostic-save-diagnostic-files", this, &DiagnosticsActionsController::saveDiagnosticFiles);
}

void DiagnosticsActionsController::openUri(const mu::UriQuery& uri, bool isSingle)
{
    if (isSingle && interactive()->isOpened(uri.uri()).val) {
        return;
    }

    interactive()->open(uri);
}

void DiagnosticsActionsController::saveDiagnosticFiles()
{
    Ret ret = saveDiagnosticsScenario()->saveDiagnosticFiles();
    if (!ret) {
        LOGE() << ret.toString();
    }
}
