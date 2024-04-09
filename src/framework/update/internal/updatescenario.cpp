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

#include "updatescenario.h"

#include <QTimer>

#include "global/concurrency/concurrent.h"

#include "updateerrors.h"

#include "types/val.h"

#include "translation.h"
#include "defer.h"
#include "log.h"

static constexpr int AUTO_CHECK_UPDATE_INTERVAL = 1000;

using namespace muse;
using namespace muse::update;
using namespace muse::actions;

static ValList releasesNotesToValList(const PrevReleasesNotesList& list)
{
    ValList valList;
    for (const PrevReleaseNotes& release : list) {
        valList.emplace_back(Val(ValMap {
            { "version", Val(release.version) },
            { "notes", Val(release.notes) }
        }));
    }

    return valList;
}

static PrevReleasesNotesList releasesNotesFromValList(const ValList& list)
{
    PrevReleasesNotesList notes;
    for (const Val& val : list) {
        ValMap releaseMap = val.toMap();
        notes.emplace_back(releaseMap.at("version").toString(), releaseMap.at("notes").toString());
    }

    return notes;
}

static ValMap releaseInfoToValMap(const ReleaseInfo& info)
{
    return {
        { "version", Val(info.version) },
        { "fileName", Val(info.fileName) },
        { "fileUrl", Val(info.fileUrl) },
        { "notes", Val(info.notes) },
        { "previousReleasesNotes", Val(releasesNotesToValList(info.previousReleasesNotes)) },
        { "additionalInfo", Val(info.additionInfo) }
    };
}

static ReleaseInfo releaseInfoFromValMap(const ValMap& map)
{
    ReleaseInfo info;
    info.version = map.at("version").toString();
    info.fileName = map.at("fileName").toString();
    info.fileUrl = map.at("fileUrl").toString();
    info.notes = map.at("notes").toString();
    info.previousReleasesNotes = releasesNotesFromValList(map.at("previousReleasesNotes").toList());
    info.additionInfo = map.at("additionalInfo").toMap();

    return info;
}

void UpdateScenario::delayedInit()
{
    if (configuration()->needCheckForUpdate() && multiInstancesProvider()->instances().size() == 1) {
        QTimer::singleShot(AUTO_CHECK_UPDATE_INTERVAL, [this]() {
            doCheckForAppUpdate(false);
        });

        doCheckForMuseSamplerUpdate(false);
    }
}

void UpdateScenario::checkForAppUpdate()
{
    if (isAppCheckStarted()) {
        return;
    }

    doCheckForAppUpdate(true);
}

void UpdateScenario::checkForMuseSamplerUpdate()
{
    if (isMuseSamplerCheckStarted()) {
        return;
    }

    RetVal<ReleaseInfo> lastCheckResult = museSamplerUpdateService()->lastCheckResult();
    if (lastCheckResult.ret) {
        if (shouldIgnoreMuseSamplerUpdate(lastCheckResult.val)) {
            return;
        }

        showMuseSamplerReleaseInfo(lastCheckResult.val);
        return;
    }

    doCheckForMuseSamplerUpdate(true);
}

bool UpdateScenario::isAppCheckStarted() const
{
    return m_appCheckProgress;
}

bool UpdateScenario::isMuseSamplerCheckStarted() const
{
    return m_museSamplerCheckProgress;
}

bool UpdateScenario::shouldIgnoreMuseSamplerUpdate(const ReleaseInfo& info) const
{
    return info.version == configuration()->lastShownMuseSamplerReleaseVersion();
}

void UpdateScenario::setIgnoredMuseSamplerUpdate(const std::string& version)
{
    configuration()->setLastShownMuseSamplerReleaseVersion(version);
}

void UpdateScenario::doCheckForAppUpdate(bool manual)
{
    m_appCheckProgressChannel = std::make_shared<Progress>();
    m_appCheckProgressChannel->started.onNotify(this, [this]() {
        m_appCheckProgress = true;
    });

    m_appCheckProgressChannel->finished.onReceive(this, [this, manual](const ProgressResult& res) {
        DEFER {
            m_appCheckProgress = false;
        };

        if (!res.ret) {
            LOGE() << "Unable to check for update, error: " << res.ret.toString();

            if (manual) {
                processUpdateResult(res.ret.code());
            }

            return;
        }

        ReleaseInfo info = releaseInfoFromValMap(res.val.toMap());
        bool noUpdate = res.ret.code() == static_cast<int>(Err::NoUpdate);
        if (!manual) {
            bool shouldIgnoreUpdate = info.version == configuration()->skippedReleaseVersion();
            if (noUpdate || shouldIgnoreUpdate) {
                return;
            }
        } else if (noUpdate) {
            showNoAppUpdateMsg();
            return;
        }

        showAppReleaseInfo(info);
    });

    Concurrent::run(this, &UpdateScenario::th_checkForAppUpdate);
}

void UpdateScenario::th_checkForAppUpdate()
{
    m_appCheckProgressChannel->started.notify();

    RetVal<ReleaseInfo> retVal = appUpdateService()->checkForUpdate();

    RetVal<Val> result;
    result.ret = retVal.ret;
    result.val = Val(releaseInfoToValMap(retVal.val));

    m_appCheckProgressChannel->finished.send(result);
}

