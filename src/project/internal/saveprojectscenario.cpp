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
using namespace mu::framework;
using namespace mu::project;

constexpr int RET_CODE_CHANGE_SAVE_LOCATION_TYPE = 1234;

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

    IF_ASSERT_FAILED(type != SaveLocationType::Undefined) {
        return make_ret(Ret::Code::UnknownError);
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
            case RET_CODE_CHANGE_SAVE_LOCATION_TYPE:
                type = SaveLocationType::Cloud;
                continue;
            default:
                return path.ret;
            }
        }

        case SaveLocationType::Cloud: {
            RetVal<SaveLocation::CloudInfo> info = doAskCloudLocation(project, true, CloudProjectVisibility::Private);
            switch (info.ret.code()) {
            case int(Ret::Code::Ok):
                return RetVal<SaveLocation>::make_ok(SaveLocation(info.val));
            case RET_CODE_CHANGE_SAVE_LOCATION_TYPE:
                type = SaveLocationType::Local;
                continue;
            default:
                return info.ret;
            }
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

    configuration()->setLastSavedProjectsPath(io::dirpath(selectedPath));

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

RetVal<SaveLocation::CloudInfo> SaveProjectScenario::askCloudLocation(INotationProjectPtr project,
                                                                      CloudProjectVisibility defaultVisibility) const
{
    return doAskCloudLocation(project, false, defaultVisibility);
}

RetVal<SaveLocation::CloudInfo> SaveProjectScenario::doAskCloudLocation(INotationProjectPtr project, bool canSaveLocallyInstead,
                                                                        CloudProjectVisibility defaultVisibility) const
{
    // TODO(save-to-cloud): better name?
    QString defaultName = project->displayName();

    UriQuery query("musescore://project/savetocloud");
    query.addParam("canSaveToComputer", Val(canSaveLocallyInstead));
    query.addParam("name", Val(defaultName));
    query.addParam("visibility", Val(defaultVisibility));

    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();
    using Response = QMLSaveToCloudResponse::SaveToCloudResponse;
    auto response = static_cast<Response>(vals["response"].toInt());
    switch (response) {
    case Response::Cancel:
        return make_ret(Ret::Code::Cancel);
    case Response::SaveLocallyInstead:
        return Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE);
    case Response::Ok:
        break;
    }

    QString name = vals["name"].toString();
    auto visibility = static_cast<CloudProjectVisibility>(vals["visibility"].toInt());

    LOGD() << "name: " << name;
    LOGD() << "visibility: " << int(visibility);

    if (visibility == CloudProjectVisibility::Public) {
        if (!warnBeforePublishing()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    return make_ret(Ret::Code::NotImplemented);
}

bool SaveProjectScenario::warnBeforePublishing() const
{
    if (!configuration()->shouldWarnBeforePublishing()) {
        return true;
    }

    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, trc("project", "Publish online"), true)
    };

    IInteractive::Result result = interactive()->warning(
        trc("project", "Publish this score online?"),
        trc("project", "All saved changes will be publicly visible on MuseScore.com. "
                       "If you want to make frequent changes, we recommend saving this "
                       "score privately until you’re ready to share it to the world. "),
        buttons,
        int(IInteractive::Button::Ok),
        IInteractive::Option::WithIcon | IInteractive::Option::WithShowAgain);

    bool publish = result.standardButton() == IInteractive::Button::Ok;
    if (publish && !result.showAgain()) {
        configuration()->setShouldWarnBeforePublishing(false);
    }

    return publish;
}
