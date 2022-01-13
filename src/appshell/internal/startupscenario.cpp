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

#include "startupscenario.h"

#include "async/async.h"
#include "log.h"

using namespace mu::appshell;
using namespace mu::actions;

static const mu::Uri FIRST_LAUNCH_SETUP_URI("musescore://firstLaunchSetup");
static const mu::Uri HOME_URI("musescore://home");
static const mu::Uri NOTATION_URI("musescore://notation");

static StartupModeType modeTypeTromString(const QString& str)
{
    if ("start-empty" == str) {
        return StartupModeType::StartEmpty;
    }

    if ("continue-last" == str) {
        return StartupModeType::ContinueLastSession;
    }

    if ("start-with-new" == str) {
        return StartupModeType::StartWithNewScore;
    }

    if ("start-with-file" == str) {
        return StartupModeType::StartWithScore;
    }

    return StartupModeType::StartEmpty;
}

void StartupScenario::setModeType(const QString& modeType)
{
    m_modeTypeStr = modeType;
}

void StartupScenario::setStartupScorePath(const io::path& path)
{
    m_startupScorePath = path;
}

void StartupScenario::run()
{
    TRACEFUNC;

    if (m_startupCompleted) {
        return;
    }

    StartupModeType modeType = resolveStartupModeType();
    Uri startupUri = startupPageUri(modeType);

    async::Channel<Uri> opened = interactive()->opened();
    opened.onReceive(this, [this, opened, modeType](const Uri&) {
        static bool once = false;
        if (once) {
            return;
        }
        once = true;

        onStartupPageOpened(modeType);

        async::Async::call(this, [this, opened]() {
            async::Channel<Uri> mut = opened;
            mut.resetOnReceive(this);
            m_startupCompleted = true;
        });
    });

    interactive()->open(startupUri);
}

bool StartupScenario::startupCompleted() const
{
    return m_startupCompleted;
}

StartupModeType StartupScenario::resolveStartupModeType() const
{
    if (!m_startupScorePath.empty()) {
        return StartupModeType::StartWithScore;
    }

    if (!m_modeTypeStr.isEmpty()) {
        return modeTypeTromString(m_modeTypeStr);
    }

    return configuration()->startupModeType();
}

void StartupScenario::onStartupPageOpened(StartupModeType modeType)
{
    TRACEFUNC;

    switch (modeType) {
    case StartupModeType::StartEmpty:
        break;
    case StartupModeType::StartWithNewScore:
        dispatcher()->dispatch("file-new");
        break;
    case StartupModeType::ContinueLastSession:
        dispatcher()->dispatch("continue-last-session");
        break;
    case StartupModeType::StartWithScore: {
        io::path path = m_startupScorePath.empty() ? configuration()->startupScorePath()
                        : m_startupScorePath;
        openScore(path);
    } break;
    }

    if (!configuration()->hasCompletedFirstLaunchSetup()) {
        interactive()->open(FIRST_LAUNCH_SETUP_URI);
    }
}

mu::Uri StartupScenario::startupPageUri(StartupModeType modeType) const
{
    switch (modeType) {
    case StartupModeType::StartEmpty:
    case StartupModeType::StartWithNewScore:
        return HOME_URI;
    case StartupModeType::ContinueLastSession:
    case StartupModeType::StartWithScore:
        return NOTATION_URI;
    }

    return HOME_URI;
}

void StartupScenario::openScore(const io::path& path)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(path));
}
