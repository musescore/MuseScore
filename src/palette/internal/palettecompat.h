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

#ifndef MU_PALETTE_PALETTECOMPAT_H
#define MU_PALETTE_PALETTECOMPAT_H

#include "libmscore/engravingitem.h"

namespace mu::palette {
class Palette;
class PaletteCompat
{
public:
    static void migrateOldPaletteItemIfNeeded(engraving::ElementPtr& element, engraving::Score* paletteScore);
    static void addNewItemsIfNeeded(Palette& palette, engraving::Score* paletteScore);

private:
    static void addNewGuitarItems(Palette& guitarPalette, engraving::Score* paletteScore);
};
} // namespace mu::palette
#endif // MU_PALETTE_PALETTECOMPAT_H
