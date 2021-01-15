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
#ifndef MU_NOTATION_NOTATIONCONTEXTMENU_H
#define MU_NOTATION_NOTATIONCONTEXTMENU_H

#include "modularity/ioc.h"

#include "shortcuts/ishortcutsregister.h"
#include "actions/iactionsregister.h"

#include "inotationcontextmenu.h"

namespace mu::notation {
class NotationContextMenu : public INotationContextMenu
{
    INJECT(notation, shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(notation, actions::IActionsRegister, actionsRegister)

public:
    uicomponents::MenuItemList items(const ElementType& elementType) const override;

private:
    uicomponents::MenuItem makeItem(const actions::ActionItem& action, const std::string& section = "", bool enabled = true,
                                    bool checked = false) const;

    uicomponents::MenuItemList measureItems() const;
    uicomponents::MenuItemList pageItems() const;
    uicomponents::MenuItemList elementItems() const;

    uicomponents::MenuItemList defaultCopyPasteItems() const;
};
}

#endif // MU_NOTATION_NOTATIONCONTEXTMENU_H
