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
#include "../notationtypes.h"
#include "../iaddmenucontroller.h"

namespace mu::notation {
class AddMenuController : public IAddMenuController, public async::Asyncable
{
    INJECT(notation, INotationActionsController, controller)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    using AvailableCallback = std::function<bool ()>;
    std::map<actions::ActionCode, AvailableCallback> actions() const;

    bool isNoteInputAvailable() const;
    bool isNoteAvailable(NoteName noteName, NoteAddingMode addingMode) const;
    bool isIntervalAvailable(int interval, IntervalType intervalType) const;
    bool isTupletAvailable(TupletType tupletType) const;
    bool isTupletDialogAvailable() const;
    bool isMeasuresAvailable(ElementChangeOperation operation, int count) const;
    bool isBoxAvailable(ElementChangeOperation operation, BoxType boxType) const;
    bool isTextAvailable(TextType textType) const;
    bool isFiguredBassAvailable() const;
    bool isSlurAvailable() const;
    bool isHarpinAvailable(HairpinType hairpinType) const;
    bool isOttavaAvailable(OttavaType ottavaType) const;
    bool isNoteLineAvailable() const;

    std::string noteAddingModeActionCode(NoteAddingMode mode) const;
    std::string intervalTypeActionCode(IntervalType type) const;
    std::string measuresActionCode(int count) const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_NOTATION_ADDMENUCONTROLLER_H
