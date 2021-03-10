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

#include "ihelpmenucontroller.h"

namespace mu::appshell {
class HelpMenuController : public IHelpMenuController, public async::Asyncable
{
    INJECT(appshell, IApplicationActionController, controller)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    using AvailableCallback = std::function<bool ()>;
    std::map<actions::ActionCode, AvailableCallback> actions() const;

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
    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_HELPMENUCONTROLLER_H
