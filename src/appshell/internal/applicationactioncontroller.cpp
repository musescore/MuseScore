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

#include "translation.h"

using namespace mu::appshell;

void ApplicationActionController::init()
{
    dispatcher()->reg(this, "quit", this, &ApplicationActionController::quit);

    dispatcher()->reg(this, "online-handbook", this, &ApplicationActionController::openOnlineHandbookPage);
    dispatcher()->reg(this, "ask-help", this, &ApplicationActionController::openAskForHelpPage);
    dispatcher()->reg(this, "report-bug", this, &ApplicationActionController::openBugReportPage);
    dispatcher()->reg(this, "leave-feedback", this, &ApplicationActionController::openLeaveFeedbackPage);
}

void ApplicationActionController::quit()
{
    QCoreApplication::quit();
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
