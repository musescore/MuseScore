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

RetVal<SaveLocation> SaveProjectScenario::askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                                          SaveLocationType preselectedType) const
{
    SaveLocationType type = preselectedType;

    if (type == SaveLocationType::Undefined) {
        RetVal<SaveLocationType> askedType = saveLocationType();
        if (!askedType.ret) {
            return askedType.ret;
        }

        type = askedType.val;
    }

    // The user may switch between Local and Cloud as often as they want
    for (;;) {
        configuration()->setLastUsedSaveLocationType(type);

        switch (type) {
        case SaveLocationType::Undefined:
            return make_ret(Ret::Code::UnknownError);

        case SaveLocationType::Local: {
            RetVal<io::path> path = askLocalPath(project, mode);
            switch (path.ret.code()) {
            case int(Ret::Code::Ok): {
                SaveLocation::LocalInfo localInfo { path.val };
                return RetVal<SaveLocation>::make_ok(SaveLocation(localInfo));
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

RetVal<SaveLocationType> SaveProjectScenario::saveLocationType() const
{
    bool shouldAsk = configuration()->shouldAskSaveLocationType();
    SaveLocationType lastUsed = configuration()->lastUsedSaveLocationType();
    if (!shouldAsk && lastUsed != SaveLocationType::Undefined) {
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
