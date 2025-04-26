/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_SYSTEMLAYOUT_DEV_H
#define MU_ENGRAVING_SYSTEMLAYOUT_DEV_H

#include <vector>

#include "../dom/measure.h"

#include "../layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class Score;
class Segment;
class Spanner;
class System;
class Measure;
class Bracket;
class BracketItem;
class SkylineLine;
}

namespace mu::engraving::rendering::score {
class SystemLayout
{
public:
    static System* collectSystem(LayoutContext& ctx);
    static void layoutSystemElements(System* system, LayoutContext& ctx);

    static void layoutSystem(System* system, LayoutContext& ctx, double xo1, bool isFirstSystem = false, bool firstSystemIndent = false);

    static void hideEmptyStaves(System* system, LayoutContext& ctx, bool isFirstSystem);

    static void layout2(System* system, LayoutContext& ctx);
    static void restoreLayout2(System* system, LayoutContext& ctx);
    static void setMeasureHeight(System* system, double height, const LayoutContext& ctx);
    static void layoutBracketsVertical(System* system, LayoutContext& ctx);
    static void layoutInstrumentNames(System* system, LayoutContext& ctx);

    static void setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick = { 0, 1 });

    static double minDistance(const System* top, const System* bottom, const LayoutContext& ctx);

    static void centerElementsBetweenStaves(const System* system);
    static void centerBigTimeSigsAcrossStaves(const System* system);

    static void updateSkylineForElement(EngravingItem* element, const System* system, double yMove);

    static void layoutSystemLockIndicators(System* system, LayoutContext& ctx);

private:
    struct MeasureState
    {
        Measure* measure = nullptr;
        double measureWidth = 0.0;
        double measurePos = 0.0;
        std::vector < std::pair<Segment*, double> > segmentsPos;
        bool curHeader = false;
        bool curTrailer = false;

        void clear()
        {
            measure = nullptr;
            measureWidth = 0.0;
            measurePos = 0.0;
            segmentsPos.clear();
        }

        void restoreMeasure()
        {
            measure->mutldata()->setPosX(measurePos);
            measure->setWidth(measureWidth);
            for (auto pair : segmentsPos) {
                Segment* segment = pair.first;
                double x = pair.second;
                segment->mutldata()->setPosX(x);
            }
        }
    };

    struct ElementsToLayout
    {
        System* system;
        std::vector<Measure*> measures;
        std::vector<Segment*> segments;

        std::vector<ChordRest*> chordRests;
        std::vector<Chord*> chords;
        std::vector<BarLine*> barlines;
        std::vector<TimeSig*> timeSigAboveStaves;

        std::vector<MeasureNumber*> measureNumbers;
        std::vector<MMRestRange*> mmrRanges;
        std::vector<EngravingItem*> markersAndJumps;

        std::vector<Sticking*> stickings;
        std::vector<EngravingItem*> fermatasAndTremoloBars;
        std::vector<FiguredBass*> figuredBass;
        std::vector<Dynamic*> dynamics;
        std::vector<Expression*> expressions;
        std::vector<HarpPedalDiagram*> harpDiagrams;
        std::vector<FretDiagram*> fretDiagrams;
        std::vector<StaffText*> staffText;
        std::vector<InstrumentChange*> instrChanges;
        std::vector<SystemText*> systemText;
        std::vector<EngravingItem*> playTechCapoStringTunSystemTextTripletFeel;
        std::vector<RehearsalMark*> rehMarks;
        std::vector<TempoText*> tempoText;
        std::vector<Image*> images;
        std::vector<Parenthesis*> parenthesis;

        std::vector<Spanner*> slurs;
        std::vector<Spanner*> trills;
        std::vector<Spanner*> hairpins;
        std::vector<Spanner*> ottavas;
        std::vector<Spanner*> pedal;
        std::vector<Spanner*> voltas;
        std::vector<Spanner*> tempoChangeLines;
        std::vector<Spanner*> partialLyricsLines;
        std::vector<Spanner*> allOtherSpanners;

        ElementsToLayout(System* s)
            : system(s) {}
    };

    static void collectElementsToLayout(Measure* measure, ElementsToLayout& elements, const LayoutContext& ctx);
    static void collectSpannersToLayout(ElementsToLayout& elements, const LayoutContext& ctx);

    static System* getNextSystem(LayoutContext& lc);
    static void createSkylines(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);
    static void processLines(System* system, LayoutContext& ctx, const std::vector<Spanner*>& lines, bool align = false);
    static void layoutTies(Chord* ch, System* system, const Fraction& stick, LayoutContext& ctx);
    static void doLayoutTies(System* system, const std::vector<Segment*>& sl, const Fraction& stick, const Fraction& etick,
                             LayoutContext& ctx);
    static void doLayoutNoteSpannersLinear(System* system, LayoutContext& ctx);
    static void layoutNoteAnchoredSpanners(System* system, Chord* chord);
    static void layoutGuitarBends(Chord* chord, LayoutContext& ctx);
    static void updateCrossBeams(System* system, LayoutContext& ctx);
    static bool measureHasCrossStuffOrModifiedBeams(const Measure* measure);
    static void restoreTiesAndBends(System* system, LayoutContext& ctx);
    static void layoutTuplets(const std::vector<ChordRest*>& chordRests, LayoutContext& ctx);

    static void layoutTiesAndBends(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static double instrumentNamesWidth(System* system, LayoutContext& ctx, bool isFirstSystem);
    static double totalBracketOffset(LayoutContext& ctx);
    static double layoutBrackets(System* system, LayoutContext& ctx);
    static void addBrackets(System* system, Measure* measure, LayoutContext& ctx);
    static Bracket* createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                  std::vector<Bracket*>& bl, Measure* measure);
    static double minVertSpaceForCrossStaffBeams(System* system, staff_idx_t staffIdx1, staff_idx_t staffIdx2, LayoutContext& ctx);

    static bool elementShouldBeCenteredBetweenStaves(const EngravingItem* item, const System* system);
    static bool mmRestShouldBeCenteredBetweenStaves(const MMRest* mmRest, const System* system);
    static bool elementHasAnotherStackedOutside(const EngravingItem* element, const Shape& elementShape, const SkylineLine& skylineLine);
    static void centerElementBetweenStaves(EngravingItem* element, const System* system);
    static void centerMMRestBetweenStaves(MMRest* mmRest, const System* system);

    static bool shouldBeJustified(System* system, double curSysWidth, double targetSystemWidth, LayoutContext& ctx);

    static void updateBigTimeSigIfNeeded(System* system, LayoutContext& ctx);

    static void layoutSticking(const std::vector<Sticking*> stickings, System* system, LayoutContext& ctx);

    static void layoutLyrics(const ElementsToLayout& elements, LayoutContext& ctx);

    static void layoutVoltas(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static void layoutDynamicExpressionAndHairpins(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static void layoutParenthesisAndBigTimeSigs(const ElementsToLayout& elementsToLayout);
};
}

#endif // MU_ENGRAVING_SYSTEMLAYOUT_DEV_H
