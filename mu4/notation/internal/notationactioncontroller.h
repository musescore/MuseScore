//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONACTIONCONTROLLER_H
#define MU_NOTATION_NOTATIONACTIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "inotation.h"
#include "iinteractive.h"

namespace mu {
namespace notation {
class NotationActionController : public actions::Actionable
{
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, framework::IInteractive, interactive)

public:

    void init();

private:

    bool canReceiveAction(const actions::ActionName& action) const override;

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;

    void toggleNoteInput();
    void padNote(const Pad& pad);
    void putNote(const actions::ActionData& data);

    void moveAction(const actions::ActionName& action);
    void moveText(INotationInteractionPtr interaction, const actions::ActionName& action);

    void deleteSelection();
    void openPageStyle();
};
}
}

#endif // MU_NOTATION_NOTATIONACTIONCONTROLLER_H
