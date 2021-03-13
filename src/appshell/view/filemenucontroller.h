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
#ifndef MU_APPSHELL_FILEMENUCONTROLLER_H
#define MU_APPSHELL_FILEMENUCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "userscores/ifilescorecontroller.h"
#include "notation/inotationactionscontroller.h"

#include "uicomponents/imenucontroller.h"

namespace mu::appshell {
class FileMenuController : public uicomponents::IMenuController, public async::Asyncable
{
    INJECT(appshell, userscores::IFileScoreController, fileController)
    INJECT(appshell, notation::INotationActionsController, notationController)

public:
    void init();

    bool contains(const actions::ActionCode& actionCode) const override;
    uicomponents::ActionState actionState(const actions::ActionCode& actionCode) const override;

    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    actions::ActionCodeList fileControllerActions() const;
    actions::ActionCodeList notationControllerActions() const;

    bool fileControllerContains(const actions::ActionCode& actionCode) const;
    bool notationControllerContains(const actions::ActionCode& actionCode) const;

    bool actionEnabled(const actions::ActionCode& actionCode) const;

    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_APPSHELL_FILEMENUCONTROLLER_H
