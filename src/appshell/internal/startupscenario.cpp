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

static StartupSessionType sessionTypeTromString(const QString& str)
{
    if ("start-empty" == str) {
        return StartupSessionType::StartEmpty;
    }

    if ("continue-last" == str) {
        return StartupSessionType::ContinueLastSession;
    }

    if ("start-with-new" == str) {
        return StartupSessionType::StartWithNewScore;
    }

    if ("start-with-file" == str) {
        return StartupSessionType::StartWithScore;
    }

    return StartupSessionType::StartEmpty;
}

void StartupScenario::setSessionType(const QString& sessionType)
{
    m_sessionType = sessionType;
}

void StartupScenario::setStartupScorePath(const io::path& path)
{
    m_startupScorePath = path;
}

void StartupScenario::run()
{
    TRACEFUNC;

    StartupSessionType sessionType = resolveStartupSessionType();
    Uri startupUri = startupPageUri(sessionType);
    async::Channel<Uri> opened = interactive()->opened();

    opened.onReceive(this, [this, opened, startupUri, sessionType](const Uri& uri) {
        if (uri != startupUri) {
            return;
        }

        onStartupPageOpened(sessionType);

        async::Async::call(this, [this, opened]() {
            async::Channel<Uri> mut = opened;
            mut.resetOnReceive(this);
        });
    });

    interactive()->open(startupUri);
}

StartupSessionType StartupScenario::resolveStartupSessionType() const
{
    if (!m_startupScorePath.empty()) {
        return StartupSessionType::StartWithScore;
    }

    if (!m_sessionType.isEmpty()) {
        return sessionTypeTromString(m_sessionType);
    }

    return configuration()->startupSessionType();
}

void StartupScenario::onStartupPageOpened(StartupSessionType sessionType)
{
    TRACEFUNC;

    switch (sessionType) {
    case StartupSessionType::StartEmpty:
        break;
    case StartupSessionType::StartWithNewScore:
        dispatcher()->dispatch("file-new");
        break;
    case StartupSessionType::ContinueLastSession:
        dispatcher()->dispatch("continue-last-session");
        break;
    case StartupSessionType::StartWithScore: {
        io::path path = m_startupScorePath.empty() ? configuration()->startupScorePath()
                        : m_startupScorePath;
        openScore(path);
    } break;
    }

    if (!configuration()->hasCompletedFirstLaunchSetup()) {
        interactive()->open(FIRST_LAUNCH_SETUP_URI);
    }
}

mu::Uri StartupScenario::startupPageUri(StartupSessionType sessionType) const
{
    switch (sessionType) {
    case StartupSessionType::StartEmpty:
    case StartupSessionType::StartWithNewScore:
        return HOME_URI;
    case StartupSessionType::ContinueLastSession:
    case StartupSessionType::StartWithScore:
        return NOTATION_URI;
    }

    return HOME_URI;
}

void StartupScenario::openScore(const io::path& path)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(path));
}
