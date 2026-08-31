/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "appshellcommandscontroller.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileOpenEvent>
#include <QWindow>
#include <QMimeData>

#include "global/async/async.h"
#include "global/types/ret.h"
#include "global/translation.h"
#include "global/containers.h"

#include "audio/common/soundfonttypes.h"

#include "rcommand/actiontocommand.h"

#include "../appshellcommands.h"

#include "log.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace mu::appshell;

static const std::map<rcommand::Command, DockName> s_dockToggleCommands = {
    { DOCK_TOGGLE_PLAYBACK_COMMAND, PLAYBACK_TOOLBAR_NAME },
    { DOCK_TOGGLE_NOTEINPUT_COMMAND, NOTE_INPUT_BAR_NAME },
    { DOCK_TOGGLE_PALETTES_COMMAND, PALETTES_PANEL_NAME },
    { DOCK_TOGGLE_INSTRUMENTS_COMMAND, LAYOUT_PANEL_NAME },
    { DOCK_TOGGLE_PROPERTIES_COMMAND, PROPERTIES_PANEL_NAME },
    { DOCK_TOGGLE_SELECTION_FILTER_COMMAND, SELECTION_FILTERS_PANEL_NAME },
    { DOCK_TOGGLE_UNDO_HISTORY_COMMAND, UNDO_HISTORY_PANEL_NAME },
    { DOCK_TOGGLE_NAVIGATOR_COMMAND, NOTATION_NAVIGATOR_PANEL_NAME },
    { DOCK_TOGGLE_BRAILLE_COMMAND, NOTATION_BRAILLE_PANEL_NAME },
    { DOCK_TOGGLE_TIMELINE_COMMAND, TIMELINE_PANEL_NAME },
    { DOCK_TOGGLE_MIXER_COMMAND, MIXER_PANEL_NAME },
    { DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND, PIANO_KEYBOARD_PANEL_NAME },
    { DOCK_TOGGLE_PERCUSSION_COMMAND, PERCUSSION_PANEL_NAME },
    { DOCK_TOGGLE_STATUSBAR_COMMAND, NOTATION_STATUSBAR_NAME },
};

void AppshellCommandsController::preInit()
{
    qApp->installEventFilter(this);
}

