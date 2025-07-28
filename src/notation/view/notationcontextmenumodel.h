/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
class NotationContextMenuModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    Q_INVOKABLE void loadItems(int elementType);

private:
    muse::uicomponents::MenuItemList makeItemsByElementType(ElementType type);

    muse::uicomponents::MenuItemList makePageItems();
    muse::uicomponents::MenuItemList makeDefaultCopyPasteItems();
    muse::uicomponents::MenuItemList makeMeasureItems();
    muse::uicomponents::MenuItemList makeStaffTextItems();
    muse::uicomponents::MenuItemList makeSystemTextItems();
    muse::uicomponents::MenuItemList makeTimeSignatureItems();
    muse::uicomponents::MenuItemList makeInstrumentNameItems();
    muse::uicomponents::MenuItemList makeHarmonyItems();
    muse::uicomponents::MenuItemList makeFretboardDiagramItems();
    muse::uicomponents::MenuItemList makeElementInFretBoxItems();
    muse::uicomponents::MenuItemList makeSelectItems();
    muse::uicomponents::MenuItemList makeElementItems();
    muse::uicomponents::MenuItemList makeInsertMeasuresItems();
    muse::uicomponents::MenuItemList makeMoveMeasureItems();
    muse::uicomponents::MenuItemList makeChangeInstrumentItems();
    muse::uicomponents::MenuItemList makeVerticalBoxItems();
    muse::uicomponents::MenuItemList makeHorizontalBoxItems();
    muse::uicomponents::MenuItemList makeHairpinItems();
    muse::uicomponents::MenuItemList makeGradualTempoChangeItems();
    muse::uicomponents::MenuItemList makeTextItems();

    muse::uicomponents::MenuItem* makeEditStyle(const EngravingItem* element);

    bool isSingleSelection() const;
    bool canSelectSimilarInRange() const;
    bool canSelectSimilar() const;
    bool isDrumsetStaff() const;

    INotationInteractionPtr interaction() const;
    INotationSelectionPtr selection() const;

    const EngravingItem* currentElement() const;

    const INotationInteraction::HitElementContext& hitElementContext() const;
};
}

#endif // MU_NOTATION_NOTATIONCONTEXTMENUMODEL_H
