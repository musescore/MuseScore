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
#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::framework;

static const mu::Uri FIRST_LAUNCH_SETUP_URI("musescore://firstLaunchSetup");
static const mu::Uri HOME_URI("musescore://home");
static const mu::Uri NOTATION_URI("musescore://notation");

static StartupModeType modeTypeTromString(const std::string& str)
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

void StartupScenario::setStartupType(const std::optional<std::string>& type)
{
    m_startupTypeStr = type ? type.value() : "";
}

bool StartupScenario::isStartWithNewFileAsSecondaryInstance() const
{
    if (m_startupScoreFile.isValid()) {
        return false;
    }

    if (!m_startupTypeStr.empty()) {
        return modeTypeTromString(m_startupTypeStr) == StartupModeType::StartWithNewScore;
    }

    return false;
}

const mu::project::ProjectFile& StartupScenario::startupScoreFile() const
{
    return m_startupScoreFile;
}

void StartupScenario::setStartupScoreFile(const std::optional<project::ProjectFile>& file)
{
    m_startupScoreFile = file ? file.value() : project::ProjectFile();
}

void StartupScenario::run()
{
    TRACEFUNC;

    if (m_startupCompleted) {
        return;
    }

    StartupModeType modeType = resolveStartupModeType();
    bool isMainInstance = multiInstancesProvider()->isMainInstance();
    if (isMainInstance && sessionsManager()->hasProjectsForRestore()) {
        modeType = StartupModeType::ContinueLastSession;
    }

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
    if (m_startupScoreFile.isValid()) {
        return StartupModeType::StartWithScore;
    }

    if (!m_startupTypeStr.empty()) {
        return modeTypeTromString(m_startupTypeStr);
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
        restoreLastSession();
        break;
    case StartupModeType::StartWithScore: {
        project::ProjectFile file
            = m_startupScoreFile.isValid() ? m_startupScoreFile : project::ProjectFile(configuration()->startupScorePath());
        openScore(file);
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
    case StartupModeType::StartWithScore:
        return NOTATION_URI;
    case StartupModeType::ContinueLastSession:
        return HOME_URI;
    }

    return HOME_URI;
}

void StartupScenario::openScore(const project::ProjectFile& file)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg2<io::path_t, QString>(file.path, file.displayNameOverride));
}

void StartupScenario::restoreLastSession()
{
    if (!sessionsManager()->hasProjectsForRestore()) {
        dispatcher()->dispatch("continue-last-session");
        return;
    }

    IInteractive::Result result = interactive()->question(trc("appshell", "The previous session quit unexpectedly."),
                                                          trc("appshell", "Do you want to restore the session?"),
                                                          { IInteractive::Button::No, IInteractive::Button::Yes });

    if (result.button() == static_cast<int>(IInteractive::Button::Yes)) {
        sessionsManager()->restore();
    } else {
        removeProjectsUnsavedChanges(configuration()->sessionProjectsPaths());
        sessionsManager()->reset();
    }
}

void StartupScenario::removeProjectsUnsavedChanges(const io::paths_t& projectsPaths)
{
    for (const io::path_t& path : projectsPaths) {
        projectAutoSaver()->removeProjectUnsavedChanges(path);
    }
}
