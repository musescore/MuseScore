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
        if (!manual && releaseInfo.val.version == configuration()->skippedReleaseVersion()) {
            m_progress = false;
            return;
        }

        showReleaseInfo(releaseInfo.val);
        m_progress = false;
    })
    .onReject(this, [this](int errCode, std::string text) {
        LOGE() << "unable to check for update, error code: " << errCode
               << ", " << text;

        processUpdateResult(errCode);
        m_progress = false;
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
        installRelease();
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

void UpdateScenario::installRelease()
{
    RetVal<Val> rv = interactive()->open("musescore://update");
    if (!rv.ret) {
        processUpdateResult(rv.ret.code());
    }
}
