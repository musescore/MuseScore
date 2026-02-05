/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QCoreApplication>

#include "async/async.h"
#include "translation.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

static const muse::UriQuery FIRST_LAUNCH_SETUP_URI("musescore://firstLaunchSetup?floating=true");
static const muse::UriQuery WELCOME_DIALOG_URI("musescore://welcomedialog");
static const muse::Uri HOME_URI("musescore://home");
static const muse::Uri NOTATION_URI("musescore://notation");

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

static const Uri& startupPageUri(StartupModeType modeType)
{
    switch (modeType) {
    case StartupModeType::StartEmpty:
    case StartupModeType::StartWithNewScore:
    case StartupModeType::Recovery:
        return HOME_URI;
    case StartupModeType::StartWithScore:
    case StartupModeType::ContinueLastSession:
        return NOTATION_URI;
    }

    return HOME_URI;
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

void StartupScenario::runOnSplashScreen()
{
    TRACEFUNC;

    if (multiwindowsProvider()->windowCount() != 1) {
        registerAudioPlugins();
        return;
    }

    if (appUpdateScenario() && appUpdateScenario()->needCheckForUpdate()) {
        appUpdateScenario()->checkForUpdate(/*manual*/ false);
    }

    if (museSoundsUpdateScenario() && museSoundsUpdateScenario()->needCheckForUpdate()) {
        museSoundsUpdateScenario()->checkForUpdate(/*manual*/ false);
    }

    registerAudioPlugins();
}

void StartupScenario::registerAudioPlugins()
{
    if (!registerAudioPluginsScenario()) {
        return;
    }

    //! NOTE Registering plugins shows a window (dialog) before the main window is shown.
    //! After closing it, the application may in a state where there are no open windows,
    //! which leads to automatic exit from the application.
    //! (Thanks to the splashscreen, but this is not an obvious detail)
    qApp->setQuitLockEnabled(false);

    Ret ret = registerAudioPluginsScenario()->updatePluginsRegistry();
    if (!ret) {
        LOGE() << ret.toString();
    }

    qApp->setQuitLockEnabled(true);
}

void StartupScenario::runAfterSplashScreen()
{
    TRACEFUNC;

#ifdef MUSE_MULTICONTEXT_WIP
    interactive()->open(HOME_URI);
    return;
#endif

    if (m_startupCompleted) {
        return;
    }

    StartupModeType modeType = resolveStartupModeType();
    if (multiwindowsProvider()->windowCount() == 1 && sessionsManager()->hasProjectsForRestore()) {
        modeType = StartupModeType::Recovery;
    }

    muse::async::Channel<Uri> opened = interactive()->opened();
    opened.onReceive(this, [this, opened, modeType](const Uri&) {
        static bool once = false;
        if (once) {
            return;
        }
        once = true;

        onStartupPageOpened(modeType);

        async::Async::call(this, [this, opened]() {
            muse::async::Channel<Uri> mut = opened;
            mut.disconnect(this);
            m_startupCompleted = true;
        });
    });

    const Uri& startupUri = startupPageUri(modeType);
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
        dispatcher()->dispatch("continue-last-session");
        break;
    case StartupModeType::Recovery:
        restoreLastSession();
        break;
    case StartupModeType::StartWithScore: {
        project::ProjectFile file = m_startupScoreFile.isValid()
                                    ? m_startupScoreFile
                                    : project::ProjectFile(configuration()->startupScorePath());
        openScore(file);
    } break;
    }

    m_activeUpdateCheckCount = 0;

    if (appUpdateScenario() && appUpdateScenario()->checkInProgress()) {
        m_activeUpdateCheckCount++;
        appUpdateScenario()->checkInProgressChanged().onNotify(this, [this, modeType]() {
            appUpdateScenario()->checkInProgressChanged().disconnect(this);
            m_activeUpdateCheckCount--;
            showStartupDialogsIfNeed(modeType);
        }, Asyncable::Mode::SetReplace);
    }

    if (museSoundsUpdateScenario() && museSoundsUpdateScenario()->checkInProgress()) {
        m_activeUpdateCheckCount++;
        museSoundsUpdateScenario()->checkInProgressChanged().onNotify(this, [this, modeType]() {
            museSoundsUpdateScenario()->checkInProgressChanged().disconnect(this);
            m_activeUpdateCheckCount--;
            showStartupDialogsIfNeed(modeType);
        }, Asyncable::Mode::SetReplace);
    }

    showStartupDialogsIfNeed(modeType);
}

