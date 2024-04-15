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

#include "updateerrors.h"

#include "types/val.h"

#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::update;
using namespace muse::actions;

void MuseSoundsCheckUpdateScenario::delayedInit()
{
    if (service()->needCheckForUpdate() && multiInstancesProvider()->instances().size() == 1) {
        doCheckForUpdate(false);
    }
}

void MuseSoundsCheckUpdateScenario::checkForUpdate()
{
    if (isCheckStarted() || !service()->needCheckForUpdate()) {
        return;
    }

    RetVal<ReleaseInfo> lastCheckResult = service()->lastCheckResult();
    if (lastCheckResult.ret) {
        //! NOTE: recheck if we already shown the info
        if (shouldIgnoreUpdate(lastCheckResult.val)) {
            return;
        }

        showReleaseInfo(lastCheckResult.val);
        return;
    }

    bool noUpdate = lastCheckResult.ret.code() == static_cast<int>(Err::NoUpdate);
    if (noUpdate) {
        return;
    }

    doCheckForUpdate(true);
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
    m_checkProgressChannel->started.onNotify(this, [this]() {
        m_checkProgress = true;
    });

    m_checkProgressChannel->finished.onReceive(this, [this, manual](const ProgressResult& res) {
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
    m_checkProgressChannel->started.notify();

    RetVal<ReleaseInfo> retVal = service()->checkForUpdate();

    RetVal<Val> result;
    result.ret = retVal.ret;
    result.val = Val(releaseInfoToValMap(retVal.val));

    m_checkProgressChannel->finished.send(result);
}

void MuseSoundsCheckUpdateScenario::showReleaseInfo(const ReleaseInfo& info)
{
    DEFER {
        setIgnoredUpdate(info.version);
    };

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
        service()->openMuseHub();
    }
}
