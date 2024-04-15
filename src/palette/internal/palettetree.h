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

#ifndef MU_PALETTE_PALETTETREE_H
#define MU_PALETTE_PALETTETREE_H

#include "palette.h"

namespace mu::engraving {
class XmlWriter;
class XMLReader;
}

namespace mu::palette {
struct PaletteTree
{
    std::vector<PalettePtr> palettes;

    void insert(size_t idx, PalettePtr palette);
    void append(PalettePtr palette);

    bool read(mu::engraving::XmlReader&, bool pasteMode);
    void write(mu::engraving::XmlWriter&, bool pasteMode) const;

    void retranslate();
};

using PaletteTreePtr = std::shared_ptr<PaletteTree>;
}

#endif // MU_PALETTE_PALETTETREE_H
