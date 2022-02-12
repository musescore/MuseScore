/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "saveprojectscenario.h"

using namespace mu;
using namespace mu::project;

RetVal<io::path> SaveProjectScenario::askLocalPath(INotationProjectPtr project, SaveMode saveMode) const
{
    QString dialogTitle = qtrc("project", "Save score");
    QString filenameAddition;

    if (saveMode == SaveMode::SaveCopy) {
        //: used to form a filename suggestion, like "originalFile - copy"
        filenameAddition = " - " + qtrc("project", "copy", "a copy of a file");
    } else if (saveMode == SaveMode::SaveSelection) {
        filenameAddition = " - " + qtrc("project", "selection");
    }

    io::path defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition);

    QString filter = qtrc("project", "MuseScore File") + " (*.mscz)";

    io::path selectedPath = interactive()->selectSavingFile(dialogTitle, defaultPath, filter);

    if (selectedPath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    return RetVal<io::path>::make_ok(selectedPath);
}
