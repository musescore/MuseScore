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

#include "updateerrors.h"

#include "translation.h"
#include "config.h"
#include "defer.h"
#include "log.h"

static constexpr int AUTO_CHECK_UPDATE_INTERVAL = 1000;

using namespace mu::update;
using namespace mu::framework;

void UpdateScenario::delayedInit()
{
    //! NOTE: temporary disabled auto checking
    return;

    if (configuration()->needCheckForUpdate()) {
        QTimer::singleShot(AUTO_CHECK_UPDATE_INTERVAL, [this]() {
            doCheckForUpdate(false);
        });
    }
}

void UpdateScenario::checkForUpdate()
{
    doCheckForUpdate(true);
}

bool UpdateScenario::isCheckStarted() const
{
    return m_progress;
}

void UpdateScenario::doCheckForUpdate(bool manual)
{
    if (isCheckStarted()) {
        return;
    }

    m_progress = true;

    updateService()->checkForUpdate().onResolve(this, [this, manual](const mu::RetVal<ReleaseInfo>& releaseInfo) {
        DEFER {
            m_progress = false;
        };

        bool noUpdate = releaseInfo.ret.code() == static_cast<int>(Err::NoUpdate);
        if (!manual) {
            bool shouldIgnoreUpdate = releaseInfo.val.version == configuration()->skippedReleaseVersion();
            if (noUpdate || shouldIgnoreUpdate) {
                return;
            }
        } else if (noUpdate) {
            showNoUpdateMsg();
            return;
        }

        showReleaseInfo(releaseInfo.val);
    })
    .onReject(this, [this](int errCode, std::string text) {
        DEFER {
            m_progress = false;
        };

        LOGE() << "unable to check for update, error code: " << errCode
               << ", " << text;

        processUpdateResult(errCode);
    });
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
    QString str = qtrc("update", "You already have the latest version of MuseScore. "
                                 "Please visit <a href=\"%1\">musescore.org</a> for news on what's coming next.")
                  .arg(QString::fromStdString(configuration()->museScoreUrl()));

    IInteractive::Text text(str.toStdString(), IInteractive::TextFormat::RichText);
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    interactive()->info(trc("update", "You’re up to date!"), text, { okBtn }, (int)IInteractive::Button::Ok,
                        IInteractive::Option::WithIcon);
}

void UpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    QStringList params = {
        "title=" + QString::fromStdString(info.title),
        "notes=" + QString::fromStdString(info.notes)
    };

    RetVal<Val> rv = interactive()->open(QString("musescore://update/releaseinfo?%1").arg(params.join('&')).toStdString());
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

void UpdateScenario::showServerErrorMsg()
{
    interactive()->error(trc("update", "Cannot connect to server"),
                         trc("update", "Sorry - please try again later"),  {}, 0,
                         IInteractive::Option::WithIcon);
}

void UpdateScenario::downloadRelease()
{
    RetVal<Val> rv = interactive()->open("musescore://update?mode=download");
    if (!rv.ret) {
        processUpdateResult(rv.ret.code());
        return;
    }

    closeAppAndStartInstallation(rv.val.toString());
}

void UpdateScenario::closeAppAndStartInstallation(const io::path_t& installerPath)
{
    std::string info = trc("update", "MuseScore needs to close to complete the installation. "
                                     "If you have any unsaved changes, you will be prompted to save them before MuseScore closes.");

    int closeBtn = int(IInteractive::Button::CustomButton) + 1;
    IInteractive::Result result = interactive()->info("", info,
                                                      { interactive()->buttonData(IInteractive::Button::Cancel),
                                                        IInteractive::ButtonData(closeBtn, trc("update", "Close"), true) },
                                                      closeBtn);

    if (result.standardButton() == IInteractive::Button::Cancel) {
        return;
    }

    if (multiInstancesProvider()->instances().size() != 1) {
        multiInstancesProvider()->quitAllAndRunInstallation(installerPath);
    }

    dispatcher()->dispatch("quit", actions::ActionData::make_arg2<bool, std::string>(false, installerPath.toStdString()));
}
