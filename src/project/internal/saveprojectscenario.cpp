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

#include "cloud/clouderrors.h"
#include "engraving/infrastructure/mscio.h"

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
            RetVal<io::path_t> path = askLocalPath(project, mode);
            switch (path.ret.code()) {
            case int(Ret::Code::Ok): {
                return RetVal<SaveLocation>::make_ok(SaveLocation(path.val));
            }
            case RET_CODE_CHANGE_SAVE_LOCATION_TYPE:
                type = SaveLocationType::Cloud;
                continue;
            default:
                return path.ret;
            }
        }

        case SaveLocationType::Cloud: {
            RetVal<CloudProjectInfo> info = askCloudLocation(project, mode);
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
}

RetVal<io::path_t> SaveProjectScenario::askLocalPath(INotationProjectPtr project, SaveMode saveMode) const
{
    QString dialogTitle = qtrc("project/save", "Save score");
    std::string filenameAddition;

    if (saveMode == SaveMode::SaveCopy) {
        //: used to form a filename suggestion, like "originalFile - copy"
        filenameAddition = " - " + trc("project/save", "copy", "a copy of a file");
    } else if (saveMode == SaveMode::SaveSelection) {
        //: used to form a filename suggestion, like "originalFile - selection"
        filenameAddition = " - " + trc("project/save", "selection");
    }

    io::path_t defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition);

    std::vector<std::string> filter {
        trc("project", "MuseScore file") + " (*.mscz)",
        trc("project", "Uncompressed MuseScore folder (experimental)")
#ifdef Q_OS_MAC
        + " (*)"
#else
        + " (*.)"
#endif
    };

    io::path_t selectedPath = interactive()->selectSavingFile(dialogTitle, defaultPath, filter);

    if (selectedPath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    if (!engraving::isMuseScoreFile(io::suffix(selectedPath))) {
        // Then it must be that the user is trying to save a mscx file.
        // At the selected path, a folder will be created,
        // and inside the folder, a mscx file will be created.
        // We should return the path to the mscx file.
        selectedPath = selectedPath.appendingComponent(io::filename(selectedPath)).appendingSuffix(engraving::MSCX);
    }

    configuration()->setLastSavedProjectsPath(io::dirpath(selectedPath));

    return RetVal<io::path_t>::make_ok(selectedPath);
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

RetVal<CloudProjectInfo> SaveProjectScenario::askCloudLocation(INotationProjectPtr project, SaveMode mode) const
{
    return doAskCloudLocation(project, mode, false);
}

RetVal<CloudProjectInfo> SaveProjectScenario::askPublishLocation(INotationProjectPtr project) const
{
    return doAskCloudLocation(project, SaveMode::Save, true);
}

RetVal<CloudProjectInfo> SaveProjectScenario::doAskCloudLocation(INotationProjectPtr project, SaveMode mode, bool isPublish) const
{
    bool isCloudAvailable = authorizationService()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        return warnCloudIsNotAvailable(isPublish);
    }

    Ret ret = authorizationService()->ensureAuthorization(
        isPublish
        ? trc("project/save", "Login or create a free account on musescore.com to save this score to the cloud.")
        : trc("project/save", "Login or create a free account on musescore.com to publish this score."));
    if (!ret) {
        return ret;
    }

    QString defaultName = project->displayName();
    cloud::Visibility defaultVisibility = isPublish ? cloud::Visibility::Public : cloud::Visibility::Private;
    const QUrl existingOnlineScoreUrl = project->cloudInfo().sourceUrl;

    if (!existingOnlineScoreUrl.isEmpty()) {
        RetVal<cloud::ScoreInfo> scoreInfo = cloudProjectsService()->downloadScoreInfo(existingOnlineScoreUrl);

        switch (scoreInfo.ret.code()) {
        case int(Ret::Code::Ok):
            defaultName = scoreInfo.val.title;
            defaultVisibility = scoreInfo.val.visibility;
            break;

        case int(cloud::Err::AccountNotActivated):
        case int(cloud::Err::NetworkError):
            return doShowCloudSaveError(scoreInfo.ret, isPublish, false);

        // It's possible the source URL is invalid or points to a score on a different user's account.
        // In this situation we shouldn't see an error.
        default: break;
        }
    }

    UriQuery query("musescore://project/savetocloud");
    query.addParam("isPublish", Val(isPublish));
    query.addParam("name", Val(defaultName));
    query.addParam("visibility", Val(defaultVisibility));
    query.addParam("existingOnlineScoreUrl", Val(existingOnlineScoreUrl.toString()));

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

    CloudProjectInfo result;
    result.name = vals["name"].toString();
    result.visibility = static_cast<cloud::Visibility>(vals["visibility"].toInt());

    if (!warnBeforePublishing(isPublish, result.visibility)) {
        return make_ret(Ret::Code::Cancel);
    }

    if (mode == SaveMode::Save && vals["replaceExistingOnlineScore"].toBool()) {
        result.sourceUrl = existingOnlineScoreUrl;
    }

    return RetVal<CloudProjectInfo>::make_ok(result);
}