void UpdateScenario::doCheckForMuseSamplerUpdate(bool manual)
{
    m_museSamplerCheckProgressChannel = std::make_shared<Progress>();
    m_museSamplerCheckProgressChannel->started.onNotify(this, [this]() {
        m_museSamplerCheckProgress = true;
    });

    m_museSamplerCheckProgressChannel->finished.onReceive(this, [this, manual](const ProgressResult& res) {
        DEFER {
            m_museSamplerCheckProgress = false;
        };

        if (!res.ret) {
            LOGE() << "Unable to check for update, error: " << res.ret.toString();
            return;
        }
        if (!manual) {
            return;
        }

        ReleaseInfo info = releaseInfoFromValMap(res.val.toMap());
        bool noUpdate = res.ret.code() == static_cast<int>(Err::NoUpdate);
        bool shouldIgnoreUpdate = shouldIgnoreMuseSamplerUpdate(info);

        if (noUpdate || shouldIgnoreUpdate) {
            return;
        }

        showMuseSamplerReleaseInfo(info);
    });

    Concurrent::run(this, &UpdateScenario::th_checkForMuseSamplerUpdate);
}

void UpdateScenario::th_checkForMuseSamplerUpdate()
{
    m_museSamplerCheckProgressChannel->started.notify();

    RetVal<ReleaseInfo> retVal = museSamplerUpdateService()->checkForUpdate();

    RetVal<Val> result;
    result.ret = retVal.ret;
    result.val = Val(releaseInfoToValMap(retVal.val));

    m_museSamplerCheckProgressChannel->finished.send(result);
}

void UpdateScenario::processUpdateResult(int errorCode)
{
    if (errorCode < static_cast<int>(Ret::Code::UpdateFirst)
        || errorCode > static_cast<int>(Ret::Code::UpdateLast)) {
        return;
    }

    Err error = static_cast<Err>(errorCode);

    switch (error) {
    case Err::NoError:
        break;
    case Err::NoUpdate:
        showNoAppUpdateMsg();
        break;
    default:
        showServerErrorMsg();
        break;
    }
}

void UpdateScenario::showNoAppUpdateMsg()
{
    QString str = muse::qtrc("update", "You already have the latest version of MuseScore. "
                                       "Please visit <a href=\"%1\">musescore.org</a> for news on what’s coming next.")
                  .arg(QString::fromStdString(configuration()->museScoreUrl()));

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    interactive()->info(muse::trc("update", "You’re up to date!"), text, { okBtn }, okBtn.btn,
                        IInteractive::Option::WithIcon);
}

void UpdateScenario::showAppReleaseInfo(const ReleaseInfo& info)
{
    UriQuery query("muse://update/appreleaseinfo");
    query.addParam("notes", Val(info.notes));
    query.addParam("previousReleasesNotes", Val(releasesNotesToValList(info.previousReleasesNotes)));

    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        return;
    }

    QString actionCode = rv.val.toQString();

    if (actionCode == "install") {
        downloadRelease();
    } else if (actionCode == "remindLater") {
        return;
    } else if (actionCode == "skip") {
        configuration()->setSkippedReleaseVersion(info.version);
    }
}

void UpdateScenario::showMuseSamplerReleaseInfo(const ReleaseInfo& info)
{
    UriQuery query("muse://update/musesoundsreleaseinfo");
    query.addParam("notes", Val(info.notes));
    query.addParam("features", Val(info.additionInfo.at("features")));

    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        return;
    }

    QString actionCode = rv.val.toQString();

    if (actionCode == "openMuseHub") {
        museSamplerUpdateService()->openMuseHub();
    }

    setIgnoredMuseSamplerUpdate(info.version);
}

void UpdateScenario::showServerErrorMsg()
{
    interactive()->error(muse::trc("update", "Cannot connect to server"),
                         muse::trc("update", "Sorry - please try again later"));
}

void UpdateScenario::downloadRelease()
{
    RetVal<Val> rv = interactive()->open("muse://update?mode=download");
    if (!rv.ret) {
        processUpdateResult(rv.ret.code());
        return;
    }

    closeAppAndStartInstallation(rv.val.toString());
}

void UpdateScenario::closeAppAndStartInstallation(const muse::io::path_t& installerPath)
{
    std::string info = muse::trc("update", "MuseScore needs to close to complete the installation. "
                                           "If you have any unsaved changes, you will be prompted to save them before MuseScore closes.");

    int closeBtn = int(IInteractive::Button::CustomButton) + 1;
    IInteractive::Result result = interactive()->info("", info,
                                                      { interactive()->buttonData(IInteractive::Button::Cancel),
                                                        IInteractive::ButtonData(closeBtn, muse::trc("update", "Close"), true) },
                                                      closeBtn);

    if (result.standardButton() == IInteractive::Button::Cancel) {
        return;
    }

    if (multiInstancesProvider()->instances().size() != 1) {
        multiInstancesProvider()->quitAllAndRunInstallation(installerPath);
    }

    dispatcher()->dispatch("quit", ActionData::make_arg2<bool, std::string>(false, installerPath.toStdString()));
}
