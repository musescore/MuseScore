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
#include "applicationactioncontroller.h"

#include <QCoreApplication>
#include <QMainWindow>

#include "translation.h"
#include "applicationactions.h"

using namespace mu::appshell;
using namespace mu::framework;
using namespace mu::actions;
using namespace mu::shortcuts;

void ApplicationActionController::init()
{
    dispatcher()->reg(this, "quit", this, &ApplicationActionController::quit);

    dispatcher()->reg(this, "fullscreen", this, &ApplicationActionController::toggleFullScreen);

    dispatcher()->reg(this, "about", this, &ApplicationActionController::openAboutDialog);
    dispatcher()->reg(this, "about-qt", this, &ApplicationActionController::openAboutQtDialog);
    dispatcher()->reg(this, "about-musicxml", this, &ApplicationActionController::openAboutMusicXMLDialog);
    dispatcher()->reg(this, "online-handbook", this, &ApplicationActionController::openOnlineHandbookPage);
    dispatcher()->reg(this, "ask-help", this, &ApplicationActionController::openAskForHelpPage);
    dispatcher()->reg(this, "report-bug", this, &ApplicationActionController::openBugReportPage);
    dispatcher()->reg(this, "leave-feedback", this, &ApplicationActionController::openLeaveFeedbackPage);
    dispatcher()->reg(this, "preference-dialog", this, &ApplicationActionController::openPreferencesDialog);

    dispatcher()->reg(this, "revert-factory", this, &ApplicationActionController::revertToFactorySettings);

    setupConnections();
}

mu::ValCh<bool> ApplicationActionController::isFullScreen() const
{
    ValCh<bool> result;
    result.ch = m_fullScreenChannel;
    // todo
//    result.val = mainWindow()->qMainWindow() ? mainWindow()->qMainWindow()->isFullScreen() : false;
    return result;
}

bool ApplicationActionController::actionAvailable(const mu::actions::ActionCode& actionCode) const
{
    if (!canReceiveAction(actionCode)) {
        return false;
    }

    ActionItem action = actionsRegister()->action(actionCode);
    if (!action.isValid()) {
        return false;
    }

    switch (action.shortcutContext) {
    case ShortcutContext::NotationActive:
        return isNotationPage();
    default:
        break;
    }

    return true;
}

mu::async::Channel<mu::actions::ActionCodeList> ApplicationActionController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

void ApplicationActionController::setupConnections()
{
    interactive()->currentUri().ch.onReceive(this, [this](const Uri&) {
        ActionCodeList actionCodes = ApplicationActions::actionCodes(ShortcutContext::NotationActive);
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

void ApplicationActionController::quit()
{
    QCoreApplication::quit();
}

void ApplicationActionController::toggleFullScreen()
{
    QMainWindow* qMainWindow = mainWindow()->qMainWindow();
    if (!qMainWindow) {
        return;
    }

    if (!qMainWindow->isFullScreen()) {
        qMainWindow->showFullScreen();
    } else {
        qMainWindow->showNormal();
    }

    m_fullScreenChannel.send(qMainWindow->isFullScreen());
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

void ApplicationActionController::openBugReportPage()
{
    std::string bugReportUrl = configuration()->bugReportUrl();
    interactive()->openUrl(bugReportUrl);
}

void ApplicationActionController::openLeaveFeedbackPage()
{
    std::string leaveFeedbackUrl = configuration()->leaveFeedbackUrl();
    interactive()->openUrl(leaveFeedbackUrl);
}

void ApplicationActionController::openPreferencesDialog()
{
    interactive()->open("musescore://preferences");
}

void ApplicationActionController::revertToFactorySettings()
{
    std::string question = trc("appshell", "This will reset all your preferences.\n"
                                           "Custom palettes, custom shortcuts, and the list of recent scores will be deleted. "
                                           "Reverting will not remove any scores from your computer.\n"
                                           "Are you sure you want to proceed?");

    IInteractive::Button button = interactive()->question(std::string(), question, {
        IInteractive::Button::Yes,
        IInteractive::Button::No
    });

    if (button == IInteractive::Button::Yes) {
        configuration()->revertToFactorySettings();
    }
}

bool ApplicationActionController::isNotationPage() const
{
    return interactive()->isOpened("musescore://notation").val;
}
