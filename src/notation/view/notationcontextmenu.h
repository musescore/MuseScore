/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_NOTATION_NOTATIONCONTEXTMENU_H
#define MU_NOTATION_NOTATIONCONTEXTMENU_H

#include "ui/view/abstractmenumodel.h"
#include "inotationcontextmenu.h"

namespace mu::notation {
class NotationContextMenu : public ui::AbstractMenuModel, public INotationContextMenu
{
    Q_OBJECT

public:
    ui::MenuItemList items(const ElementType& elementType) const override;

private:
    ui::MenuItemList pageItems() const;
    ui::MenuItemList defaultCopyPasteItems() const;
    ui::MenuItemList measureItems() const;
    ui::MenuItemList staffTextItems() const;
    ui::MenuItemList systemTextItems() const;
    ui::MenuItemList timeSignatureItems() const;
    ui::MenuItemList selectItems() const;
    ui::MenuItemList elementItems() const;
};
}

#endif // MU_NOTATION_NOTATIONCONTEXTMENU_H
