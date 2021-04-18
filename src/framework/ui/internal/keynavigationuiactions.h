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
#ifndef MU_UI_KEYNAVIGATIONUIACTIONS_H
#define MU_UI_KEYNAVIGATIONUIACTIONS_H

#include "../iuiactionsmodule.h"

namespace mu::ui {
class KeyNavigationUiActions : public IUiActionsModule
{
public:
    KeyNavigationUiActions() = default;

    const UiActionList& actionsList() const override;
    bool actionEnabled(const UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

private:
    static const UiActionList m_actions;
};
}

#endif // MU_UI_KEYNAVIGATIONUIACTIONS_H
