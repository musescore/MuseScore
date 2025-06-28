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
#include "applicationactioncontroller.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileOpenEvent>
#include <QWindow>
#include <QMimeData>

#include "async/async.h"
#include "audio/soundfonttypes.h"

#include "defer.h"
#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

void ApplicationActionController::preInit()
{
    qApp->installEventFilter(this);
}

void ApplicationActionController::init()
{
    dispatcher()->reg(this, "quit", [this](const ActionData& args) {
        bool isAllInstances = args.count() > 0 ? args.arg<bool>(0) : true;
        muse::io::path_t installatorPath = args.count() > 1 ? args.arg<muse::io::path_t>(1) : "";
        quit(isAllInstances, installatorPath);
    });

    dispatcher()->reg(this, "restart", [this]() {
        restart();
    });

    dispatcher()->reg(this, "fullscreen", this, &ApplicationActionController::toggleFullScreen);

    dispatcher()->reg(this, "about-musescore", this, &ApplicationActionController::openAboutDialog);
    dispatcher()->reg(this, "about-qt", this, &ApplicationActionController::openAboutQtDialog);
    dispatcher()->reg(this, "about-musicxml", this, &ApplicationActionController::openAboutMusicXMLDialog);
    dispatcher()->reg(this, "online-handbook", this, &ApplicationActionController::openOnlineHandbookPage);
    dispatcher()->reg(this, "ask-help", this, &ApplicationActionController::openAskForHelpPage);
    dispatcher()->reg(this, "preference-dialog", this, &ApplicationActionController::openPreferencesDialog);

    dispatcher()->reg(this, "revert-factory", this, &ApplicationActionController::revertToFactorySettings);

    dispatcher()->reg(this, "manage-plugins", [this]() {
        interactive()->open("musescore://home?section=plugins");
    });
}

bool ApplicationActionController::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::Close && watched == mainWindow()->qWindow())
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
                    dispatcher()->dispatch("file-open", ActionData::make_arg1<QUrl>(url));
                } else {
                    startupScenario()->setStartupScoreFile(project::ProjectFile { url });
                }

                return true;
            }
        }
    }

    if (watched == mainWindow()->qWindow()) {
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

ApplicationActionController::DragTarget ApplicationActionController::dragTarget(const QUrl& url) const
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

bool ApplicationActionController::onDragEnterEvent(QDragEnterEvent* event)
{
    return onDragMoveEvent(event);
}

bool ApplicationActionController::onDragMoveEvent(QDragMoveEvent* event)
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

bool ApplicationActionController::onDropEvent(QDropEvent* event)
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
                    soundFontRepository()->addSoundFont(filePath);
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

bool ApplicationActionController::quit(bool isAllInstances, const muse::io::path_t& installerPath)
{
    if (m_quiting) {
        return false;
    }

    m_quiting = true;
    DEFER {
        m_quiting = false;
    };

    if (!projectFilesController()->closeOpenedProject(false)) {
        return false;
    }

    if (isAllInstances) {
        multiInstancesProvider()->quitForAll();
    }

    if (multiInstancesProvider()->instances().size() == 1 && !installerPath.empty()) {
#if defined(Q_OS_LINUX)
        interactive()->revealInFileBrowser(installerPath);
#else
        interactive()->openUrl(QUrl::fromLocalFile(installerPath.toQString()));
#endif
    }

    if (multiInstancesProvider()->instances().size() > 1) {
        multiInstancesProvider()->notifyAboutInstanceWasQuited();
    }

    QCoreApplication::exit();
    return true;
}

void ApplicationActionController::restart()
{
    if (projectFilesController()->closeOpenedProject(false)) {
        if (multiInstancesProvider()->instances().size() == 1) {
            application()->restart();
        } else {
            multiInstancesProvider()->quitAllAndRestartLast();

            QCoreApplication::exit();
        }
    }
}

void ApplicationActionController::toggleFullScreen()
{
    mainWindow()->toggleFullScreen();
}

void ApplicationActionController::openAboutDialog()
{
    interactive()->open("musescore://about/musescore");
}

void ApplicationActionController::openAboutQtDialog()
{
    QApplication::aboutQt();
}

void ApplicationActionController::openAboutMusicXMLDialog()
{
    interactive()->open("musescore://about/musicxml");
}

void ApplicationActionController::openOnlineHandbookPage()
{
    std::string handbookUrl = configuration()->handbookUrl();
    interactive()->openUrl(handbookUrl);
}

void ApplicationActionController::openAskForHelpPage()
{
    std::string askForHelpUrl = configuration()->askForHelpUrl();
    interactive()->openUrl(askForHelpUrl);
}

void ApplicationActionController::openPreferencesDialog()
{
    const context::IPlaybackStatePtr state = globalContext()->playbackState();
    if (state->playbackStatus() == audio::PlaybackStatus::Running) {
        dispatcher()->dispatch("stop");

        async::Channel<audio::PlaybackStatus> statusChanged = state->playbackStatusChanged();
        statusChanged.onReceive(this, [statusChanged, this](audio::PlaybackStatus) {
            auto statusChangedMut = statusChanged;
            statusChangedMut.resetOnReceive(this);
            doOpenPreferencesDialog();
        });

        return;
    }

    doOpenPreferencesDialog();
}

void ApplicationActionController::doOpenPreferencesDialog()
{
    if (multiInstancesProvider()->isPreferencesAlreadyOpened()) {
        multiInstancesProvider()->activateWindowWithOpenedPreferences();
        return;
    }

    interactive()->open("muse://preferences");
}

void ApplicationActionController::revertToFactorySettings()
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
