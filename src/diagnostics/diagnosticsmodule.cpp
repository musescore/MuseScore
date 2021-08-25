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
#include "diagnosticsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/diagnosticsactions.h"
#include "internal/diagnosticsactionscontroller.h"
#include "internal/diagnosticspathsregister.h"

#include "view/diagnosticspathsmodel.h"

#include "view/keynav/diagnosticnavigationmodel.h"
#include "view/keynav/abstractkeynavdevitem.h"
#include "view/keynav/keynavdevsection.h"
#include "view/keynav/keynavdevsubsection.h"
#include "view/keynav/keynavdevcontrol.h"

#include "view/diagnosticaccessiblemodel.h"

using namespace mu::diagnostics;
using namespace mu::modularity;

static std::shared_ptr<DiagnosticsActionsController> s_actionsController = std::make_shared<DiagnosticsActionsController>();

std::string DiagnosticsModule::moduleName() const
{
    return "diagnostics";
}

void DiagnosticsModule::registerExports()
{
    ioc()->registerExport<IDiagnosticsPathsRegister>(moduleName(), new DiagnosticsPathsRegister());
}

void DiagnosticsModule::resolveImports()
{
    auto ir = ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://diagnostics/system/paths"), "MuseScore/Diagnostics/DiagnosticPathsDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/navigation/tree"), "MuseScore/Diagnostics/DiagnosticNavigationDialog.qml");
        ir->registerQmlUri(Uri("musescore://diagnostics/accessible/tree"), "MuseScore/Diagnostics/DiagnosticAccessibleDialog.qml");
    }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<DiagnosticsActions>());
    }
}

void DiagnosticsModule::registerUiTypes()
{
    qmlRegisterType<DiagnosticsPathsModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticsPathsModel");

    qmlRegisterType<DiagnosticNavigationModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticNavigationModel");
    qmlRegisterUncreatableType<AbstractKeyNavDevItem>("MuseScore.Diagnostics", 1, 0, "AbstractKeyNavDevItem", "Cannot create a Abstract");
    qmlRegisterUncreatableType<KeyNavDevSubSection>("MuseScore.Diagnostics", 1, 0, "KeyNavDevSubSection", "Cannot create");
    qmlRegisterUncreatableType<KeyNavDevSection>("MuseScore.Diagnostics", 1, 0, "KeyNavDevSection", "Cannot create a KeyNavDevSection");
    qmlRegisterUncreatableType<KeyNavDevControl>("MuseScore.Diagnostics", 1, 0, "KeyNavDevControl", "Cannot create a KeyNavDevControl");

    qmlRegisterType<DiagnosticAccessibleModel>("MuseScore.Diagnostics", 1, 0, "DiagnosticAccessibleModel");
}

void DiagnosticsModule::onInit(const framework::IApplication::RunMode&)
{
    s_actionsController->init();
}
