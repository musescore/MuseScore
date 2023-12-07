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

#ifndef MU_ENGRAVING_TREMOLOBEAMLAYOUT_DEV_H
#define MU_ENGRAVING_TREMOLOBEAMLAYOUT_DEV_H

#include <vector>
#include "draw/types/geometry.h"

#include "dom/beam.h"
#include "dom/engravingitem.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Beam;
enum class ActionIconType;
enum class SpannerSegmentType;
}

namespace mu::engraving::rendering::dev {
class TremoloBeamLayout
{
public:
    TremoloBeamLayout() = default;

    double beamDist() const { return m_beamDist; }
    double beamWidth() const { return m_beamWidth; }
    PointF startAnchor() const { return m_startAnchor; }
    PointF endAnchor() const { return m_endAnchor; }
    void setAnchors(PointF startAnchor, PointF endAnchor) { m_startAnchor = startAnchor; m_endAnchor = endAnchor; }

    static void setupLData(TremoloBeamLayout* info, EngravingItem* e);

    static bool calculateAnchors(TremoloBeamLayout* info, const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes);

    static double chordBeamAnchorX(const TremoloBeamLayout* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static double chordBeamAnchorY(const TremoloBeamLayout* info, const ChordRest* chord);
    static PointF chordBeamAnchor(const TremoloBeamLayout* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static int getMaxSlope(const TremoloBeamLayout* info);
    static void extendStem(const TremoloBeamLayout* info, Chord* chord, double addition);
    bool isValid() const { return m_trem != nullptr; }

private:
    enum class SlopeConstraint
    {
        NO_CONSTRAINT,
        FLAT,
        SMALL_SLOPE,
    };

    Tremolo* m_trem = nullptr;
    Beam* m_beam = nullptr;
    bool m_up = false;
    Fraction m_tick = Fraction(0, 1);
    double m_spatium = 0.;
    PointF m_startAnchor;
    PointF m_endAnchor;
    double m_slope = 0.;
    bool m_isGrace = false;
    int m_beamSpacing = 0;
    double m_beamDist = 0.0;
    double m_beamWidth = 0.0;
    std::vector<ChordRest*> m_elements;
    std::vector<int> m_notes;
    StaffType const* m_tab = nullptr;
    bool m_isBesideTabStaff = false;

    static int getMiddleStaffLine(const TremoloBeamLayout* info, ChordRest* startChord, ChordRest* endChord, int staffLines);
    static int computeDesiredSlant(const TremoloBeamLayout* info, int startNote, int endNote, int middleLine, int dictator, int pointer);
    static SlopeConstraint getSlopeConstraint(const TremoloBeamLayout* info, int startNote, int endNote);
    static void offsetBeamWithAnchorShortening(const TremoloBeamLayout* info, std::vector<ChordRest*> chordRests, int& dictator,
                                               int& pointer, int staffLines, bool isStartDictator, int stemLengthDictator);
    static bool isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter);
    static bool isBeamInsideStaff(int yPos, int staffLines, bool allowFloater);
    static int getOuterBeamPosOffset(const TremoloBeamLayout* info, int innerBeam, int beamCount, int staffLines);
    static void offsetBeamToRemoveCollisions(const TremoloBeamLayout* info, std::vector<ChordRest*> chordRests, int& dictator, int& pointer,
                                             const double startX, const double endX, bool isFlat, bool isStartDictator);
    static int getBeamCount(const TremoloBeamLayout* info, std::vector<ChordRest*> chordRests);
    static bool is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines);
    static int findValidBeamOffset(const TremoloBeamLayout* info, int outer, int beamCount, int staffLines, bool isStart, bool isAscending,
                                   bool isFlat);
    static void setValidBeamPositions(const TremoloBeamLayout* info, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                      int staffLines, bool isStartDictator, bool isFlat, bool isAscending);
    static void addMiddleLineSlant(const TremoloBeamLayout* info, int& dictator, int& pointer, int beamCount, int middleLine, int interval,
                                   int desiredSlant);
    static void add8thSpaceSlant(TremoloBeamLayout* info, mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                 int interval, int middleLine, bool Flat);
    static bool noSlope(const Beam* beam);
    static int strokeCount(const TremoloBeamLayout* info, ChordRest* cr);
    static bool calculateAnchorsCross(TremoloBeamLayout* info);
    static bool computeTremoloUp(const TremoloBeamLayout* info);
};
} // namespace mu::engraving
#endif
