//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_APPSHELL_APPLICATIONCONTROLLER_H
#define MU_APPSHELL_APPLICATIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "async/asyncable.h"
#include "ui/imainwindow.h"
#include "languages/ilanguagesservice.h"
#include "iinteractive.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class ApplicationActionController : public actions::Actionable, public async::Asyncable
{
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, ui::IUiActionsRegister, actionsRegister)
    INJECT(appshell, ui::IMainWindow, mainWindow)
    INJECT(appshell, languages::ILanguagesService, languagesService)
    INJECT(appshell, framework::IInteractive, interactive)
    INJECT(appshell, IAppShellConfiguration, configuration)

public:
    void init();

    ValCh<bool> isFullScreen() const;

private:
    void setupConnections();

    void quit();
    void toggleFullScreen();
    void openAboutDialog();
    void openAboutQtDialog();
    void openAboutMusicXMLDialog();

    void openOnlineHandbookPage();
    void openAskForHelpPage();
    void openBugReportPage();
    void openLeaveFeedbackPage();
    void openPreferencesDialog();

    void revertToFactorySettings();

    async::Channel<bool> m_fullScreenChannel;
    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_APPLICATIONCONTROLLER_H
