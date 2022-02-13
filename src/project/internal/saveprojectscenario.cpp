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
    RetVal<SaveLocationType> typeRV = saveLocationType();
    if (!typeRV.ret) {
        return typeRV.ret;
    }

    SaveLocationType type = typeRV.val;

    // The user may switch between Local and Cloud as often as they want
    for (;;) {
        configuration()->setLastUsedSaveLocationType(type);

        switch (type) {
        case SaveLocationType::None:
            return make_ret(Ret::Code::UnknownError);

        case SaveLocationType::Local: {
            RetVal<io::path> path = askLocalPath(project, dialogTitle);
            switch (path.ret.code()) {
            case int(Ret::Code::Ok): {
                return RetVal<SaveLocation>::make_ok(SaveLocation::makeLocal(path.val));
            }
            // TODO: Add a case that changes the `type` and lets the loop rerun
            default:
                return path.ret;
            }
        }

        case SaveLocationType::Cloud: {
            return make_ret(Ret::Code::NotImplemented);
        }
        }
    }

    UNREACHABLE;
    return make_ret(Ret::Code::InternalError);
}

RetVal<SaveLocationType> SaveProjectScenario::saveLocationType() const
{
    bool shouldAsk = configuration()->shouldAskSaveLocationType();
    SaveLocationType lastUsed = configuration()->lastUsedSaveLocationType();
    if (!shouldAsk && lastUsed != SaveLocationType::None) {
        return RetVal<SaveLocationType>::make_ok(lastUsed);
    }

    return askSaveLocationType();
}

RetVal<SaveLocationType> SaveProjectScenario::askSaveLocationType() const
{
    UriQuery query("musescore://project/asksavelocationtype");
    bool shouldAsk = configuration()->shouldAskSaveLocationType();
    query.addParam("askAgain", Val(shouldAsk));

    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();

    bool askAgain = vals["askAgain"].toBool();
    configuration()->setShouldAskSaveLocationType(askAgain);

    SaveLocationType type = static_cast<SaveLocationType>(vals["saveLocationType"].toInt());
    return RetVal<SaveLocationType>::make_ok(type);
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

//RetVal<??> SaveProjectScenario::askOnlineLocation(INotationProjectPtr project) const
//{
//    return
//}
