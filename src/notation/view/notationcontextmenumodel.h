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
#ifndef MU_NOTATION_NOTATIONCONTEXTMENUMODEL_H
#define MU_NOTATION_NOTATIONCONTEXTMENUMODEL_H

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "uicomponents/view/abstractmenumodel.h"
#include "notation/notationtypes.h"

namespace mu::notation {
class NotationContextMenuModel : public uicomponents::AbstractMenuModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    Q_INVOKABLE void loadItems(int elementType);

private:
    uicomponents::MenuItemList makeItemsByElementType(ElementType type);

    uicomponents::MenuItemList makePageItems();
    uicomponents::MenuItemList makeDefaultCopyPasteItems();
    uicomponents::MenuItemList makeMeasureItems();
    uicomponents::MenuItemList makeStaffTextItems();
    uicomponents::MenuItemList makeSystemTextItems();
    uicomponents::MenuItemList makeTimeSignatureItems();
    uicomponents::MenuItemList makeInstrumentNameItems();
    uicomponents::MenuItemList makeHarmonyItems();
    uicomponents::MenuItemList makeSelectItems();
    uicomponents::MenuItemList makeElementItems();
    uicomponents::MenuItemList makeInsertMeasuresItems();
    uicomponents::MenuItemList makeChangeInstrumentItems();
    uicomponents::MenuItemList makeVerticalBoxItems();
    uicomponents::MenuItemList makeHorizontalBoxItems();
    uicomponents::MenuItemList makeImageItems();

    bool isSingleSelection() const;
    bool canSelectSimilarInRange() const;
    bool canSelectSimilar() const;
    bool isDrumsetStaff() const;

    INotationInteractionPtr interaction() const;
    INotationSelectionPtr selection() const;

    const INotationInteraction::HitElementContext& hitElementContext() const;
};
}

#endif // MU_NOTATION_NOTATIONCONTEXTMENUMODEL_H
