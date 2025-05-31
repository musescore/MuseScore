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

#include "savediagnosticfilesscenario.h"

#include <QCursor>
#include <QGuiApplication>

#include "diagnosticfileswriter.h"
#include "translation.h"

using namespace muse::diagnostics;
using namespace muse;

Ret SaveDiagnosticFilesScenario::saveDiagnosticFiles()
{
    if (configuration()->shouldWarnBeforeSavingDiagnosticFiles()) {
        IInteractive::Result result = interactive()->warningSync(
            muse::trc("diagnostics", "Save diagnostic files?"),
            muse::trc("diagnostics", "This will create a .zip file with information about your MuseScore Studio setup "
                                     "to help developers diagnose any problems you are having. "
                                     "You can inspect the contents of this file before sending it to anyone."),
            { IInteractive::Button::Cancel, IInteractive::Button::Save }, IInteractive::Button::Save,
            IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

        if (result.standardButton() != IInteractive::Button::Save) {
            return make_ret(Ret::Code::Cancel);
        }

        configuration()->setShouldWarnBeforeSavingDiagnosticFiles(result.showAgain());
    }

    muse::io::path_t path = interactive()->selectSavingFile(
        muse::qtrc("diagnostics", "Save diagnostic files"),
        configuration()->diagnosticFilesDefaultSavingPath(),
        { "(*.zip)" });

    if (path.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();

    Ret ret = DiagnosticFilesWriter(iocContext()).writeDiagnosticFiles(path);

    qApp->restoreOverrideCursor();

    if (!ret) {
        return ret;
    }

    interactive()->revealInFileBrowser(path);

    return muse::make_ok();
}
