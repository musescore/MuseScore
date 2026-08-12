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

#include "paletteactionscontroller.h"

#include "rcommand/actiontocommand.h"

#include "notation/inotation.h"
#include "notation/inotationinteraction.h"
#include "../palettecommands.h"
#include <vector>

using namespace mu::palette;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::rcommand;

static const muse::UriQuery MASTER_PALETTE_URI("musescore://palette/masterpalette?modal=false");
static const muse::UriQuery SPECIAL_CHARACTERS_URI("musescore://palette/specialcharacters?modal=false");
static const muse::UriQuery TIME_SIGNATURE_PROPERTIES_URI("musescore://palette/timesignatureproperties");
static const muse::UriQuery CUSTOMIZE_KIT_URI("musescore://palette/customizekit");

void PaletteActionsController::init()
{
    auto cd = commandDispatcher();
    cd->onRequest(this, TOGGLE_MASTER_PALETTE_COMMAND, [this](const CommandQuery& query) { return toggleMasterPalette(query); });
    cd->onRequest(this, TOGGLE_SPECIAL_CHARACTERS_COMMAND, [this]() { toggleSpecialCharactersDialog(); return muse::make_ok(); });
    cd->onRequest(this, OPEN_TIME_SIGNATURE_PROPERTIES_COMMAND, [this]() { openTimeSignaturePropertiesDialog(); return muse::make_ok(); });
    cd->onRequest(this, OPEN_CUSTOMIZE_KIT_COMMAND, [this]() { openCustomizeKitDialog(); return muse::make_ok(); });

    cd->onRequest(this, PALETTE_TOGGLE_SINGLE_CLICK_TO_OPEN_COMMAND, [this]() { return toggleSingleClickToOpen(); });
    cd->onRequest(this, PALETTE_TOGGLE_SINGLE_PALETTE_COMMAND, [this]() { return toggleSinglePalette(); });
    cd->onRequest(this, PALETTE_TOGGLE_DRAG_ENABLED_COMMAND, [this]() { return toggleDragEnabled(); });

    cd->onRequest(this, PALETTE_SEARCH_COMMAND, [this]() { m_paletteSearchRequested.notify(); return muse::make_ok(); });
    cd->onRequest(this, PALETTE_APPLY_CURRENT_ELEMENT_COMMAND, [this]() { m_applyElementRequested.notify(); return muse::make_ok(); });
    cd->onRequest(this, PALETTE_EXPAND_ALL_COMMAND, [this]() { m_expandCollapseAllRequested.send(true); return muse::make_ok(); });
    cd->onRequest(this, PALETTE_COLLAPSE_ALL_COMMAND, [this]() { m_expandCollapseAllRequested.send(false); return muse::make_ok(); });

    // compat
    {
        static const std::vector<ActionToCommand> actionToCommands = {
            { "masterpalette", TOGGLE_MASTER_PALETTE_COMMAND, make_conv({ { "palette_name", param<std::string>  } }) },
            { "show-keys", TOGGLE_SPECIAL_CHARACTERS_COMMAND, {} },
            { "time-signature-properties", OPEN_TIME_SIGNATURE_PROPERTIES_COMMAND, {} },
            { "customize-kit", OPEN_CUSTOMIZE_KIT_COMMAND, {} },
            { "palette-search", PALETTE_SEARCH_COMMAND, {} },
            { "apply-current-palette-element", PALETTE_APPLY_CURRENT_ELEMENT_COMMAND, {} },
            { "toggle-single-click-to-open-palette", PALETTE_TOGGLE_SINGLE_CLICK_TO_OPEN_COMMAND, {} },
            { "toggle-single-palette", PALETTE_TOGGLE_SINGLE_PALETTE_COMMAND, {} },
            { "toggle-palette-drag", PALETTE_TOGGLE_DRAG_ENABLED_COMMAND, {} },
            { "expand-all-palettes", PALETTE_EXPAND_ALL_COMMAND, {} },
            { "collapse-all-palettes", PALETTE_COLLAPSE_ALL_COMMAND, {} },
        };

        registerActionToCommand(this, actionToCommands, cd, dispatcher());
    }

    interactive()->currentUri().ch.onReceive(this, [this](const Uri& uri) {
        //! NOTE If MasterPalette are not open, then it is reasonably to compare with the current uri,
        //! so as not to call the more expensive `interactive()->isOpened` method.
        //! If MasterPalette is open, then we will call `interactive()->isOpened`,
        //! in case if they suddenly did not close MasterPalette,
        //! but opened something else on top of MasterPalette.
        bool isOpened = false;
        if (!m_masterPaletteOpened.val) {
            isOpened = uri == MASTER_PALETTE_URI.uri();
        } else {
            isOpened = interactive()->isOpened(MASTER_PALETTE_URI.uri()).val;
        }

        if (isOpened != m_masterPaletteOpened.val) {
            m_masterPaletteOpened.set(isOpened);
        }
    });
}

