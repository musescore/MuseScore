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

#include "shape.h"

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

    class AccidentalPlacementEngine
    {
    public:
        AccidentalPlacementEngine(const std::vector<Accidental*>& allAccidentals, const std::vector<Chord*>& chords);
        void doAccidentalPlacement();

    private:
        void findOctaves();
        void findSeconds();

        void splitIntoSubChords();
        void mergeAdjacentSubGroupsIfTooSmall(std::vector<std::vector<Accidental*> >& subGroups);
        void mergeSubGroupsWithOctavesAcross(std::vector<std::vector<Accidental*> >& subGroups);

        void createChordsShape();

        void layoutSubChord(std::vector<Accidental*>& accidentals);
        void stackAccidental(Accidental* acc);

        void computeOrdering(std::vector<Accidental*>& accidentals);
        std::vector<std::vector<Accidental*> > splitIntoPriorityGroups(std::vector<Accidental*>& accidentals);

        std::vector<std::vector<Accidental*> > splitAccordingToAccidDisplacement(std::vector<Accidental*>& accidentals);
        std::vector<std::vector<Accidental*> > groupAccidentalsByXPos(std::vector<Accidental*>& accidentals);
        void moveOctavesToSecondGroup(std::vector<std::vector<Accidental*> >& subGroups);

        std::vector<std::vector<Accidental*> > splitAccordingToNoteDisplacement(std::vector<Accidental*>& accidentals);
        std::vector<std::vector<Accidental*> > groupAccidentalsByNoteXPos(std::vector<Accidental*>& accidentals);
        void moveSecondsInSameGroup(std::vector<std::vector<Accidental*> >& subGroups);

        void determineStackingOrder(std::vector<Accidental*>& accidentals);
        void computeStandardOrdering(std::vector<Accidental*>& accidentals);
        void computeCompactOrdering(std::vector<Accidental*>& accidentals);
        void computeOrderingWithLeastColumns(std::vector<Accidental*>& accidentals);
        void findAndInsertSecond(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced, std::list<Accidental*>& accidentalsToPlace);
        bool findAndInsertOctave(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced, std::list<Accidental*>& accidentalsToPlace,
                                 bool acceptAbove = true, bool acceptBelow = true);

        void applyOrderingOffsets(std::vector<Accidental*>& accidentals);

        double minAccidentalToChordDistance(Accidental* acc, const Shape& accShape);
        double kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape, const ShapeElement& chordElement);
        double computePadding(Accidental* acc, const EngravingItem* chordElement);

        double minAccidentalToAccidentalGroupDistance(Accidental* acc, Shape accShape, const Shape& accidentalsShape);
        double kerningOfFourth(const Shape& accShape, const ShapeElement& accGroupShapeElement);
        void checkZeroColumn(Accidental* acc);

        double additionalPaddingForVerticals(const Accidental* acc, const EngravingItem* item);

        void verticallyAlignAccidentals();
        void collectVerticalSetsOfOffsetOctaves(
            std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets);
        void collectVerticalSets(
            std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets);
        void alignVerticalSets(std::vector<std::vector<Accidental*> >& vertSets);

        double verticalPadding(const Accidental* acc1, const Accidental* acc2);
        bool canFitInSameColumn(const Accidental* acc1, const Accidental* acc2);

        double horizontalPadding(const Accidental* acc1, const Accidental* acc2);

        static bool isExceptionOfFourth(const Accidental* acc1, const Accidental* acc2);
        static bool isExceptionOfNaturalsSixth(const Accidental* acc1, const Accidental* acc2);
        static bool isOctave(const Accidental* acc1, const Accidental* acc2);

        static void setXposRelativeToSegment(Accidental* accidental, double x);
        static double xPosRelativeToSegment(const Accidental* accidental);

        static void sortTopDown(std::vector<Accidental*>& accidentals);

    private:
        std::vector<Accidental*> m_allAccidentals;
        std::vector<Chord*> m_chords;

        std::vector<std::vector<Accidental*> > m_accidentalSubChords;
        std::vector<Accidental*> m_stackedAccidentals;

        Shape m_stackedAccidentalsShape;
        Shape m_chordsShape;

        double m_spatium = 0.0;
        double m_verticalAccidentalToAccidentalClearance = 0.0;
        double m_verticalSharpToSharpClearance = 0.0;
        double m_verticalAccidentalToChordClearance = 0.0;
        double m_accidentalAccidentalDistance = 0.0;
        double m_additionalPaddingForVerticals = 0.0;

        bool m_orderFollowNoteDisplacement = false;
        bool m_alignOctavesAcrossSubChords = false;
        bool m_keepSecondsTogether = false;
        bool m_alignOffsetOctaves = false;

        size_t m_subChordsCountBeforeOctaveMerge = 1;

        static constexpr int m_smallGroupLimit = 3;
        static constexpr int m_largeGroupLimit = 6;
    };
};
}

#endif // MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H
