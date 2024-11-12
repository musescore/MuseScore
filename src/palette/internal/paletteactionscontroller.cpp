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

#include "paletteactionscontroller.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

static const muse::UriQuery MASTER_PALETTE_URI("musescore://palette/masterpalette?sync=false&modal=false");
static const muse::UriQuery SPECIAL_CHARACTERS_URI("musescore://palette/specialcharacters?sync=false");
static const muse::UriQuery TIME_SIGNATURE_PROPERTIES_URI("musescore://palette/timesignatureproperties");
static const muse::UriQuery CUSTOMIZE_KIT_URI("musescore://palette/customizekit");

void PaletteActionsController::init()
{
    dispatcher()->reg(this, "masterpalette", this, &PaletteActionsController::toggleMasterPalette);
    dispatcher()->reg(this, "show-keys", this, &PaletteActionsController::toggleSpecialCharactersDialog);
    dispatcher()->reg(this, "time-signature-properties", this, &PaletteActionsController::openTimeSignaturePropertiesDialog);
    dispatcher()->reg(this, "customize-kit", this, &PaletteActionsController::openCustomizeKitDialog);

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

void PaletteActionsController::toggleMasterPalette(const ActionData& args)
{
    if (interactive()->isOpened(MASTER_PALETTE_URI.uri()).val) {
        interactive()->close(MASTER_PALETTE_URI.uri());
    } else {
        if (args.empty()) {
            interactive()->open(MASTER_PALETTE_URI);
        } else {
            std::string selectedPaletteName = args.arg<std::string>(0);
            interactive()->open(MASTER_PALETTE_URI.addingParam("selectedPaletteName", Val(selectedPaletteName)));
        }
    }
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
    interactive()->open(TIME_SIGNATURE_PROPERTIES_URI);
}

void PaletteActionsController::openCustomizeKitDialog()
{
    interactive()->open(CUSTOMIZE_KIT_URI);
}
