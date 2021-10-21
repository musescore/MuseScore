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

using namespace mu::appshell;
using namespace mu::actions;

static const std::string HOME_URI("musescore://home");
static const std::string NOTATION_URI("musescore://notation");

StartupSessionType StartupScenario::sessionTypeTromString(const QString& str) const
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
    if (!m_startupScorePath.empty()) {
        openScore(m_startupScorePath);
        return;
    }

    StartupSessionType sessionType;
    if (!m_sessionType.isEmpty()) {
        sessionType = sessionTypeTromString(m_sessionType);
    } else {
        sessionType = configuration()->startupSessionType();
    }

    interactive()->open(startupPageUri(sessionType));

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
        openScore(configuration()->startupScorePath());
    } break;
    }
}

std::string StartupScenario::startupPageUri(StartupSessionType sessionType) const
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
