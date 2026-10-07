/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "notationcontextmenumodel.h"

#include "types/translatablestring.h"

#include "ui/view/iconcodes.h"

#include "engraving/dom/fret.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationselection.h"

#include "notationscene/notationcommands.h"
#include "notationscene/notationmenus.h"
#include "palette/palettecommands.h"
#include "instrumentsscene/instrumentscommands.h"

#include "widgets/editstyleutils.h"

using namespace mu::notation;
using namespace mu::palette;
using namespace mu::instrumentsscene;
using namespace muse;
using namespace muse::uicomponents;
using namespace muse::actions;

void NotationContextMenuModel::loadItems(int elementType)
{
    AbstractMenuModel::load();

    MenuItemList items = makeItemsByElementType(static_cast<ElementType>(elementType));

    const INotationAutomationPtr automation = this->automation();
    if (automation && automation->isAutomationModeEnabled()) {
        items << makeSeparator()
              << makeMenu(TranslatableString::untranslatable("Automation type"), makeAutomationTypeItems());
    }

    setItems(items);
}

MenuItemList NotationContextMenuModel::makeItemsByElementType(ElementType elementType)
{
    switch (elementType) {
    case ElementType::MEASURE:
        return makeMeasureItems();
    case ElementType::PAGE:
        return makePageItems();
    case ElementType::STAFF_TEXT:
        return makeStaffTextItems();
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
        return makeSystemTextItems();
    case ElementType::TIMESIG:
        return makeTimeSignatureItems();
    case ElementType::INSTRUMENT_NAME:
        return makeInstrumentNameItems();
    case ElementType::HARMONY:
        return makeHarmonyItems();
    case ElementType::FRET_DIAGRAM:
        return makeFretboardDiagramItems();
    case ElementType::INSTRUMENT_CHANGE:
        return makeChangeInstrumentItems();
    case ElementType::VBOX:
        return makeVerticalBoxItems();
    case ElementType::HBOX:
        return makeHorizontalBoxItems();
    case ElementType::HAIRPIN_SEGMENT:
        return makeHairpinItems();
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
        return makeGradualTempoChangeItems();
    case ElementType::TEXT:
        return makeTextItems();
    default:
        break;
    }

    return makeElementItems();
}

