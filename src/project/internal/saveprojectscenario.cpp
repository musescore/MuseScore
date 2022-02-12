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

RetVal<SaveLocation> SaveProjectScenario::askSaveLocation(INotationProjectPtr project, const QString& dialogTitle) const
{
    // TODO(save-to-cloud): add ability to ask cloud location
    RetVal<io::path> localPath = askLocalPath(project, dialogTitle);
    if (!localPath.ret) {
        return localPath.ret;
    }

    return RetVal<SaveLocation>::make_ok(SaveLocation::makeLocal(localPath.val));
}

RetVal<io::path> SaveProjectScenario::askLocalPath(INotationProjectPtr project, const QString& dialogTitle) const
{
    QString title = dialogTitle.isEmpty() ? qtrc("project", "Save score") : dialogTitle;
    io::path defaultPath = defaultLocalPath(project);
    QString filter = qtrc("project", "MuseScore File") + " (*.mscz)";

    io::path filePath = interactive()->selectSavingFile(title, defaultPath, filter);
    if (filePath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    return RetVal<io::path>::make_ok(filePath);
}

io::path SaveProjectScenario::defaultLocalPath(INotationProjectPtr project) const
{
    io::path folderPath;
    io::path filename;

    SaveLocation saveLocation = project->saveLocation();
    switch (saveLocation.type) {
    case SaveLocationType::None: {
        io::path pathOrNameHint = saveLocation.unsavedInfo().pathOrNameHint;
        if (io::isAbsolute(pathOrNameHint)) {
            folderPath = io::dirpath(pathOrNameHint);
        }
        filename = io::filename(pathOrNameHint, false);
    } break;
    case SaveLocationType::Local: {
        io::path projectPath = saveLocation.localInfo().path;
        folderPath = io::dirpath(projectPath);
        filename = io::filename(projectPath, false);
    } break;
    case SaveLocationType::Cloud: {
        // TODO(save-to-cloud)
        //filename = ???
    } break;
    }

    if (folderPath.empty()) {
        folderPath = configuration()->userProjectsPath();
    }

    if (filename.empty()) {
        filename = qtrc("project", "Untitled");
    }

    return folderPath + "/" + filename + "." + engraving::MSCZ;
}
