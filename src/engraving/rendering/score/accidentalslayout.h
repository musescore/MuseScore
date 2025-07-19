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

namespace mu::engraving::rendering::score {
class LayoutContext;

using AccidentalGroups = std::vector<std::vector<Accidental*> >;

class AccidentalsLayout
{
public:
    static void layoutAccidentals(const std::vector<Chord*>& chords, LayoutContext& ctx);

private:
    struct AccidentalsLayoutContext {
        AccidentalsLayoutContext (std::vector<Accidental*> acc, std::vector<Chord*> ch);

        std::vector<Accidental*> allAccidentals;
        std::vector<Chord*> chords;
        AccidentalGroups accidentalSubChords;
        std::vector<Accidental*> stackedAccidentals;
        Shape stackedAccidentalsShape;
        Shape chordsShape;
        size_t subChordsCountBeforeOctaveMerge = 1;

        double spatium() const { return m_spatium; }
        double verticalAccidentalToAccidentalClearance() const { return m_verticalAccidentalToAccidentalClearance; }
        double verticalSharpToSharpClearance() const { return m_verticalSharpToSharpClearance; }
        double verticalAccidentalToChordClearance() const { return m_verticalAccidentalToChordClearance; }
        double accidentalAccidentalDistance() const { return m_accidentalAccidentalDistance; }
        double additionalPaddingForVerticals() const { return m_additionalPaddingForVerticals; }
        double xPosSplitThreshold() const { return m_xPosSplitThreshold; }
        double xVerticalAlignmentThreshold() const { return m_xVerticalAlignmentThreshold; }
        double sharpAndNaturalLedgerLinePadding() const { return m_sharpAndNaturalLedgerLinePadding; }
        double reducedFlatToNotePadding() const { return m_reducedFlatToNotePadding; }
        double flatKerningOfFourth() const { return m_flatKerningOfFourth; }
        double naturalKerningOfFourth() const { return m_naturalKerningOfFourth; }

        bool orderFollowNoteDisplacement() const { return m_orderFollowNoteDisplacement; }
        bool alignOctavesAcrossSubChords() const { return m_alignOctavesAcrossSubChords; }
        bool keepSecondsTogether() const { return m_keepSecondsTogether; }
        bool alignOffsetOctaves() const { return m_alignOffsetOctaves; }
        size_t largeGroupLimit() const { return m_largeGroupLimit; }
        size_t smallGroupLimit() const { return m_smallGroupLimit; }

    private:
        void initConstants();

        double m_spatium = 0.0;
        double m_verticalAccidentalToAccidentalClearance = 0.0;
        double m_verticalSharpToSharpClearance = 0.0;
        double m_verticalAccidentalToChordClearance = 0.0;
        double m_accidentalAccidentalDistance = 0.0;
        double m_additionalPaddingForVerticals = 0.0;
        double m_xPosSplitThreshold = 0.0;
        double m_xVerticalAlignmentThreshold = 0.0;
        double m_sharpAndNaturalLedgerLinePadding = 0.0;
        double m_reducedFlatToNotePadding = 0.0;
        double m_flatKerningOfFourth = 0.0;
        double m_naturalKerningOfFourth = 0.0;
        bool m_orderFollowNoteDisplacement = false;
        bool m_alignOctavesAcrossSubChords = false;
        bool m_keepSecondsTogether = false;
        bool m_alignOffsetOctaves = false;

        static constexpr size_t m_smallGroupLimit = 3;
        static constexpr size_t m_largeGroupLimit = 6;
    };

    static void collectAccidentals(const std::vector<Chord*> chords, std::vector<Accidental*>& allAccidentals,
                                   std::vector<Accidental*>& redundantAccidentals, std::vector<Accidental*>& invisibleAccidentals);
    static bool accidentalIsRedundant(const Accidental* acc, const std::vector<Accidental*>& allAccidentals);

    static void doAccidentalPlacement(AccidentalsLayoutContext& ctx);

    static void findOctavesAndSeconds(const AccidentalsLayoutContext& ctx);