bool SaveProjectScenario::warnBeforePublishing(bool isPublish, cloud::Visibility visibility) const
{
    if (isPublish) {
        if (!configuration()->shouldWarnBeforePublish()) {
            return true;
        }
    } else {
        if (!configuration()->shouldWarnBeforeSavingPubliclyToCloud()) {
            return true;
        }
    }

    std::string title, message;

    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, trc("project/save", "Publish"), true)
    };

    IInteractive::Options options = IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox;

    if (isPublish) {
        title = trc("project/save", "Publish changes online?");
        message = trc("project/save", "We will need to generate a new MP3 for web playback.");
    } else if (visibility == cloud::Visibility::Public) {
        title = trc("project/save", "Publish this score online?"),
        message = trc("project/save", "All saved changes will be publicly visible on MuseScore.com. "
                                      "If you want to make frequent changes, we recommend saving this "
                                      "score privately until you’re ready to share it to the world.");
    } else {
        return true;
    }

    IInteractive::Result result = interactive()->warning(title, message, buttons, int(IInteractive::Button::Ok), options);

    bool ok = result.standardButton() == IInteractive::Button::Ok;
    if (ok && !result.showAgain()) {
        if (isPublish) {
            configuration()->setShouldWarnBeforePublish(false);
        } else {
            configuration()->setShouldWarnBeforeSavingPubliclyToCloud(false);
        }
    }

    return ok;
}

bool SaveProjectScenario::warnBeforeSavingToExistingPubliclyVisibleCloudProject() const
{
    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, trc("project/save", "Publish"), true)
    };

    IInteractive::Result result = interactive()->warning(
        trc("project/save", "Publish changes online?"),
        trc("project/save", "Your saved changes will be publicly visible. We will also "
                            "need to generate a new MP3 for public playback."),
        buttons, int(IInteractive::Button::Ok));

    return result.standardButton() == IInteractive::Button::Ok;
}

Ret SaveProjectScenario::warnCloudIsNotAvailable(bool isPublish) const
{
    if (isPublish) {
        interactive()->warning(trc("project/save", "Unable to connect to MuseScore.com"),
                               trc("project/save", "Please check your internet connection or try again later."));
        return make_ret(Ret::Code::Cancel);
    }

    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, trc("project/save", "Save to computer"), true)
    };

    IInteractive::Result result = interactive()->warning(trc("project/save", "Unable to connect to the cloud"),
                                                         trc("project/save", "Please check your internet connection or try again later."),
                                                         buttons, int(IInteractive::Button::Ok));

    if (result.standardButton() == framework::IInteractive::Button::Ok) {
        return Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE);
    }

    return make_ret(Ret::Code::Cancel);
}

void SaveProjectScenario::showCloudSaveError(const Ret& ret, bool isPublish, bool alreadyAttempted) const
{
    (void)doShowCloudSaveError(ret, isPublish, alreadyAttempted);
}

Ret SaveProjectScenario::doShowCloudSaveError(const Ret& ret, bool isPublish, bool alreadyAttempted) const
{
    std::string title;
    if (alreadyAttempted) {
        title = isPublish
                ? trc("project/save", "Your score could not be published")
                : trc("project/save", "Your score could not be saved to the cloud");
    } else {
        title = isPublish
                ? trc("project/save", "Your score cannot be published")
                : trc("project/save", "Your score cannot be saved to the cloud");
    }

    std::string msg;

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    IInteractive::ButtonData saveLocallyBtn { int(IInteractive::Button::CustomButton) + 1, trc("project/save", "Save to computer") };
    IInteractive::ButtonData helpBtn { IInteractive::Button::CustomButton, trc("project/save", "Get help") };

    IInteractive::ButtonDatas buttons = (alreadyAttempted || isPublish)
                                        ? (IInteractive::ButtonDatas { helpBtn, okBtn })
                                        : (IInteractive::ButtonDatas { helpBtn, saveLocallyBtn, okBtn });

    switch (ret.code()) {
    case int(cloud::Err::AccountNotActivated):
        msg = trc("project/save", "Your musescore.com account needs to be verified first. "
                                  "Please activate your account via the link in the activation email.");
        buttons = { okBtn };
        break;
    case int(cloud::Err::NetworkError):
        msg = cloud::cloudNetworkErrorUserDescription(ret);
        if (!msg.empty()) {
            msg += "\n\n" + trc("project/save", "Please try again later, or get help for this problem on musescore.org.");
            break;
        }
    // FALLTHROUGH
    default:
        msg = trc("project/save", "Please try again later, or get help for this problem on musescore.org.");
        break;
    }

    IInteractive::Result result = interactive()->warning(title, msg, buttons, okBtn.btn);
    if (result.button() == helpBtn.btn) {
        interactive()->openUrl(configuration()->supportForumUrl());
    } else if (result.button() == saveLocallyBtn.btn) {
        return Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE);
    }

    return make_ret(Ret::Code::Cancel);
}