MenuItemList NotationContextMenuModel::makePageItems()
{
    MenuItemList items {
        makeMenuItem(OPEN_EDIT_STYLE_COMMAND),
        makeMenuItem(OPEN_PAGE_SETTINGS_COMMAND),
        makeMenuItem(LOAD_STYLE_COMMAND),
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeDefaultCopyPasteItems()
{
    MenuItemList items {
        makeMenuItem(CUT_COMMAND),
        makeMenuItem(COPY_COMMAND),
        makeMenuItem(PASTE_COMMAND),
        makeMenuItem(COPY_PASTE_SWAP_COMMAND),
        makeMenuItem(DELETE_COMMAND),
    };

    return items;
}

MenuItemList NotationContextMenuModel::makeMeasureItems()
{
    MenuItemList items = {
        makeMenuItem(CUT_COMMAND),
        makeMenuItem(COPY_COMMAND),
        makeMenuItem(PASTE_COMMAND),
        makeMenuItem(COPY_PASTE_SWAP_COMMAND),
    };

    items << makeSeparator();

    items << makeMenu(TranslatableString("notation", "Insert measures"), makeInsertMeasuresItems());

    MenuItem* clearItem = makeMenuItem(DELETE_COMMAND);
    clearItem->setTitle(TranslatableString("notation", "Clear measures"));
    clearItem->setIcon(ui::IconCode::Code::ERASER);
    MenuItem* deleteItem = makeMenuItem(REMOVE_SELECTED_RANGE_COMMAND);
    deleteItem->setTitle(TranslatableString("notation", "Delete measures"));
    items << clearItem;
    items << deleteItem;

    items << makeSeparator();

    if (globalContext()->currentNotation()->viewMode() == mu::notation::ViewMode::PAGE) {
        items << makeMenu(TranslatableString("notation", "System & page layout"), makeSystemAndPageLayoutItems());
        items << makeSeparator();
    }

    items << makeMenuItem(OPEN_MEASURE_PROPERTIES_COMMAND);
    items << makeMenuItem(OPEN_STAFF_PROPERTIES_COMMAND);
    if (isDrumsetStaff()) {
        items << makeMenuItem(OPEN_CUSTOMIZE_KIT_COMMAND);
    }

    return items;
}

MenuItemList NotationContextMenuModel::makeStaffTextItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem(OPEN_STAFF_TEXT_PROPERTIES_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeSystemTextItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem(OPEN_SYSTEM_TEXT_PROPERTIES_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeTimeSignatureItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem(OPEN_TIME_SIGNATURE_PROPERTIES_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeInstrumentNameItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem(OPEN_STAFF_PROPERTIES_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeHarmonyItems()
{
    const EngravingItem* element = currentElement();
    if (element && engraving::toHarmony(element)->isInFretBox()) {
        return makeElementInFretBoxItems();
    }

    MenuItemList items = makeElementItems();
    items << makeSeparator();

    if (element) {
        engraving::EngravingObject* parent = element->isHarmony() ? element->ownershipParent() : nullptr;
        bool hasLinkedFretboardDiagram = parent && parent->isFretDiagram();
        if (!hasLinkedFretboardDiagram) {
            items << makeMenuItem(ADD_FRETBOARD_DIAGRAM_COMMAND);
        }
    }

    items << makeMenuItem(OPEN_REALIZECHORDSYMBOLS_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeFretboardDiagramItems()
{
    const EngravingItem* element = currentElement();
    if (element && engraving::toFretDiagram(element)->isInFretBox()) {
        return makeElementInFretBoxItems();
    }

    MenuItemList items = makeElementItems();

    const engraving::FretDiagram* fretDiagram = engraving::toFretDiagram(element);
    if (!fretDiagram->harmony()) {
        items << makeSeparator();
        items << makeMenuItem(ADD_CHORD_TEXT_COMMAND, TranslatableString("notation", "Add c&hord symbol"));
    }

    return items;
}

MenuItemList NotationContextMenuModel::makeElementInFretBoxItems()
{
    MenuItemList items {
        makeMenuItem(COPY_COMMAND)
    };

    MenuItem* hideItem = makeMenuItem(DELETE_COMMAND);
    hideItem->setTitle(TranslatableString("notation", "Hide"));
    hideItem->setIcon(ui::IconCode::Code::NONE);

    items << hideItem
          << makeSeparator();

    MenuItemList selectItems = makeSelectItems();

    if (!selectItems.isEmpty()) {
        items << makeMenu(TranslatableString("notation", "Select"), selectItems)
              << makeSeparator();
    }

    const EngravingItem* element = currentElement();
    items << makeEditStyle(element);

    return items;
}

MenuItemList NotationContextMenuModel::makeSelectItems()
{
    if (isSingleSelection()) {
        return MenuItemList { makeMenuItem(SELECT_SIMILAR_COMMAND),
                              makeMenuItem(SELECT_SIMILAR_IN_STAFF_COMMAND),
                              makeMenuItem(OPEN_SELECTION_OPTIONS_COMMAND) };
    } else if (canSelectSimilarInRange()) {
        return MenuItemList { makeMenuItem(SELECT_SIMILAR_IN_RANGE_COMMAND), makeMenuItem(OPEN_SELECTION_OPTIONS_COMMAND) };
    } else if (canSelectSimilar()) {
        return MenuItemList{ makeMenuItem(OPEN_SELECTION_OPTIONS_COMMAND) };
    }

    return MenuItemList();
}

MenuItemList NotationContextMenuModel::makeElementItems()
{
    MenuItemList items = makeDefaultCopyPasteItems();

    if (interaction()->isTextEditingStarted()) {
        return items;
    }

    MenuItemList selectItems = makeSelectItems();

    if (!selectItems.isEmpty()) {
        items << makeMenu(TranslatableString("notation", "Select"), selectItems);
    }

    const EngravingItem* element = currentElement();

    if (element && element->isEditable()) {
        items << makeSeparator();
        items << makeMenuItem(SCREEN_EDIT_ELEMENT_COMMAND);
    }

    items << makeSeparator()
          << makeEditStyle(element);

    return items;
}

MenuItemList NotationContextMenuModel::makeInsertMeasuresItems()
{
    return makeItems(MEASURES_MENU_COMMANDS);
}

MenuItemList NotationContextMenuModel::makeSystemAndPageLayoutItems()
{
    MenuItemList items;

    INotationSelectionPtr sel = selection();
    if (!sel) {
        return items;
    }

    std::vector<engraving::System*> systems = sel->selectedSystems();
    std::vector<engraving::Page*> pages = sel->pagesContainingSelection();
    int nSystems = static_cast<int>(systems.size());
    int nMeasures = static_cast<int>(sel->selectedMeasuresCount());
    int nPages = static_cast<int>(pages.size());
    bool systemslocked = allSystemsAreLocked(systems);
    bool pageslocked = allPagesAreLocked(pages);

    items << makeSystemLayoutItems(systemslocked, nSystems, nMeasures);
    items << makeSeparator();
    items << makePageLayoutItems(pageslocked, nPages, nSystems);
    items << makeSeparator();
    items << makeMenuItem(OPEN_BREAKS_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeSystemLayoutItems(bool locked, int nSystems, int nMeasures)
{
    TranslatableString lockLabel = locked ? TranslatableString("notation", "Unlock selected system(s)", nullptr, nSystems)
                                   : TranslatableString("notation", "Lock selected system(s)", nullptr, nSystems);
    MenuItem* systemLockMenuItem = makeMenuItem(TOGGLE_SYSTEM_LOCK_COMMAND, lockLabel);
    systemLockMenuItem->setIcon(locked ? ui::IconCode::Code::LOCK_CLOSED : ui::IconCode::Code::LOCK_OPEN);

    return {
        systemLockMenuItem,
        makeMenuItem(MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND,
                     TranslatableString("notation", "Move measure(s) to previous system", nullptr, nMeasures)),
        makeMenuItem(MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND,
                     TranslatableString("notation", "Move measure(s) to next system", nullptr, nMeasures)),
        makeMenuItem(MAKE_INTO_SYSTEM_COMMAND)
    };
}

MenuItemList NotationContextMenuModel::makePageLayoutItems(bool locked, int nPages, int nSystems)
{
    TranslatableString lockLabel = locked ? TranslatableString("notation", "Unlock selected page(s)", nullptr, nPages)
                                   : TranslatableString("notation", "Lock selected page(s)", nullptr, nPages);
    MenuItem* pageLockMenuItem = makeMenuItem(TOGGLE_PAGE_LOCK_COMMAND, lockLabel);
    pageLockMenuItem->setIcon(locked ? ui::IconCode::Code::LOCK_CLOSED : ui::IconCode::Code::LOCK_OPEN);

    return {
        pageLockMenuItem,
        makeMenuItem(MOVE_SYSTEM_TO_PREV_PAGE_COMMAND,
                     TranslatableString("notation", "Move system(s) to previous page", nullptr, nSystems)),
        makeMenuItem(MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND,
                     TranslatableString("notation", "Move system(s) to next page", nullptr, nSystems)),
        makeMenuItem(MAKE_INTO_PAGE_COMMAND)
    };
}

MenuItemList NotationContextMenuModel::makeChangeInstrumentItems()
{
    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenuItem(INSTRUMENTS_CHANGE_COMMAND);

    return items;
}

MenuItemList NotationContextMenuModel::makeVerticalBoxItems()
{
    MenuItemList addMenuItems;
    addMenuItems << makeMenuItem(ADD_FRAME_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_TITLE_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_SUBTITLE_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_COMPOSER_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_LYRICIST_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_PART_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_IMAGE_COMMAND);

    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenu(TranslatableString("notation", "Add"), addMenuItems);

    return items;
}

MenuItemList NotationContextMenuModel::makeHorizontalBoxItems()
{
    MenuItemList addMenuItems;
    addMenuItems << makeMenuItem(ADD_FRAME_TEXT_COMMAND);
    addMenuItems << makeMenuItem(ADD_IMAGE_COMMAND);

    MenuItemList items = makeElementItems();
    items << makeSeparator();
    items << makeMenu(TranslatableString("notation", "Add"), addMenuItems);

    return items;
}

MenuItemList NotationContextMenuModel::makeHairpinItems()
{
    MenuItemList items = makeElementItems();

    const EngravingItem* element = currentElement();
    if (!element || !element->isHairpinSegment() || !isSingleSelection()) {
        return items;
    }

    items << makeSeparator();

    const engraving::Hairpin* h = toHairpinSegment(element)->hairpin();

    MenuItem* snapPrev = makeMenuItem(TOGGLE_SNAP_TO_PREV_COMMAND);
    snapPrev->setChecked(h->snapToItemBefore());
    items << snapPrev;

    MenuItem* snapNext = makeMenuItem(TOGGLE_SNAP_TO_NEXT_COMMAND);
    snapNext->setChecked(h->snapToItemAfter());
    items << snapNext;

    return items;
}

MenuItemList NotationContextMenuModel::makeGradualTempoChangeItems()
{
    MenuItemList items = makeElementItems();

    const EngravingItem* element = currentElement();
    if (!element || !element->isGradualTempoChangeSegment() || !isSingleSelection()) {
        return items;
    }

    items << makeSeparator();

    const engraving::GradualTempoChange* gtc = toGradualTempoChangeSegment(element)->tempoChange();

    MenuItem* snapNext = makeMenuItem(TOGGLE_SNAP_TO_NEXT_COMMAND);
    snapNext->setChecked(gtc->snapToItemAfter());
    items << snapNext;

    return items;
}

MenuItemList NotationContextMenuModel::makeTextItems()
{
    const EngravingItem* element = currentElement();
    if (!(element->ownershipParent() && element->ownershipParent()->isBarLine())) {
        // Regular text
        return makeElementItems();
    }

    // Play count text
    MenuItemList items;

    if (interaction()->isTextEditingStarted()) {
        return items;
    }

    MenuItemList selectItems = makeSelectItems();

    if (!selectItems.isEmpty()) {
        items << makeMenu(TranslatableString("notation", "Select"), selectItems);
    }

    items << makeSeparator()
          << makeEditStyle(element);

    return items;
}

MenuItem* NotationContextMenuModel::makeEditStyle(const EngravingItem* element)
{
    MenuItem* item = makeMenuItem(OPEN_EDIT_STYLE_COMMAND);

    if (element) {
        std::string pageCode = EditStyleUtils::pageCodeForElement(element).toStdString();
        if (!pageCode.empty()) {
            rcommand::CommandQuery query(OPEN_EDIT_STYLE_COMMAND);
            query.addParam("page_code", Val(pageCode));

            std::string subPageCode = EditStyleUtils::subPageCodeForElement(element).toStdString();
            if (!subPageCode.empty()) {
                query.addParam("sub_page_code", Val(subPageCode));
            }

            item->setCommandQuery(query);
        }
    }

    return item;
}

bool NotationContextMenuModel::isSingleSelection() const
{
    INotationSelectionPtr selection = this->selection();
    return selection ? selection->element() != nullptr : false;
}

bool NotationContextMenuModel::canSelectSimilar() const
{
    return currentElement() != nullptr;
}

bool NotationContextMenuModel::canSelectSimilarInRange() const
{
    return canSelectSimilar() && selection()->isRange();
}

bool NotationContextMenuModel::isDrumsetStaff() const
{
    const INotationInteraction::HitElementContext& ctx = hitElementContext();
    if (!ctx.staff) {
        return false;
    }

    Fraction tick = ctx.element ? ctx.element->tick() : Fraction { -1, 1 };
    return ctx.staff->part()->instrument(tick)->drumset() != nullptr;
}

bool NotationContextMenuModel::allSystemsAreLocked(const std::vector<engraving::System*>& systems) const
{
    if (systems.empty()) {
        return false;
    }

    for (System* system : systems) {
        if (!system->isLocked()) {
            return false;
        }
    }
    return true;
}

bool NotationContextMenuModel::allPagesAreLocked(const std::vector<engraving::Page*>& pages) const
{
    if (pages.empty()) {
        return false;
    }

    for (Page* page : pages) {
        if (!page->isLocked()) {
            return false;
        }
    }
    return true;
}

MenuItemList NotationContextMenuModel::makeAutomationTypeItems()
{
    return {
        makeAutomationTypeItem(AutomationType::Dynamics, "dynamics", TranslatableString::untranslatable("Dynamics")),
        makeAutomationTypeItem(AutomationType::Tempo, "tempo", TranslatableString::untranslatable("Tempo")),
        makeAutomationTypeItem(AutomationType::Volume, "volume", TranslatableString::untranslatable("Volume")),
        makeAutomationTypeItem(AutomationType::Pan, "pan", TranslatableString::untranslatable("Pan")),
    };
}

MenuItem* NotationContextMenuModel::makeAutomationTypeItem(AutomationType type, const std::string& queryTypeParam,
                                                           const TranslatableString& title)
{
    MenuItem* item = makeMenuItem(SELECT_AUTOMATION_TYPE_COMMAND, title);
    if (!item) {
        return item;
    }

    rcommand::CommandQuery query(SELECT_AUTOMATION_TYPE_COMMAND);
    query.addParam("type", Val(queryTypeParam));
    item->setCommandQuery(query);

    item->setChecked(notationConfiguration()->currentAutomationType() == type);

    return item;
}

INotationInteractionPtr NotationContextMenuModel::interaction() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

INotationSelectionPtr NotationContextMenuModel::selection() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction()->selection() : nullptr;
}

INotationAutomationPtr NotationContextMenuModel::automation() const
{
    IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();
    return masterNotation ? masterNotation->automation() : nullptr;
}

const EngravingItem* NotationContextMenuModel::currentElement() const
{
    const EngravingItem* element = hitElementContext().element;
    if (element) {
        return element;
    }

    auto selection = this->selection();
    return selection && selection->element() ? selection->element() : nullptr;
}

const INotationInteraction::HitElementContext& NotationContextMenuModel::hitElementContext() const
{
    if (INotationInteractionPtr interaction = this->interaction()) {
        return interaction->hitElementContext();
    }

    static INotationInteraction::HitElementContext dummy;
    return dummy;
}
