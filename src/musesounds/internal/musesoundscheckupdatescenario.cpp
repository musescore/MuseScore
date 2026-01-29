/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "musesoundscheckupdatescenario.h"

#include "musesoundserrors.h"

#include "global/translation.h"
#include "global/containers.h"
#include "global/stringutils.h"
#include "global/defer.h"
#include "global/log.h"

using namespace muse;
using namespace mu::musesounds;
using namespace muse::update;

bool MuseSoundsCheckUpdateScenario::needCheckForUpdate() const
{
    return configuration()->needCheckForMuseSoundsUpdate();
}

void MuseSoundsCheckUpdateScenario::checkForUpdate(bool manual)
{
    if (m_checkInProgress) {
        return;
    }

    m_checkInProgress = true;
    m_checkInProgressChanged.notify();

    service()->checkForUpdate().onResolve(this, [this, manual](const RetVal<ReleaseInfo>& res) {
        DEFER {
            m_checkInProgress = false;
            m_checkInProgressChanged.notify();
        };

        if (!res.ret) {
            if (res.ret.code() != static_cast<int>(Err::NoUpdate)) {
                LOGE() << res.ret.toString();
            }
            return;
        }

        if (manual && !shouldIgnoreUpdate(res.val)) {
            showReleaseInfo(res.val);
        }
    });
}

bool MuseSoundsCheckUpdateScenario::checkInProgress() const
{
    return m_checkInProgress;
}

async::Notification MuseSoundsCheckUpdateScenario::checkInProgressChanged() const
{
    return m_checkInProgressChanged;
}

bool MuseSoundsCheckUpdateScenario::hasUpdate() const
{
    if (m_checkInProgress || !service()->needCheckForUpdate()) {
        return false;
    }

    const RetVal<ReleaseInfo>& lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return false;
    }

    if (lastCheckResult.ret.code() == static_cast<int>(Err::NoUpdate)) {
        return false;
    }

    return !shouldIgnoreUpdate(lastCheckResult.val);
}

muse::Ret MuseSoundsCheckUpdateScenario::showUpdate()
{
    const RetVal<ReleaseInfo>& lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return lastCheckResult.ret;
    }

    return showReleaseInfo(lastCheckResult.val);
}

bool MuseSoundsCheckUpdateScenario::shouldIgnoreUpdate(const ReleaseInfo& info) const
{
    return info.version == configuration()->lastShownMuseSoundsReleaseVersion() && !configuration()->museSoundsCheckForUpdateTestMode();
}

void MuseSoundsCheckUpdateScenario::setIgnoredUpdate(const std::string& version)
{
    configuration()->setLastShownMuseSoundsReleaseVersion(version);
}

muse::Ret MuseSoundsCheckUpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    Ret ret = make_ok();

    DEFER {
        if (ret || ret.code() == static_cast<int>(Ret::Code::Cancel)) {
            setIgnoredUpdate(info.version);
        }
    };

    UriQuery query("musescore://musesounds/musesoundsreleaseinfo");

    if (!configuration()->museSoundsCheckForUpdateTestMode()) {
        query.addParam("notes", Val(info.notes));
        query.addParam("features", Val(info.additionInfo.at("features")));
    }

    if (info.actionTitle.empty()) {
        query.addParam("actionTitle", Val(trc("musesounds", "Take me to MuseHub")));
    } else {
        query.addParam("actionTitle", Val(info.actionTitle));
    }

    if (info.cancelTitle.empty()) {
        query.addParam("cancelTitle", Val(trc("musesounds", "No thanks")));
    } else {
        query.addParam("cancelTitle", Val(info.cancelTitle));
    }

    if (info.imageUrl.empty()) {
        query.addParam("imageUrl", Val("qrc:/qt/qml/MuseScore/MuseSounds/resources/muse_sounds_promo.png"));
    } else {
        query.addParam("imageUrl", Val(info.imageUrl));
    }

    RetVal<Val> rv = interactive()->openSync(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        ret = rv.ret;
        return ret;
    }

    std::string actionCode = rv.val.toString();
    if (actionCode == "openMuseHub") {
        tryOpenMuseHub(info.actions);
    }

    return ret;
}

void MuseSoundsCheckUpdateScenario::tryOpenMuseHub(ValList actions) const
{
    if (actions.empty()) {
        LOGE() << "no actions to open MuseHub";
        return;
    }

    std::string action = muse::takeFirst(actions).toString();
    LOGI() << "try open: " << action;

    if (muse::strings::startsWith(action, "http")) { // or https
        interactive()->openUrl(action);
        return;
    }

    interactive()->openApp(muse::UriQuery(action)).onReject(this, [this, actions](int, const std::string&) {
        tryOpenMuseHub(actions);
    });
}
