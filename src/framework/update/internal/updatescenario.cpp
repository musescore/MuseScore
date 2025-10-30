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

using namespace muse;
using namespace muse::update;
using namespace muse::actions;

bool UpdateScenario::needCheckForUpdate() const
{
    return configuration()->needCheckForUpdate();
}

muse::async::Promise<Ret> UpdateScenario::checkForUpdate(bool manual)
{
    return async::make_promise<Ret>([this, manual](auto resolve, auto) {
        m_checkProgressChannel = std::make_shared<Progress>();
        m_checkProgressChannel->started().onNotify(this, [this]() {
            m_checkInProgress = true;
        });

        if (isCheckInProgress()) {
            LOGE() << "Check already in progress";
            const Ret ret = muse::make_ret(Ret::Code::UnknownError);
            return resolve(ret);
        }

        m_checkProgressChannel = std::make_shared<Progress>();
        m_checkProgressChannel->started().onNotify(this, [this]() {
            m_checkInProgress = true;
        });

        m_checkProgressChannel->finished().onReceive(this, [this, manual, resolve](const ProgressResult& res) {
            Ret ret = muse::make_ok();
            DEFER {
                m_checkInProgress = false;
                (void)resolve(ret);
            };

            const bool noUpdate = res.ret.code() == static_cast<int>(Err::NoUpdate);
            if (!noUpdate && !res.ret) {
                LOGE() << "Unable to check for update, error: " << res.ret.toString();
                ret = muse::make_ret(Ret::Code::UnknownError);

                if (manual) {
                    showServerErrorMsg();
                }

                return;
            }

            if (!manual) {
                return;
            }

            ReleaseInfo info = releaseInfoFromValMap(res.val.toMap());
            noUpdate ? showNoUpdateMsg() : showReleaseInfo(info);
        });

        Concurrent::run(this, &UpdateScenario::th_checkForUpdate);
        return muse::async::Promise<Ret>::dummy_result();
    });
}

bool UpdateScenario::hasUpdate() const
{
    if (isCheckInProgress()) {
        return false;
    }

    const RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return false;
    }

    bool noUpdate = lastCheckResult.ret.code() == static_cast<int>(Err::NoUpdate);
    if (noUpdate) {
        return false;
    }

    return !shouldIgnoreUpdate(lastCheckResult.val);
}

muse::Ret UpdateScenario::showUpdate()
{
    RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return lastCheckResult.ret;
    }

    showReleaseInfo(lastCheckResult.val);
    return muse::make_ok();
}

bool UpdateScenario::isCheckInProgress() const
{
    return m_checkInProgress;
}

void UpdateScenario::th_checkForUpdate()
{
    m_checkProgressChannel->start();

    RetVal<ReleaseInfo> retVal = service()->checkForUpdate();

    RetVal<Val> result;
    result.ret = retVal.ret;
    result.val = Val(releaseInfoToValMap(retVal.val));

    m_checkProgressChannel->finish(result);
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
        showNoUpdateMsg();
        break;
    default:
        showServerErrorMsg();
        break;
    }
}

void UpdateScenario::showNoUpdateMsg()
{
    QString str = muse::qtrc("update", "You already have the latest version of MuseScore Studio. "
                                       "Please visit <a href=\"%1\">MuseScore.org</a> for news on what’s coming next.")
                  .arg(QString::fromStdString(configuration()->museScoreUrl()));

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    interactive()->info(muse::trc("update", "You’re up to date!"), text, { okBtn }, okBtn.btn,
                        IInteractive::Option::WithIcon);
}

void UpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    UriQuery query("muse://update/appreleaseinfo");
    query.addParam("notes", Val(info.notes));
    query.addParam("previousReleasesNotes", Val(releasesNotesToValList(info.previousReleasesNotes)));

    RetVal<Val> rv = interactive()->openSync(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        return;
    }

    if (configuration()->checkForUpdateTestMode()) {
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

void UpdateScenario::showServerErrorMsg()
{
    interactive()->error(muse::trc("update", "Cannot connect to server"),
                         muse::trc("update", "Sorry - please try again later"));
}

void UpdateScenario::downloadRelease()
{
    RetVal<Val> rv = interactive()->openSync("muse://update?mode=download");
    if (!rv.ret) {
        processUpdateResult(rv.ret.code());
        return;
    }
    askToCloseAndCompleteInstall(rv.val.toString());
}

void UpdateScenario::askToCloseAndCompleteInstall(const muse::io::path_t& installerPath)
{
    const std::string info = muse::trc("update", "MuseScore Studio needs to close to complete the installation. "
                                                 "If you have any unsaved changes, you will be prompted to save them before MuseScore Studio closes.");
    const int closeBtn = int(IInteractive::Button::CustomButton) + 1;
    const IInteractive::Result res = interactive()->infoSync("", info,
                                                             { interactive()->buttonData(IInteractive::Button::Cancel),
                                                               IInteractive::ButtonData(closeBtn, muse::trc("update", "Close"), true) },
                                                             closeBtn);
    if (res.isButton(IInteractive::Button::Cancel)) {
        return;
    }

    if (multiInstancesProvider()->instances().size() != 1) {
        multiInstancesProvider()->quitAllAndRunInstallation(installerPath);
    }

    dispatcher()->dispatch("quit", ActionData::make_arg2<bool, std::string>(false, installerPath.toStdString()));
}

bool UpdateScenario::shouldIgnoreUpdate(const ReleaseInfo& info) const
{
    return info.version == configuration()->skippedReleaseVersion() && !configuration()->checkForUpdateTestMode();
}