ValCh<bool> PaletteActionsController::isMasterPaletteOpened() const
{
    return m_masterPaletteOpened;
}

Ret PaletteActionsController::toggleMasterPalette(const CommandQuery& query)
{
    if (interactive()->isOpened(MASTER_PALETTE_URI.uri()).val) {
        interactive()->close(MASTER_PALETTE_URI.uri());
    } else {
        if (query.contains("palette_name")) {
            std::string paletteName = query.param("palette_name").toString();
            interactive()->open(MASTER_PALETTE_URI.addingParam("selectedPaletteName", Val(paletteName)));
        } else {
            interactive()->open(MASTER_PALETTE_URI);
        }
    }
    return muse::make_ok();
}

void PaletteActionsController::toggleSpecialCharactersDialog()
{
    if (interactive()->isOpened(SPECIAL_CHARACTERS_URI.uri()).val) {
        interactive()->close(SPECIAL_CHARACTERS_URI.uri());
    } else {
        auto notation = globalContext()->currentNotation();
        if (notation && notation->interaction()->isTextEditingStarted()) {
            interactive()->open(SPECIAL_CHARACTERS_URI);
        }
    }
}

void PaletteActionsController::openTimeSignaturePropertiesDialog()
{
    const engraving::EngravingItem* element = notationInteraction() ? notationInteraction()->hitElementContext().element : nullptr;
    if (!element || !element->isTimeSig()) {
        return;
    }
    interactive()->open(TIME_SIGNATURE_PROPERTIES_URI);
}

void PaletteActionsController::openCustomizeKitDialog()
{
    interactive()->open(CUSTOMIZE_KIT_URI);
}

Ret PaletteActionsController::toggleSingleClickToOpen()
{
    ValCh<bool> checked = configuration()->isSingleClickToOpenPalette();
    configuration()->setIsSingleClickToOpenPalette(!checked.val);
    return muse::make_ok();
}

muse::Ret PaletteActionsController::toggleSinglePalette()
{
    ValCh<bool> checked = configuration()->isSinglePalette();
    configuration()->setIsSinglePalette(!checked.val);
    return muse::make_ok();
}

muse::Ret PaletteActionsController::toggleDragEnabled()
{
    ValCh<bool> checked = configuration()->isPaletteDragEnabled();
    configuration()->setIsPaletteDragEnabled(!checked.val);
    return muse::make_ok();
}

mu::notation::INotationInteractionPtr PaletteActionsController::notationInteraction() const
{
    const notation::INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

async::Notification PaletteActionsController::paletteSearchRequested() const
{
    return m_paletteSearchRequested;
}

async::Notification PaletteActionsController::applyCurrentPaletteElementRequested() const
{
    return m_applyElementRequested;
}

async::Channel<bool> PaletteActionsController::expandCollapseAllRequested() const
{
    return m_expandCollapseAllRequested;
}
