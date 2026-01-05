/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#pragma once

#include "ui/iuiactionsregister.h"
#include "shortcuts/ishortcutsregister.h"

#include "iinteractive.h"

#include "engraving/rendering/isinglerenderer.h"
#include "engraving/rw/xmlreader.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"

#include "types/retval.h"

namespace mu::notation {
class PercussionUtilities : public muse::Injectable
{
    muse::Inject<muse::ui::IUiActionsRegister> uiactionsRegister = { this };
    muse::Inject<muse::shortcuts::IShortcutsRegister> shortcutsRegister = { this };

    muse::Inject<muse::IInteractive> interactive = { this };

    muse::Inject<mu::engraving::rendering::ISingleRenderer> engravingRender = { this };

public:

    PercussionUtilities(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx)
    {
    }

    void readDrumset(const muse::ByteArray& drumMapping, mu::engraving::Drumset& drumset);
    std::shared_ptr<mu::engraving::Chord> getDrumNoteForPreview(const mu::engraving::Drumset* drumset, int pitch);
    bool editPercussionShortcut(mu::engraving::Drumset& drumset, int originPitch);

private:
    muse::RetVal<muse::Val> openPercussionShortcutDialog(const mu::engraving::Drumset& drumset, int originPitch);
    QVariantMap drumToQVariantMap(int pitch, const engraving::DrumInstrument& drum);
};
}
