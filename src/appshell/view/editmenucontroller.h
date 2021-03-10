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
#ifndef MU_APPSHELL_EDITMENUCONTROLLER_H
#define MU_APPSHELL_EDITMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "notation/inotationactionscontroller.h"
#include "notation/notationtypes.h"

#include "ieditmenucontroller.h"

namespace mu::appshell {
class EditMenuController : public IEditMenuController, public async::Asyncable
{
    INJECT(appshell, notation::INotationActionsController, controller)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    using AvailableCallback = std::function<bool ()>;
    std::map<actions::ActionCode, AvailableCallback> actions() const;

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    bool isCutAvailable() const;
    bool isCopyAvailable() const;
    bool isPasteAvailable(notation::PastingType pastingType) const;
    bool isSwapAvailable() const;
    bool isDeleteAvailable() const;
    bool isSelectSimilarAvailable() const;
    bool isSelectAllAvailable() const;
    bool isFindAvailable() const;
    bool isPreferenceDialogAvailable() const;

    std::string pastingActionCode(notation::PastingType type) const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_EDITMENUCONTROLLER_H