    static void splitIntoSubChords(AccidentalsLayoutContext& ctx);
    static void mergeAdjacentSubGroupsIfTooSmall(AccidentalsLayoutContext& ctx);
    static void mergeSubGroupsWithOctavesAcross(AccidentalsLayoutContext& ctx);

    static void createChordsShape(AccidentalsLayoutContext& ctx);

    static void layoutSubChord(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static void stackAccidental(Accidental* acc, AccidentalsLayoutContext& ctx);

    static void computeOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static AccidentalGroups splitIntoPriorityGroups(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);

    static AccidentalGroups splitAccordingToAccidDisplacement(std::vector<Accidental*>& accidentals, const AccidentalsLayoutContext& ctx);
    static AccidentalGroups groupAccidentalsByXPos(std::vector<Accidental*>& accidentals, const AccidentalsLayoutContext& ctx);
    static void moveOctavesToSecondGroup(AccidentalGroups& subGroups);

    static AccidentalGroups splitAccordingToNoteDisplacement(std::vector<Accidental*>& accidentals, const AccidentalsLayoutContext& ctx);
    static AccidentalGroups groupAccidentalsByNoteXPos(const std::vector<Accidental*>& accidentals);
    static void moveSecondsInSameGroup(AccidentalGroups& subGroups);

    static void determineStackingOrder(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static void computeStandardOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static void computeCompactOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static void computeOrderingWithLeastColumns(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx);
    static void findAndInsertSecond(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                    std::list<Accidental*>& accidentalsToPlace, AccidentalsLayoutContext& ctx);
    static bool findAndInsertOctave(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                    std::list<Accidental*>& accidentalsToPlace, AccidentalsLayoutContext& ctx, bool acceptAbove = true,
                                    bool acceptBelow = true);

    static void applyOrderingOffsets(std::vector<Accidental*>& accidentals);

    static double minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const AccidentalsLayoutContext& ctx);
    static double kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape, const ShapeElement& chordElement,
                                              const AccidentalsLayoutContext& ctx);
    static double computePadding(Accidental* acc, const EngravingItem* chordElement, const AccidentalsLayoutContext& ctx);

    static double minAccidentalToAccidentalGroupDistance(Accidental* acc, Shape accShape, const Shape& accidentalsShape,
                                                         const AccidentalsLayoutContext& ctx);
    static double kerningOfFourth(const Shape& accShape, const ShapeElement& accGroupShapeElement, const AccidentalsLayoutContext& ctx);
    static void checkZeroColumn(Accidental* acc, const AccidentalsLayoutContext& ctx);

    static double additionalPaddingForVerticals(const Accidental* acc, const EngravingItem* item, const AccidentalsLayoutContext& ctx);

    static void verticallyAlignAccidentals(AccidentalsLayoutContext& ctx);
    static void collectVerticalSetsOfOffsetOctaves(std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*,
                                                                                                              std::vector<Accidental*> >& verticalSets, AccidentalsLayoutContext& ctx);
    static void collectVerticalSets(std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*,
                                                                                               std::vector<Accidental*> >& verticalSets,
                                    AccidentalsLayoutContext& ctx);
    static void alignVerticalSets(AccidentalGroups& vertSets, AccidentalsLayoutContext& ctx);

    static double verticalPadding(const Accidental* acc1, const Accidental* acc2, const AccidentalsLayoutContext& ctx);
    static bool canFitInSameColumn(const Accidental* acc1, const Accidental* acc2, AccidentalsLayoutContext& ctx);

    static double horizontalPadding(const Accidental* acc1, const Accidental* acc2, const AccidentalsLayoutContext& ctx);

    static bool isExceptionOfFourth(const Accidental* acc1, const Accidental* acc2);
    static bool isExceptionOfNaturalsSixth(const Accidental* acc1, const Accidental* acc2);
    static bool isOctave(const Accidental* acc1, const Accidental* acc2);

    static void setXposRelativeToSegment(Accidental* accidental, double x);
    static double xPosRelativeToSegment(const Accidental* accidental);

    static void sortTopDown(std::vector<Accidental*>& accidentals);

    static bool keepAccidentalsCloseToChord(const Chord* chord);
};
}

#endif // MU_ENGRAVING_ACCIDENTALSLAYOUT_DEV_H
