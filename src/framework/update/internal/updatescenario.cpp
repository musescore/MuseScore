/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
                LOGE() << "Unable to check for app update, error: " << res.ret.toString();
                ret = muse::make_ret(Ret::Code::UnknownError);

                if (manual) {
                    showServerErrorMsg();
                }

                return;
            }

            if (!manual) {
                return;
            }

            const ReleaseInfo info = releaseInfoFromValMap(res.val.toMap());
            if (noUpdate) {
                showNoUpdateMsg();
            } else {
                showReleaseInfo(info);
            }
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

muse::async::Promise<Ret> UpdateScenario::showUpdate()
{
    const RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (lastCheckResult.ret) {
        return showReleaseInfo(lastCheckResult.val);
    }
    return async::make_promise<Ret>([lastCheckResult](auto resolve, auto) {
        return resolve(lastCheckResult.ret);
    });
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

muse::async::Promise<Ret> UpdateScenario::processUpdateError(int errorCode)
{
    const auto unknownError = async::make_promise<Ret>([](auto resolve, auto) {
        return resolve(muse::make_ret(Ret::Code::UnknownError));
    });

    IF_ASSERT_FAILED(errorCode >= static_cast<int>(Ret::Code::UpdateFirst)
                     && errorCode <= static_cast<int>(Ret::Code::UpdateLast)) {
        return unknownError;
    }

    const Err error = static_cast<Err>(errorCode);
    IF_ASSERT_FAILED(error != Err::NoError) {
        return unknownError;
    }

    auto message = error == Err::NoUpdate ? showNoUpdateMsg() : showServerErrorMsg();
    return message.then<Ret>(this, [errorCode](const IInteractive::Result&, auto resolve) {
        const Ret::Code code = static_cast<Ret::Code>(errorCode);
        return resolve(muse::make_ret(code));
    });
}

async::Promise<IInteractive::Result> UpdateScenario::showNoUpdateMsg()
{
    const QString str = muse::qtrc("update", "You already have the latest version of MuseScore Studio. "
                                             "Please visit <a href=\"%1\">MuseScore.org</a> for news on what’s coming next.")
                        .arg(QString::fromStdString(configuration()->museScoreUrl()));

    const IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);
    const IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    return interactive()->info(muse::trc("update", "You’re up to date!"), text, { okBtn }, okBtn.btn,
                               IInteractive::Option::WithIcon);
}

muse::async::Promise<Ret> UpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    UriQuery query("muse://update/appreleaseinfo");
    query.addParam("notes", Val(info.notes));
    query.addParam("previousReleasesNotes", Val(releasesNotesToValList(info.previousReleasesNotes)));

    return interactive()->open(query).then<Ret>(this, [this, info](const Val& val, auto resolve) {
        const QString actionCode = val.toQString();
        if (actionCode == "remindLater") {
            return resolve(muse::make_ret(Ret::Code::Cancel));
        }

        if (actionCode == "skip") {
            configuration()->setSkippedReleaseVersion(info.version);
            return resolve(muse::make_ret(Ret::Code::Cancel));
        }

        //! NOTE: In test mode we skip the progress dialog and jump straight to the "needs to close" dialog...
        const bool testMode = configuration()->checkForUpdateTestMode();
        auto promise = testMode ? askToCloseAppAndCompleteInstall(/*installerPath*/ String()) : downloadRelease();
        promise.onResolve(this, [resolve](const Ret& ret) {
            (void)resolve(ret);
        });

        return muse::async::Promise<Ret>::dummy_result();
    });
}

async::Promise<IInteractive::Result> UpdateScenario::showServerErrorMsg()
{
    return interactive()->error(muse::trc("update", "Cannot connect to server"),
                                muse::trc("update", "Sorry - please try again later"));
}

muse::async::Promise<Ret> UpdateScenario::downloadRelease()
{
    RetVal<Val> rv = interactive()->openSync("muse://update?mode=download");
    if (!rv.ret) {
        return processUpdateError(rv.ret.code());
    }
    return askToCloseAppAndCompleteInstall(rv.val.toString());
}

muse::async::Promise<Ret> UpdateScenario::askToCloseAppAndCompleteInstall(const muse::io::path_t& installerPath)
{
    const std::string info = muse::trc("update", "MuseScore Studio needs to close to complete the installation. "
                                                 "If you have any unsaved changes, you will be prompted to save them before MuseScore Studio closes.");
    const int closeBtn = int(IInteractive::Button::CustomButton) + 1;
    const IInteractive::ButtonDatas buttons = {
        interactive()->buttonData(IInteractive::Button::Cancel),
        IInteractive::ButtonData(closeBtn, muse::trc("update", "Close"), true)
    };

    return interactive()->info("", info, buttons, closeBtn)
           .then<Ret>(this, [this, installerPath](const IInteractive::Result& res, auto resolve) {
        if (res.isButton(IInteractive::Button::Cancel)) {
            return resolve(muse::make_ret(Ret::Code::Cancel));
        }

        if (multiInstancesProvider()->instances().size() != 1) {
            multiInstancesProvider()->quitAllAndRunInstallation(installerPath);
        }

        dispatcher()->dispatch("quit", ActionData::make_arg2<bool, std::string>(false, installerPath.toStdString()));
        return resolve(muse::make_ok());
    });
}

bool UpdateScenario::shouldIgnoreUpdate(const ReleaseInfo& info) const
{
    return info.version == configuration()->skippedReleaseVersion() && !configuration()->checkForUpdateTestMode();
}
