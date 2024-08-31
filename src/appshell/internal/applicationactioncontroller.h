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
#ifndef MU_APPSHELL_APPLICATIONCONTROLLER_H
#define MU_APPSHELL_APPLICATIONCONTROLLER_H

#include <QObject>

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "async/asyncable.h"
#include "ui/imainwindow.h"
#include "languages/ilanguagesservice.h"
#include "iinteractive.h"
#include "iappshellconfiguration.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "project/iprojectfilescontroller.h"
#include "audio/isoundfontrepository.h"
#include "istartupscenario.h"
#include "iapplication.h"
#include "extensions/iextensioninstaller.h"
#include "context/iglobalcontext.h"

namespace mu::appshell {
class ApplicationActionController : public QObject, public muse::actions::Actionable, public muse::async::Asyncable
{
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher;
    muse::Inject<muse::ui::IUiActionsRegister> actionsRegister;
    muse::Inject<muse::ui::IMainWindow> mainWindow;
    muse::Inject<muse::languages::ILanguagesService> languagesService;
    muse::Inject<muse::IInteractive> interactive;
    muse::Inject<IAppShellConfiguration> configuration;
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider;
    muse::Inject<project::IProjectFilesController> projectFilesController;
    muse::Inject<muse::audio::ISoundFontRepository> soundFontRepository;
    muse::Inject<IStartupScenario> startupScenario;
    muse::Inject<muse::IApplication> application;
    muse::Inject<muse::extensions::IExtensionInstaller> extensionInstaller;
    muse::Inject<context::IGlobalContext> globalContext;

public:
    void preInit();
    void init();

    muse::ValCh<bool> isFullScreen() const;

private:

    enum DragTarget {
        Unknown = 0,
        ProjectFile,
        SoundFont,
        Extension
    };

    bool eventFilter(QObject* watched, QEvent* event) override;

    DragTarget dragTarget(const QUrl& url) const;
    bool onDragEnterEvent(QDragEnterEvent* event);
    bool onDragMoveEvent(QDragMoveEvent* event);
    bool onDropEvent(QDropEvent* event);

    void setupConnections();

    bool quit(bool isAllInstances, const muse::io::path_t& installerPath = muse::io::path_t());
    void restart();

    void toggleFullScreen();
    void openAboutDialog();
    void openAboutQtDialog();
    void openAboutMusicXMLDialog();

    void openOnlineHandbookPage();
    void openAskForHelpPage();
    void openPreferencesDialog();
    void doOpenPreferencesDialog();

    void revertToFactorySettings();

    bool m_quiting = false;

    muse::async::Channel<muse::actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_APPLICATIONCONTROLLER_H
