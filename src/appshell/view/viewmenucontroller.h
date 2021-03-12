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
#ifndef MU_APPSHELL_VIEWMENUCONTROLLER_H
#define MU_APPSHELL_VIEWMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iapplicationactioncontroller.h"
#include "notation/inotationactionscontroller.h"
#include "palette/ipaletteactionscontroller.h"
#include "appshelltypes.h"
#include "notation/notationtypes.h"
#include "inotationpagestate.h"
#include "context/iglobalcontext.h"

#include "uicomponents/imenucontroller.h"

namespace mu::appshell {
class ViewMenuController : public uicomponents::IMenuController, public async::Asyncable
{
    INJECT(appshell, notation::INotationActionsController, notationController)
    INJECT(appshell, palette::IPaletteActionsController, paletteController)
    INJECT(appshell, IApplicationActionController, applicationController)

    INJECT(appshell, INotationPageState, notationPageState)
    INJECT(appshell, context::IGlobalContext, globalContext)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;
    uicomponents::ActionState actionState(const actions::ActionCode& actionCode) const override;

    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr notationInteraction() const;

    actions::ActionCodeList notationControllerActions() const;
    actions::ActionCodeList paletteControllerActions() const;
    actions::ActionCodeList applicationControllerActions() const;

    bool notationControllerContains(const actions::ActionCode& actionCode) const;
    bool paletteControllerContains(const actions::ActionCode& actionCode) const;
    bool applicationControllerContains(const actions::ActionCode& actionCode) const;

    bool actionEnabled(const actions::ActionCode& actionCode) const;
    bool actionCheckable(const actions::ActionCode& actionCode) const;
    bool actionChecked(const actions::ActionCode& actionCode) const;

    actions::ActionCodeList panelsActions() const;

    PanelType panelType(const actions::ActionCode& actionCode) const;
    actions::ActionCode panelTypeActionCode(PanelType type) const;
    bool isPanelVisible(PanelType panelType) const;

    notation::ScoreConfigType scoreConfigType(const actions::ActionCode& actionCode) const;
    bool isScoreConfigChecked(notation::ScoreConfigType configType) const;

    bool isFullScreen() const;
    bool isMasterPaletteOpened() const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_NOTATION_VIEWMENUCONTROLLER_H
