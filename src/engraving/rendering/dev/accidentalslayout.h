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
struct ShapeElement;
}

namespace mu::engraving::rendering::dev {
class LayoutContext;

class AccidentalsLayout
{
public:
    static void layoutAccidentals(const std::vector<Chord*>& chords, LayoutContext& ctx);

private:
    static std::vector<Accidental*> collectAccidentals(const std::vector<Chord*> chords);
    static std::vector<std::vector<Accidental*> > AccidentalsLayout::splitVertically(std::vector<Accidental*> allAccidentals);
    static Shape createChordsShape(const std::vector<Chord*> chords);

    static void layoutAccidentalGroup(std::vector<Accidental*>& group, const Shape& chordsShape, LayoutContext& ctx);
    static void doLayoutAccidental(Accidental* acc, const Shape& chordsShape, Shape& accidGroupShape);
    static std::vector<std::vector<Accidental*>> splitHorizontally(std::vector<Accidental*>& accidentals, const Shape& chordsShape);

    static void determineStackingOrder(std::vector<Accidental*>& accidentals);
    static void computeStandardOrdering(std::vector<Accidental*>& accidentals);
    static void computeCompactOrdering(std::vector<Accidental*>& accidentals);

    static double minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const Shape& chordsShape);
    static double kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape, const ShapeElement& chordElement);
    static double computePadding(Accidental* acc, const EngravingItem* chordElement);

    static double minAccidentalToAccidentalGroupDistance(const Accidental* acc, Shape accShape, const Shape& accidentalsShape);
    static bool isExceptionOfFourth(const Accidental* acc1, const Accidental* acc2);
    static bool isExceptionOfNaturalsSixth(const Accidental* acc1, const Accidental* acc2);
    static double kerningOfFourth(const Shape& accShape, const ShapeElement& accGroupShapeElement);

    static void verticallyAlignAccidentalsWhereNeeded(std::vector<Accidental*> allAccidentals,
                                                      std::vector<std::vector<Accidental*>> accidentalGroups);

    static void setXposRelativeToSegment(Accidental* accidental, double x);
    static double xPosRelativeToSegment(const Accidental* accidental);
    static double verticalClearance(const Accidental* acc1, const Accidental* acc2);

private:
    static void computeCommonConstants(const Accidental* accidental);

    inline static double m_spatium = 0.0;
    inline static double m_verticalAccidentalToAccidentalClearance = 0.0;
    inline static double m_verticalSharpToSharpClearance = 0.0;
    inline static double m_verticalAccidentalToChordClearance = 0.0;
    inline static double m_accidentalAccidentalDistance = 0.0;
    inline static std::map<const Accidental*, int> m_verticalSubgroup;
    inline static std::map<const Accidental*, int> m_column;
};
}

#endif // MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H
