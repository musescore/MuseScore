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
#include <QTimer>

#include "async/async.h"
#include "network/networkerrors.h"
#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

static const muse::UriQuery FIRST_LAUNCH_SETUP_URI("musescore://firstLaunchSetup?floating=true");
static const muse::UriQuery WELCOME_DIALOG_URI("musescore://welcomedialog");
static const muse::Uri HOME_URI("musescore://home");
static const muse::Uri NOTATION_URI("musescore://notation");

static constexpr int CHECK_FOR_UPDATES_TIMEOUT(7500);

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

muse::async::Promise<muse::Ret> StartupScenario::runOnSplashScreen()
{
    return async::make_promise<Ret>([this](auto resolve, auto) {
        registerAudioPlugins();

        if (multiInstancesProvider()->instances().size() != 1) {
            const Ret ret = muse::make_ret(Ret::Code::Ok);
            return resolve(ret);
        }

        // Calculate the total number of expected update checks (TODO: check the
        // connection before trying any of this)...

        const bool canCheckAppUpdate = appUpdateScenario() && appUpdateScenario()->needCheckForUpdate();
        const bool canCheckMuseSoundsUpdate = museSoundsUpdateScenario() && museSoundsUpdateScenario()->needCheckForUpdate();
        //! NOTE: A MuseSampler update check also exists but we run it later (see onStartupPageOpened)...
        m_totalChecksExpected = size_t(canCheckAppUpdate) + size_t(canCheckMuseSoundsUpdate);

        if (m_totalChecksExpected == 0) {
            const Ret ret = muse::make_ret(Ret::Code::Ok);
            return resolve(ret);
        }

        m_totalChecksReceived = 0;

        // Resolve once all checks are completed...
        const auto onUpdateCheckCompleted = [this, resolve](){
            if (!m_updateChecksInProgress) {
                return; // Already resolved or timed out...
            }

            ++m_totalChecksReceived;

            if (m_totalChecksReceived < m_totalChecksExpected) {
                return; // Not ready to resolve yet...
            }

            m_updateChecksInProgress = false;

            const Ret ret = muse::make_ret(Ret::Code::Ok);
            (void)resolve(ret);
        };

        // Asynchronously start the checks once we know the total number of expected checks...
        m_updateChecksInProgress = true;
        async::Async::call(this, [this, onUpdateCheckCompleted, canCheckAppUpdate, canCheckMuseSoundsUpdate]() {
            if (canCheckAppUpdate) {
                muse::async::Promise<Ret> promise = appUpdateScenario()->checkForUpdate(/*manual*/ false);
                promise.onResolve(this, [onUpdateCheckCompleted](Ret) {
                    onUpdateCheckCompleted();
                });
            }
            if (canCheckMuseSoundsUpdate) {
                muse::async::Promise<Ret> promise = museSoundsUpdateScenario()->checkForUpdate(/*manual*/ false);
                promise.onResolve(this, [onUpdateCheckCompleted](Ret) {
                    onUpdateCheckCompleted();
                });
            }
        });

        // Timeout if the checks take too long...
        QTimer::singleShot(CHECK_FOR_UPDATES_TIMEOUT, [this, resolve]() {
            if (!m_updateChecksInProgress) {
                return;
            }

            m_updateChecksInProgress = false;

            LOGE() << "Update checks timed out...";
            const Ret ret = network::make_ret(network::Err::Timeout);
            (void)resolve(ret);
        });

        return muse::async::Promise<Ret>::dummy_result();
    });
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

    Ret ret = registerAudioPluginsScenario()->registerNewPlugins();
    if (!ret) {
        LOGE() << ret.toString();
    }

    qApp->setQuitLockEnabled(true);
}

void StartupScenario::runAfterSplashScreen()
{
    TRACEFUNC;

    if (m_startupCompleted) {
        return;
    }

    StartupModeType modeType = resolveStartupModeType();
    bool isMainInstance = multiInstancesProvider()->isMainInstance();
    if (isMainInstance && sessionsManager()->hasProjectsForRestore()) {
        modeType = StartupModeType::Recovery;
    }

    Uri startupUri = startupPageUri(modeType);

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

    interactive()->open(startupUri);
}

bool StartupScenario::startupCompleted() const
{
    return m_startupCompleted;
}

