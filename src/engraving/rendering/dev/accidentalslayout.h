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
#ifndef MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H
#define MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H

namespace mu::engraving {
class Accidental;
class Chord;
class EngravingItem;
class LedgerLine;
class Shape;
}

namespace mu::engraving::rendering::dev {
class LayoutContext;

class AccidentalsLayout
{
public:
    static void layoutAccidentals(const std::vector<Chord*>& chords, LayoutContext& ctx);

private:
    static void collectAccidentals(std::vector<Accidental*>& allAccidentals, const std::vector<Chord*> chords);
    static void createGroups(std::vector<std::vector<Accidental*> >& accidentalGroups, std::vector<Accidental*> allAccidentals);
    static Shape createChordsShape(const std::vector<Chord*> chords);

    static void layoutAccidentalGroup(std::vector<Accidental*>& accidentals, const Shape& chordsShape, LayoutContext& ctx);
    static void determineAccidentalsOrder(std::vector<Accidental*>& accidentals);
    static double minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const Shape& chordsShape);
    static double computePadding(Accidental* acc, const EngravingItem* chordElement);
    static bool collidesWithPreviousAccidentals(const Accidental* acc, const Shape& accShape, const Shape& accidentalsShape);
    static double minAccidentalToAccidentalGroupDistance(const Accidental* acc, const Shape& accShape, const Shape& accidentalsShape);

    static void verticallyAlignAccidentalsWhereNeeded(std::vector<Accidental*> allAccidentals,
                                                      std::vector<std::vector<Accidental*>> accidentalGroups);

    static void setXposRelativeToSegment(Accidental* accidental, double x);
    static double xPosRelativeToSegment(const Accidental* accidental);
};
}

#endif // MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H