void AppshellCommandsController::init()
{
    auto cd = commandDispatcher();
    cd->onRequest(this, APP_QUIT_COMMAND, [this](const rcommand::Params& params) { return quit(params); });
    cd->onRequest(this, APP_RESTART_COMMAND, [this]() { restart(); return muse::make_ok(); });
    cd->onRequest(this, APP_FULLSCREEN_COMMAND, [this]() { toggleFullScreen(); return muse::make_ok(); });

    cd->onRequest(this, APP_ABOUT_MUSESCORE_COMMAND, [this]() { openAboutDialog(); return muse::make_ok(); });
    cd->onRequest(this, APP_ABOUT_QT_COMMAND, [this]() { openAboutQtDialog(); return muse::make_ok(); });
    cd->onRequest(this, APP_ABOUT_MUSICXML_COMMAND, [this]() { openAboutMusicXMLDialog(); return muse::make_ok(); });
    cd->onRequest(this, APP_ONLINE_HANDBOOK_COMMAND, [this]() { openOnlineHandbookPage(); return muse::make_ok(); });
    cd->onRequest(this, APP_ASK_HELP_COMMAND, [this]() { openAskForHelpPage(); return muse::make_ok(); });

    cd->onRequest(this, APP_ACCESSIBILITY_STATEMENT_COMMAND, [this]() { openAccessibilityStatementPage(); return muse::make_ok(); });
    cd->onRequest(this, APP_PREFERENCES_COMMAND, [this]() { openPreferencesDialog(); return muse::make_ok(); });
    cd->onRequest(this, APP_REVERT_TO_FACTORY_COMMAND, [this]() { revertToFactorySettings(); return muse::make_ok(); });
    cd->onRequest(this, APP_EXTENSIONS_COMMAND, [this]() { openExtensions(); return muse::make_ok(); });

    for (const auto& [command, dockName] : s_dockToggleCommands) {
        cd->onRequest(this, command, [this, dockName]() {
            m_dockToggleRequested.send(dockName);
            return muse::make_ok();
        });
    }

    // compat
    {
        using namespace muse::rcommand;
        static const std::vector<ActionToCommand> actionToCommands = {
            { "quit", APP_QUIT_COMMAND, make_conv({ { "all_instances", param<bool> }, { "installer_path", param<io::path_t> } }) },
            { "restart", APP_RESTART_COMMAND, {} },
            { "fullscreen", APP_FULLSCREEN_COMMAND, {} },
            { "about-musescore", APP_ABOUT_MUSESCORE_COMMAND, {} },
            { "about-qt", APP_ABOUT_QT_COMMAND, {} },
            { "about-musicxml", APP_ABOUT_MUSICXML_COMMAND, {} },
            { "online-handbook", APP_ONLINE_HANDBOOK_COMMAND, {} },
            { "ask-help", APP_ASK_HELP_COMMAND, {} },
            { "accessibility-statement", APP_ACCESSIBILITY_STATEMENT_COMMAND, {} },
            { "preference-dialog", APP_PREFERENCES_COMMAND, {} },
            { "revert-factory", APP_REVERT_TO_FACTORY_COMMAND, {} },
            { "manage-plugins", APP_EXTENSIONS_COMMAND, {} },
            { "toggle-transport", DOCK_TOGGLE_PLAYBACK_COMMAND, {} },
            { "toggle-noteinput", DOCK_TOGGLE_NOTEINPUT_COMMAND, {} },
            { "toggle-palettes", DOCK_TOGGLE_PALETTES_COMMAND, {} },
            { "toggle-instruments", DOCK_TOGGLE_INSTRUMENTS_COMMAND, {} },
            { "toggle-properties-panel", DOCK_TOGGLE_PROPERTIES_COMMAND, {} },
            { "toggle-selection-filter", DOCK_TOGGLE_SELECTION_FILTER_COMMAND, {} },
            { "toggle-undo-history-panel", DOCK_TOGGLE_UNDO_HISTORY_COMMAND, {} },
            { "toggle-navigator", DOCK_TOGGLE_NAVIGATOR_COMMAND, {} },
            { "toggle-braille-panel", DOCK_TOGGLE_BRAILLE_COMMAND, {} },
            { "toggle-timeline", DOCK_TOGGLE_TIMELINE_COMMAND, {} },
            { "toggle-mixer", DOCK_TOGGLE_MIXER_COMMAND, {} },
            { "toggle-piano-keyboard", DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND, {} },
            { "toggle-percussion-panel", DOCK_TOGGLE_PERCUSSION_COMMAND, {} },
            { "toggle-statusbar", DOCK_TOGGLE_STATUSBAR_COMMAND, {} },
        };

        rcommand::registerActionToCommand(this, actionToCommands, commandDispatcher(), dispatcher());
    }
}

