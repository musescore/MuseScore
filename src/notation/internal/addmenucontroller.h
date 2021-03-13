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
#ifndef MU_NOTATION_ADDMENUCONTROLLER_H
#define MU_NOTATION_ADDMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../inotationactionscontroller.h"
#include "context/iglobalcontext.h"

#include "uicomponents/imenucontroller.h"

namespace mu::notation {
class AddMenuController : public uicomponents::IMenuController, public async::Asyncable
{
    INJECT(notation, INotationActionsController, controller)
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;
    uicomponents::ActionState actionState(const actions::ActionCode& actionCode) const override;

    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    IMasterNotationPtr currentMasterNotation() const;
    INotationPtr currentNotation() const;
    INotationInteractionPtr notationInteraction() const;
    INotationNoteInputPtr notationNoteInput() const;

    actions::ActionCodeList actions() const;

    bool actionEnabled(const actions::ActionCode& actionCode) const;
    bool actionCheckable(const actions::ActionCode& actionCode) const;
    bool actionChecked(const actions::ActionCode& actionCode) const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_NOTATION_ADDMENUCONTROLLER_H
