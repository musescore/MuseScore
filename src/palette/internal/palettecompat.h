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

#ifndef MU_PALETTE_PALETTECOMPAT_H
#define MU_PALETTE_PALETTECOMPAT_H

#include "engraving/dom/engravingitem.h"

namespace mu::palette {
class Palette;
class PaletteCompat
{
public:
    static void migrateOldPaletteItemIfNeeded(engraving::ElementPtr& element, engraving::Score* paletteScore);
    static void addNewItemsIfNeeded(Palette& palette, engraving::Score* paletteScore);
    static void removeOldItemsIfNeeded(Palette& palette);

private:
    static void addNewGuitarItems(Palette& guitarPalette, engraving::Score* paletteScore);
    static void addNewLineItems(Palette& linesPalette);
    static void addNewFretboardDiagramItems(Palette& fretboardDiagramPalette, engraving::Score* paletteScore);
    static void addNewRepeatItems(Palette& repeatPalette, engraving::Score* paletteScore);
    static void removeOldItems(Palette& palette);
};
} // namespace mu::palette
#endif // MU_PALETTE_PALETTECOMPAT_H
