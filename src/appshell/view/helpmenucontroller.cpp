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
#include "helpmenucontroller.h"

#include "log.h"

using namespace mu::appshell;

HelpMenuController::HelpMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > HelpMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

bool HelpMenuController::isShowToursAvailable() const
{
    return controller()->actionAvailable("show-tours");
}

bool HelpMenuController::isResetToursAvailable() const
{
    return controller()->actionAvailable("reset-tours");
}

bool HelpMenuController::isOnlineHandbookAvailable() const
{
    return controller()->actionAvailable("online-handbook");
}

bool HelpMenuController::isAboutAvailable() const
{
    return controller()->actionAvailable("about");
}

bool HelpMenuController::isAboutQtAvailable() const
{
    return controller()->actionAvailable("about-qt");
}

bool HelpMenuController::isAboutMusicXMLAVailable() const
{
    return controller()->actionAvailable("about-musicxml");
}

bool HelpMenuController::isCheckUpdateAvailable() const
{
    return controller()->actionAvailable("check-update");
}

bool HelpMenuController::isAskForHelpAvailable() const
{
    return controller()->actionAvailable("ask-help");
}

bool HelpMenuController::isBugReportAvailable() const
{
    return controller()->actionAvailable("report-bug");
}

bool HelpMenuController::isLeaveFeedbackAvailable() const
{
    return controller()->actionAvailable("leave-feedback");
}

bool HelpMenuController::isRevertToFactorySettingsAvailable() const
{
    return controller()->actionAvailable("revert-factory");
}
