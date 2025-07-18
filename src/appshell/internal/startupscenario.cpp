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
#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

static const muse::UriQuery FIRST_LAUNCH_SETUP_URI("musescore://firstLaunchSetup?floating=true");
static const muse::UriQuery WELCOME_DIALOG_URI("musescore://welcomedialog");
static const muse::Uri HOME_URI("musescore://home");
static const muse::Uri NOTATION_URI("musescore://notation");

static constexpr int AUTO_CHECK_UPDATE_INTERVAL(1000);
static constexpr int CHECK_FOR_UPDATES_TIMEOUT(60000);

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

StartupScenario::StartupScenario(const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
    m_splashScreenProgress = std::make_shared<Progress>();
    m_splashScreenProgress->started().onNotify(this, [this] {
        runOnSplashScreen();
    });
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
    IF_ASSERT_FAILED(m_splashScreenProgress && m_splashScreenProgress->isStarted()) {
        return;
    }

    registerAudioPlugins();

    if (multiInstancesProvider()->instances().size() != 1) {
        m_splashScreenProgress->finish(make_ret(Ret::Code::Cancel));
        return;
    }

    QTimer::singleShot(CHECK_FOR_UPDATES_TIMEOUT, [this]() { // TODO: Check the connection first...
        if (m_splashScreenProgress && m_splashScreenProgress->isStarted()) {
            m_splashScreenProgress->finish(make_ret(Ret::Code::UnknownError));
        }
    });

    static size_t totalChecksExpected = 0;

    const auto onUpdateCheckCompleted = [this](){
        static size_t totalChecksReceived = 0;

        const bool checkInProgress = m_splashScreenProgress && m_splashScreenProgress->isStarted();
        IF_ASSERT_FAILED(checkInProgress && totalChecksReceived < totalChecksExpected) {
            return;
        }
        ++totalChecksReceived;

        // TODO: Could give a specific progress update based on the number of checks completed and the
        // total number of checks we're expecting...

        if (totalChecksReceived == totalChecksExpected) {
            m_splashScreenProgress->finish(make_ret(Ret::Code::Ok));
        }
    };

    const bool canCheckAppUpdate = appUpdateScenario() && appUpdateConfiguration()
                                   && appUpdateConfiguration()->needCheckForUpdate();
    if (canCheckAppUpdate) {
        ++totalChecksExpected;
    }

    const bool canCheckMuseSoundsUpdate = museSoundsUpdateScenario() && museSoundsUpdateService()
                                          && museSoundsUpdateService()->needCheckForUpdate();
    if (canCheckMuseSoundsUpdate) {
        ++totalChecksExpected;
    }

    //! NOTE: Only start the checks once we know the total number of expected checks...

    if (canCheckAppUpdate) {
        QTimer::singleShot(AUTO_CHECK_UPDATE_INTERVAL, [this, onUpdateCheckCompleted]() {
            muse::async::Promise<Ret> promise = appUpdateScenario()->checkForUpdate(/*manual*/ false);
            promise.onResolve(this, [onUpdateCheckCompleted](Ret) {
                onUpdateCheckCompleted();
            });
        });
    }

    if (canCheckMuseSoundsUpdate) {
        muse::async::Promise<Ret> promise = museSoundsUpdateScenario()->checkForUpdate(/*manual*/ false);
        promise.onResolve(this, [onUpdateCheckCompleted](Ret) {
            onUpdateCheckCompleted();
        });
    }
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

muse::ProgressPtr StartupScenario::splashScreenProgress()
{
    return m_splashScreenProgress;
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

QList<QVariantMap> StartupScenario::welcomeDialogData() const
{
    // TODO: Placeholder info
    QVariantMap item1;
    item1.insert("title", muse::qtrc("appshell", "What’s new in MuseScore Studio"));
    item1.insert("imageUrl", "https://i.ytimg.com/vi/-I-InDHIzdQ/hq720.jpg");
    item1.insert("description", muse::qtrc("appshell",
                                           "Includes a new system for dynamics, a new input mode, the ability to add system markings anywhere on a score and much more."));
    item1.insert("buttonText", muse::qtrc("appshell", "Watch video"));
    item1.insert("destinationUrl", "https://www.youtube.com/watch?v=-I-InDHIzdQ");

    QVariantMap item2;
    item2.insert("title", "Get amazing playback with MuseSounds!");
    item2.insert("imageUrl", "https://i.ytimg.com/vi/n7UgN69e2Y8/hq720.jpg");
    item2.insert("description", muse::qtrc("appshell",
                                           "Get free MuseSounds for orchestra, guitars & drumline, plus pro libraries from Spitfire, VSL, Berlin, Audio Imperia & more."));
    item2.insert("buttonText", muse::qtrc("appshell", "Get it on MuseHub"));
    item2.insert("destinationUrl", "https://www.musehub.com/");

    QVariantMap item3;
    item3.insert("title",
                 "Placeholder title on two lines - an example where the source text is is extremely long and gets truncated before it can finish.");
    item3.insert("imageUrl", "https://i.ytimg.com/vi/-I-InDHIzdQ/hq720.jpg");
    item3.insert("description",
                 "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    item3.insert("buttonText", muse::qtrc("appshell", "Button text"));

    QVariantMap item4;
    item4.insert("title", muse::qtrc("appshell", "Never lose your work with free cloud storage!"));
    item4.insert("imageUrl", "qrc:/SaveToCloud/images/Cloud.png");
    item4.insert("description", muse::qtrc("appshell",
                                           "While working on your score, your progress is backed up on MuseScore.com"));
    item4.insert("buttonText", muse::qtrc("appshell", "Get free cloud storage now"));
    item4.insert("destinationUrl", "https://musescore.com/");

    return { item1, item2, item3, item4 };
}

void StartupScenario::showWelcomeDialog()
{
    interactive()->open(WELCOME_DIALOG_URI);

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

    const Version welcomeDialogLastShownVersion(configuration()->welcomeDialogLastShownVersion());
    const Version currentMuseScoreVersion(configuration()->museScoreVersion());
    if (welcomeDialogLastShownVersion < currentMuseScoreVersion) {
        configuration()->setWelcomeDialogShowOnStartup(true); // override user preference
        configuration()->setWelcomeDialogLastShownIndex(-1); // reset
    }

    if (!configuration()->hasCompletedFirstLaunchSetup()) {
        interactive()->open(FIRST_LAUNCH_SETUP_URI);
    } else if (configuration()->welcomeDialogShowOnStartup() && !museSoundsUpdateScenario()->hasUpdate()) {
        showWelcomeDialog();
    }
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
        }
    });
}

void StartupScenario::removeProjectsUnsavedChanges(const io::paths_t& projectsPaths)
{
    for (const muse::io::path_t& path : projectsPaths) {
        projectAutoSaver()->removeProjectUnsavedChanges(path);
    }
}