void StartupScenario::showStartupDialogsIfNeed(StartupModeType modeType)
{
    TRACEFUNC;

    if (m_activeUpdateCheckCount != 0) {
        return;
    }

    //! NOTE: The welcome dialog should not show if the first launch setup has not been completed, or if we're going
    //! to show a MuseSounds update dialog (see ProjectActionsController::doFinishOpenProject). MuseSampler's update
    //! dialog should be shown after the welcome dialog.
    const auto showWelcomeDialogAndSamplerUpdateIfNeed = [this, modeType]() {
        if (!configuration()->hasCompletedFirstLaunchSetup()) {
            interactive()->open(FIRST_LAUNCH_SETUP_URI);
            return;
        }

        const Version welcomeDialogLastShownVersion(configuration()->welcomeDialogLastShownVersion());
        const Version currentMuseScoreVersion(configuration()->museScoreVersion());
        if (welcomeDialogLastShownVersion < currentMuseScoreVersion) {
            configuration()->setWelcomeDialogShowOnStartup(true); // override user preference
            configuration()->setWelcomeDialogLastShownIndex(-1); // reset
        }

        const bool shouldCheckForMuseSamplerUpdate = modeType == StartupModeType::StartEmpty
                                                     || modeType == StartupModeType::StartWithNewScore;

        if (shouldShowWelcomeDialog(modeType)) {
            interactive()->open(WELCOME_DIALOG_URI).onResolve(this, [this, shouldCheckForMuseSamplerUpdate](const Val&) {
                configuration()->setWelcomeDialogLastShownVersion(configuration()->museScoreVersion());

                if (shouldCheckForMuseSamplerUpdate) {
                    checkAndShowMuseSamplerUpdateIfNeed();
                }
            });
        } else if (shouldCheckForMuseSamplerUpdate) {
            checkAndShowMuseSamplerUpdateIfNeed();
        }
    };

    if (!appUpdateScenario() || !appUpdateScenario()->hasUpdate()) {
        showWelcomeDialogAndSamplerUpdateIfNeed();
        return;
    }

    auto promise = appUpdateScenario()->showUpdate();
    promise.onResolve(this, [showWelcomeDialogAndSamplerUpdateIfNeed](const Ret& ret) {
        if (ret.code() == static_cast<int>(Ret::Code::Ok)) {
            return; // OK means the user wants to close and complete installation - don't show any more dialogs...
        }
        showWelcomeDialogAndSamplerUpdateIfNeed();
    });
}

bool StartupScenario::shouldShowWelcomeDialog(StartupModeType modeType) const
{
    if (!configuration()->welcomeDialogShowOnStartup()) {
        return false;
    }

    if (multiwindowsProvider()->windowCount() != 1) {
        return false;
    }

    if (museSoundsUpdateScenario() && museSoundsUpdateScenario()->hasUpdate()) {
        return false;
    }

    const Uri& startupUri = startupPageUri(modeType);
    return interactive()->currentUri().val == startupUri;
}

void StartupScenario::checkAndShowMuseSamplerUpdateIfNeed()
{
    if (museSamplerCheckForUpdateScenario() && !museSamplerCheckForUpdateScenario()->alreadyChecked()) {
        museSamplerCheckForUpdateScenario()->checkAndShowUpdateIfNeed();
    }
}

void StartupScenario::openScore(const project::ProjectFile& file)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg2<QUrl, QString>(file.url, file.displayNameOverride));
}

void StartupScenario::restoreLastSession()
{
    auto promise = interactive()->question(muse::trc("appshell", "The previous session quit unexpectedly."),
                                           muse::trc("appshell", "Do you want to restore the session?"),
                                           { IInteractive::Button::No, IInteractive::Button::Yes });

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Yes)) {
            sessionsManager()->restore();
        } else {
            removeProjectsUnsavedChanges(configuration()->sessionProjectsPaths());
            sessionsManager()->reset();
            checkAndShowMuseSamplerUpdateIfNeed();
        }
    });
}

void StartupScenario::removeProjectsUnsavedChanges(const io::paths_t& projectsPaths)
{
    for (const muse::io::path_t& path : projectsPaths) {
        projectAutoSaver()->removeProjectUnsavedChanges(path);
    }
}
