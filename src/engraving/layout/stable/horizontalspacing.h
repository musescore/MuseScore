/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_HORIZONTALSPACINGUTILS_STABLE_H
#define MU_ENGRAVING_HORIZONTALSPACINGUTILS_STABLE_H

namespace mu::engraving {
class Chord;
class EngravingItem;
class Note;
class Rest;
class Shape;
class StemSlash;
enum class ElementType;
enum class KerningType;
}

namespace mu::engraving::layout::stable {
class HorizontalSpacing
{
public:
    static double computePadding(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeKerning(const EngravingItem* item1, const EngravingItem* item2);

private:
    static bool isSpecialNotePaddingType(ElementType type);
    static void computeNotePadding(const Note* note, const EngravingItem* item2, double& padding, double scaling);
    static void computeLedgerRestPadding(const Rest* rest2, double& padding);

    static bool isSameVoiceKerningLimited(const EngravingItem* item);
    static bool isNeverKernable(const EngravingItem* item);
    static bool isAlwaysKernable(const EngravingItem* item);

    static KerningType doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeNoteKerningType(const Note* note, const EngravingItem* item2);
    static KerningType computeStemSlashKerningType(const StemSlash* stemSlash, const EngravingItem* item2);
};
} // namespace mu::engraving::layout
#endif // MU_ENGRAVING_HORIZONTALSPACINGUTILS_STABLE_H