QList<QVariantMap> StartupScenario::welcomeDialogData() const
{
    QVariantMap item1;
    item1.insert("title", muse::qtrc("appshell/welcome", "Enjoy free cloud storage"));
    item1.insert("imageUrl", "qrc:/resources/welcomedialog/MuseScoreCom.png");
    item1.insert("description", muse::qtrc("appshell/welcome",
                                           "Save your scores privately on MuseScore.com to revisit past versions and invite others to view and comment – and when you’re ready, share your music with the world."));
    item1.insert("buttonText", muse::qtrc("appshell/welcome", "View my scores online"));
    item1.insert("destinationUrl",
                 "https://musescore.com/my-scores?utm_source=mss-app-welcome-musescore-com&utm_medium=mss-app-welcome-musescore-com&utm_campaign=mss-app-welcome-musescore-com");

    QVariantMap item2;
    item2.insert("title", muse::qtrc("appshell/welcome", "What’s new in MuseScore Studio"));
    item2.insert("imageUrl", "qrc:/resources/welcomedialog/WhatsNew.png");
    item2.insert("description", muse::qtrc("appshell/welcome",
                                           "Includes a new system for hiding empty staves, a new text editing widget, guitar notation improvements, engraving improvements and more."));
    item2.insert("buttonText", muse::qtrc("appshell/welcome", "Watch video"));
    item2.insert("destinationUrl",
                 "https://www.youtube.com/watch?v=J2gY9CbMuoI&utm_source=mss-app-yt-4.6-release&utm_medium=mss-app-yt-4.6-release&utm_campaign=mss-app-yt-4.6-release");

    QVariantMap item3;
    item3.insert("title", muse::qtrc("appshell/welcome", "Install our free MuseSounds libraries"));
    item3.insert("imageUrl", "qrc:/resources/welcomedialog/MuseSounds.png");
    item3.insert("description", muse::qtrc("appshell/welcome",
                                           "Explore our collection of realistic sample libraries, including solo instruments, marching percussion, and full orchestra - available for free on MuseHub."));
    item3.insert("buttonText", muse::qtrc("appshell/welcome", "Get it on MuseHub"));
    item3.insert("destinationUrl",
                 "https://www.musehub.com/free-musesounds?utm_source=mss-app-welcome-free-musesounds&utm_medium=mss-app-welcome-free-musesounds&utm_campaign=mss-app-welcome-free-musesounds&utm_id=mss-app-welcome-free-musesounds");

    QVariantMap item4;
    item4.insert("title", muse::qtrc("appshell/welcome", "Explore our tutorials"));
    item4.insert("imageUrl", "qrc:/resources/welcomedialog/ExploreTutorials.png");
    item4.insert("description", muse::qtrc("appshell/welcome",
                                           "We’ve put together a playlist of tutorials to help both beginners and experienced users get the most out of MuseScore Studio."));
    item4.insert("buttonText", muse::qtrc("appshell/welcome", "View tutorials"));
    item4.insert("destinationUrl",
                 "https://www.youtube.com/playlist?list=PLTYuWi2LmaPECOZrC6bkPHBkYY9_WEexT&utm_source=mss-app-welcome-tutorials&utm_medium=mss-app-welcome-tutorials&utm_campaign=mss-app-welcome-tutorials&utm_id=mss-app-welcome-tutorials");

    return { item1, item2, item3, item4 };
}

void StartupScenario::showWelcomeDialog()
{
    interactive()->openSync(WELCOME_DIALOG_URI);

    const std::string version = configuration()->museScoreVersion();
    configuration()->setWelcomeDialogLastShownVersion(version);
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

    bool shouldCheckForMuseSamplerUpdate = false;

    switch (modeType) {
    case StartupModeType::StartEmpty:
        shouldCheckForMuseSamplerUpdate = true;
        break;
    case StartupModeType::StartWithNewScore:
        shouldCheckForMuseSamplerUpdate = true;
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

    const auto showWelcomeDialogAndSamplerUpdateIfNeed = [this, shouldCheckForMuseSamplerUpdate]() {
        //! NOTE: The welcome dialog should not show if the first launch setup has not been completed, or if we're going
        //! to show a MuseSounds update dialog (see ProjectActionsController::doFinishOpenProject). MuseSampler's update
        //! dialog should be shown after the welcome dialog.

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

        const size_t numInstances = multiInstancesProvider()->instances().size();
        if (numInstances == 1 && configuration()->welcomeDialogShowOnStartup() && !museSoundsUpdateScenario()->hasUpdate()) {
            showWelcomeDialog();
        }

        if (shouldCheckForMuseSamplerUpdate) {
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

void StartupScenario::checkAndShowMuseSamplerUpdateIfNeed()
{
    if (!museSamplerCheckForUpdateScenario() || museSamplerCheckForUpdateScenario()->alreadyChecked()) {
        return;
    }
    museSamplerCheckForUpdateScenario()->checkAndShowUpdateIfNeed();
}

muse::Uri StartupScenario::startupPageUri(StartupModeType modeType) const
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
