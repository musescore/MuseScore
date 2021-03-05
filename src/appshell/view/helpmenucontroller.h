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
#ifndef MU_APPSHELL_HELPMENUCONTROLLER_H
#define MU_APPSHELL_HELPMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../iapplicationactioncontroller.h"

namespace mu::appshell {
class HelpMenuController : public async::Asyncable
{
    INJECT(appshell, IApplicationActionController, controller)

public:
    HelpMenuController();

    async::Channel<std::vector<actions::ActionCode> > actionsAvailableChanged() const;

    bool isShowToursAvailable() const;
    bool isResetToursAvailable()const;
    bool isOnlineHandbookAvailable() const;
    bool isAboutAvailable() const;
    bool isAboutQtAvailable() const;
    bool isAboutMusicXMLAVailable() const;
    bool isCheckUpdateAvailable() const;
    bool isAskForHelpAvailable() const;
    bool isBugReportAvailable() const;
    bool isLeaveFeedbackAvailable() const;
    bool isRevertToFactorySettingsAvailable() const;

private:
    async::Channel<std::vector<actions::ActionCode> > m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_HELPMENUCONTROLLER_H