bool AppshellCommandsController::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::Close && watched == qWindow())
        || event->type() == QEvent::Quit) {
        bool accepted = quit(false);
        event->setAccepted(accepted);

        return true;
    }

    if (watched == qApp) {
        if (event->type() == QEvent::FileOpen) {
            const QFileOpenEvent* openEvent = static_cast<const QFileOpenEvent*>(event);
            const QUrl url = openEvent->url();

            if (projectFilesController()->isUrlSupported(url)) {
                if (startupScenario()->startupCompleted()) {
                    dispatcher()->dispatch("file-open", actions::ActionData::make_arg1<QUrl>(url));
                } else {
                    startupScenario()->setStartupScoreFile(project::ProjectFile { url });
                }

                return true;
            }
        }
    }

    if (watched == qWindow()) {
        if (event->type() == QEvent::DragEnter) {
            if (onDragEnterEvent(static_cast<QDragEnterEvent*>(event))) {
                return true;
            }
        } else if (event->type() == QEvent::DragMove) {
            if (onDragMoveEvent(static_cast<QDragMoveEvent*>(event))) {
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            if (onDropEvent(static_cast<QDropEvent*>(event))) {
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

muse::rcommand::Command AppshellCommandsController::dockToggleCommand(const DockName& dockName) const
{
    return muse::key(s_dockToggleCommands, dockName);
}

DockName AppshellCommandsController::commandDockName(const muse::rcommand::Command& command) const
{
    return muse::value(s_dockToggleCommands, command);
}

muse::async::Channel<DockName> AppshellCommandsController::dockToggleRequested() const
{
    return m_dockToggleRequested;
}

QWindow* AppshellCommandsController::qWindow() const
{
    return mainWindow() ? mainWindow()->qWindow() : nullptr;
}

AppshellCommandsController::DragTarget AppshellCommandsController::dragTarget(const QUrl& url) const
{
    if (projectFilesController()->isUrlSupported(url)) {
        return DragTarget::ProjectFile;
    } else if (url.isLocalFile()) {
        muse::io::path_t filePath = url.toLocalFile();
        if (muse::audio::synth::isSoundFont(filePath)) {
            return DragTarget::SoundFont;
        } else if (extensionInstaller()->isFileSupported(filePath)) {
            return DragTarget::Extension;
        }
    }
    return DragTarget::Unknown;
}

bool AppshellCommandsController::onDragEnterEvent(QDragEnterEvent* event)
{
    return onDragMoveEvent(event);
}

bool AppshellCommandsController::onDragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mime = event->mimeData();
    QList<QUrl> urls = mime->urls();
    if (urls.count() > 0) {
        const QUrl& url = urls.front();
        DragTarget target = dragTarget(url);
        if (target != DragTarget::Unknown) {
            event->setDropAction(Qt::LinkAction);
            event->acceptProposedAction();
            return true;
        }
    }

    return false;
}

bool AppshellCommandsController::onDropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    QList<QUrl> urls = mime->urls();
    if (urls.count() > 0) {
        const QUrl& url = urls.front();

        bool shouldBeHandled = true;
        DragTarget target = dragTarget(url);
        switch (target) {
        case DragTarget::ProjectFile: {
            async::Async::call(this, [this, url]() {
                    Ret ret = projectFilesController()->openProject(url);
                    if (!ret) {
                        LOGE() << ret.toString();
                    }
                });
        } break;
        case DragTarget::SoundFont: {
            muse::io::path_t filePath = url.toLocalFile();
            async::Async::call(this, [this, filePath]() {
                    soundFontInstallScenario()->installSoundFont(Uri::fromLocalFile(filePath));
                });
        } break;
        case DragTarget::Extension: {
            muse::io::path_t filePath = url.toLocalFile();
            async::Async::call(this, [this, filePath]() {
                    extensionInstaller()->installExtension(filePath);
                });
        } break;
        case DragTarget::Unknown:
            shouldBeHandled = false;
            break;
        }

        if (shouldBeHandled) {
            event->accept();
        } else {
            event->ignore();
        }

        return shouldBeHandled;
    }

    return false;
}

muse::Ret AppshellCommandsController::quit(const muse::rcommand::Params& params)
{
    bool isAllInstances = params.at("all_instances").toBool();
    muse::io::path_t installatorPath = params.at("installer_path").toString();
    return quit(isAllInstances, installatorPath);
}

muse::Ret AppshellCommandsController::quit(bool isAllInstances, const muse::io::path_t& installerPath)
{
    if (m_quiting) {
        return muse::make_ret(Ret::Code::Busy);
    }

    m_quiting = true;

    if (!projectFilesController()->closeOpenedProject(false)) {
        m_quiting = false;
        return muse::make_ret(Ret::Code::UnknownError);
    }

    if (multiwindowsProvider()->isFirstWindow() && !installerPath.empty()) {
        //! NOTE: All windows are quitting to complete the update, apply it
        //! in-place, falling back to handing the package to the user.
        bool applied = false;
        if (appUpdateService()->canAutoInstall()) {
            const muse::RetVal<muse::io::path_t> prepared = appUpdateService()->prepareUpdate(installerPath);
            if (prepared.ret) {
                applied = bool(appUpdateService()->finalizeUpdate(prepared.val));
            }
        }

        if (!applied) {
#if defined(Q_OS_LINUX)
            platformInteractive()->revealInFileBrowser(installerPath);
#else
            platformInteractive()->openUrl(QUrl::fromLocalFile(installerPath.toQString()));
#endif
        }
    }

    if (!multiwindowsProvider()->isFirstWindow()) {
        multiwindowsProvider()->notifyAboutWindowWasQuited();
    }

    //! NOTE the following destroys the IoC context and `this` with it,
    //! so don't access `this` after this point!
    if (isAllInstances) {
        multiwindowsProvider()->quitForAll();
    } else {
        multiwindowsProvider()->quitWindow(iocContext());
    }

    return muse::make_ok();
}

void AppshellCommandsController::restart()
{
    if (projectFilesController()->closeOpenedProject(false)) {
        if (multiwindowsProvider()->windowCount() == 1) {
            application()->restart();
        } else {
            multiwindowsProvider()->quitAllAndRestartLast();

            QCoreApplication::exit();
        }
    }
}

void AppshellCommandsController::toggleFullScreen()
{
    mainWindow()->toggleFullScreen();
}

void AppshellCommandsController::openAboutDialog()
{
    interactive()->open("musescore://about/musescore");
}

void AppshellCommandsController::openAboutQtDialog()
{
    QApplication::aboutQt();
}

void AppshellCommandsController::openAboutMusicXMLDialog()
{
    interactive()->open("musescore://about/musicxml");
}

void AppshellCommandsController::openOnlineHandbookPage()
{
    std::string handbookUrl = configuration()->handbookUrl();
    platformInteractive()->openUrl(handbookUrl);
}

void AppshellCommandsController::openAskForHelpPage()
{
    std::string askForHelpUrl = configuration()->askForHelpUrl();
    platformInteractive()->openUrl(askForHelpUrl);
}

void AppshellCommandsController::openAccessibilityStatementPage()
{
    std::string accessibilityStatementUrl = configuration()->accessibilityStatementUrl();
    platformInteractive()->openUrl(accessibilityStatementUrl);
}

void AppshellCommandsController::openPreferencesDialog()
{
    const context::IPlaybackStatePtr state = globalContext()->playbackState();
    if (state->isPlaying()) {
        commandDispatcher()->dispatch(rcommand::Command("command://playback/stop"));

        async::Channel<audio::PlaybackStatus> statusChanged = state->playbackStatusChanged();
        statusChanged.onReceive(this, [statusChanged, this](audio::PlaybackStatus) {
            auto statusChangedMut = statusChanged;
            statusChangedMut.disconnect(this);
            doOpenPreferencesDialog();
        });

        return;
    }

    doOpenPreferencesDialog();
}

void AppshellCommandsController::doOpenPreferencesDialog()
{
    if (multiwindowsProvider()->isPreferencesAlreadyOpened()) {
        multiwindowsProvider()->activateWindowWithOpenedPreferences();
        return;
    }

    interactive()->open("muse://preferences");
}

void AppshellCommandsController::revertToFactorySettings()
{
    std::string title = muse::trc("appshell", "Are you sure you want to revert to factory settings?");
    std::string question = muse::trc("appshell", "This action will reset all your app preferences and delete all custom palettes and custom shortcuts. "
                                                 "The list of recent scores will also be cleared.\n\n"
                                                 "This action will not delete any of your scores.");

    IInteractive::ButtonData cancelBtn = interactive()->buttonData(IInteractive::Button::Cancel);
    cancelBtn.accent = true;

    int revertBtn = int(IInteractive::Button::Apply);
    auto promise = interactive()->warning(title, question,
                                          { cancelBtn,
                                            IInteractive::ButtonData(revertBtn, muse::trc("appshell", "Revert")) },
                                          cancelBtn.btn, { muse::IInteractive::Option::WithIcon },
                                          muse::trc("appshell", "Revert to factory settings"));

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Cancel)) {
            return;
        }

        static constexpr bool KEEP_DEFAULT_SETTINGS = false;
        static constexpr bool NOTIFY_ABOUT_CHANGES = false;
        static constexpr bool NOTIFY_OTHER_INSTANCES = false;
        configuration()->revertToFactorySettings(KEEP_DEFAULT_SETTINGS, NOTIFY_ABOUT_CHANGES, NOTIFY_OTHER_INSTANCES);

        std::string title = muse::trc("appshell", "Would you like to restart MuseScore Studio now?");
        std::string question = muse::trc("appshell", "MuseScore Studio needs to be restarted for these changes to take effect.");

        int restartBtn = int(IInteractive::Button::Apply);
        auto promise = interactive()->question(title, question,
                                               { interactive()->buttonData(IInteractive::Button::Cancel),
                                                 IInteractive::ButtonData(restartBtn, muse::trc("appshell", "Restart"), true) },
                                               restartBtn);

        promise.onResolve(this, [this](const IInteractive::Result& res) {
            if (!res.isButton(IInteractive::Button::Cancel)) {
                restart();
            }
        });
    });
}

void AppshellCommandsController::openExtensions()
{
    interactive()->open("musescore://home?section=plugins");
}
