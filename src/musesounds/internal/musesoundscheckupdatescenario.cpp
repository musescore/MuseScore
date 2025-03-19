/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "global/concurrency/concurrent.h"
#include "global/defer.h"
#include "global/containers.h"
#include "global/stringutils.h"
#include "global/types/val.h"
#include "global/types/translatablestring.h"

#include "update/updateerrors.h"

#include "log.h"

using namespace muse;

using namespace mu::musesounds;
using namespace muse::update;
using namespace muse::actions;

static const char* DEFAULT_IMAGE_URL = "qrc:/qml/MuseScore/MuseSounds/resources/muse_sounds_promo.png";
static const TranslatableString DEFAULT_ACTION_TITLE("musesounds", "Take me to MuseHub");
static const TranslatableString DEFAULT_CANCEL_TITLE("musesounds", "No thanks");

void MuseSoundsCheckUpdateScenario::delayedInit()
{
    if (service()->needCheckForUpdate() && multiInstancesProvider()->instances().size() == 1) {
        doCheckForUpdate(false);
    }
}

bool MuseSoundsCheckUpdateScenario::hasUpdate() const
{
    if (isCheckStarted() || !service()->needCheckForUpdate()) {
        return false;
    }

    RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return false;
    }

    bool noUpdate = lastCheckResult.ret.code() == static_cast<int>(Err::NoUpdate);
    if (noUpdate) {
        return false;
    }

    return !shouldIgnoreUpdate(lastCheckResult.val);
}

muse::Ret MuseSoundsCheckUpdateScenario::showUpdate()
{
    RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (!lastCheckResult.ret) {
        return lastCheckResult.ret;
    }

    return showReleaseInfo(lastCheckResult.val);
}

bool MuseSoundsCheckUpdateScenario::isCheckStarted() const
{
    return m_checkProgress;
}

bool MuseSoundsCheckUpdateScenario::shouldIgnoreUpdate(const ReleaseInfo& info) const
{
    return info.version == configuration()->lastShownMuseSoundsReleaseVersion();
}

void MuseSoundsCheckUpdateScenario::setIgnoredUpdate(const std::string& version)
{
    configuration()->setLastShownMuseSoundsReleaseVersion(version);
}

void MuseSoundsCheckUpdateScenario::doCheckForUpdate(bool manual)
{
    m_checkProgressChannel = std::make_shared<Progress>();
    m_checkProgressChannel->started().onNotify(this, [this]() {
        m_checkProgress = true;
    });

    m_checkProgressChannel->finished().onReceive(this, [this, manual](const ProgressResult& res) {
        DEFER {
            m_checkProgress = false;
        };

        if (!res.ret) {
            LOGE() << "Unable to check for update, error: " << res.ret.toString();
            return;
        }
        if (!manual) {
            return;
        }

        bool noUpdate = res.ret.code() == static_cast<int>(Err::NoUpdate);
        if (noUpdate) {
            return;
        }

        ReleaseInfo info = releaseInfoFromValMap(res.val.toMap());
        if (shouldIgnoreUpdate(info)) {
            return;
        }

        showReleaseInfo(info);
    });

    Concurrent::run(this, &MuseSoundsCheckUpdateScenario::th_checkForUpdate);
}

void MuseSoundsCheckUpdateScenario::th_checkForUpdate()
{
    m_checkProgressChannel->start();

    RetVal<ReleaseInfo> retVal = service()->checkForUpdate();

    RetVal<Val> result;
    result.ret = retVal.ret;
    result.val = Val(releaseInfoToValMap(retVal.val));

    m_checkProgressChannel->finish(result);
}

muse::Ret MuseSoundsCheckUpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    Ret ret = make_ok();

    DEFER {
        if (ret) {
            setIgnoredUpdate(info.version);
        }
    };

    UriQuery query("musescore://musesounds/musesoundsreleaseinfo");
    query.addParam("notes", Val(info.notes));
    query.addParam("features", Val(info.additionInfo.at("features")));

    if (info.actionTitle.empty()) {
        query.addParam("actionTitle", Val(DEFAULT_ACTION_TITLE.qTranslated()));
    } else {
        query.addParam("actionTitle", Val(QString::fromStdString(info.actionTitle)));
    }

    if (info.cancelTitle.empty()) {
        query.addParam("cancelTitle", Val(DEFAULT_CANCEL_TITLE.qTranslated()));
    } else {
        query.addParam("cancelTitle", Val(QString::fromStdString(info.cancelTitle)));
    }

    if (info.imageUrl.empty()) {
        query.addParam("imageUrl", Val(QString(DEFAULT_IMAGE_URL)));
    } else {
        query.addParam("imageUrl", Val(QString::fromStdString(info.imageUrl)));
    }

    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        ret = rv.ret;
        return ret;
    }

    QString actionCode = rv.val.toQString();

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

    interactive()->openApp(muse::Uri(action)).onReject(this, [this, actions](int, const std::string&) {
        tryOpenMuseHub(actions);
    });
}
