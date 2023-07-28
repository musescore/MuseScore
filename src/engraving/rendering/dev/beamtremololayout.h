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

#ifndef MU_ENGRAVING_BEAMTREMOLOLAYOUT_H
#define MU_ENGRAVING_BEAMTREMOLOLAYOUT_H

#include "libmscore/beam.h"
#include "libmscore/engravingitem.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Beam;
enum class ActionIconType;
enum class SpannerSegmentType;
}

enum class SlopeConstraint
{
    NO_CONSTRAINT,
    FLAT,
    SMALL_SLOPE,
};

namespace mu::engraving::rendering::dev {
class BeamTremoloLayout
{
public:
    BeamTremoloLayout() {}
    BeamTremoloLayout(EngravingItem* e);

    double beamDist() const { return m_beamDist; }
    double beamWidth() const { return m_beamWidth; }
    PointF startAnchor() const { return m_startAnchor; }
    PointF endAnchor() const { return m_endAnchor; }
    void setAnchors(PointF startAnchor, PointF endAnchor) { m_startAnchor = startAnchor; m_endAnchor = endAnchor; }

    bool calculateAnchors(const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes);

    double chordBeamAnchorX(const ChordRest* chord, ChordBeamAnchorType anchorType) const;
    double chordBeamAnchorY(const ChordRest* chord) const;
    PointF chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const;
    int getMaxSlope() const;
    void extendStem(Chord* chord, double addition);
    bool isValid() const { return !(m_beamType == BeamType::INVALID); }

private:
    enum class BeamType {
        INVALID,
        BEAM,
        TREMOLO
    };
    BeamType m_beamType = BeamType::INVALID;
    EngravingItem* m_element = nullptr;
    Beam* m_beam = nullptr;
    Tremolo* m_trem = nullptr;
    bool m_up = false;
    Fraction m_tick = Fraction(0, 1);
    double m_spatium = 0.;
    PointF m_startAnchor;
    PointF m_endAnchor;
    double m_slope = 0.;
    bool m_isGrace = false;
    int m_beamSpacing = 0;
    double m_beamDist = 0.;
    double m_beamWidth = 0.;
    std::vector<ChordRest*> m_elements;
    std::vector<int> m_notes;
    StaffType const* m_tab = nullptr;
    bool m_isBesideTabStaff = false;

    int getMiddleStaffLine(ChordRest* startChord, ChordRest* endChord, int staffLines) const;
    int computeDesiredSlant(int startNote, int endNote, int middleLine, int dictator, int pointer) const;
    SlopeConstraint getSlopeConstraint(int startNote, int endNote) const;
    void offsetBeamWithAnchorShortening(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, int staffLines,
                                        bool isStartDictator, int stemLengthDictator) const;
    bool isValidBeamPosition(int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter) const;
    bool isBeamInsideStaff(int yPos, int staffLines, bool allowFloater) const;
    int getOuterBeamPosOffset(int innerBeam, int beamCount, int staffLines) const;
    void offsetBeamToRemoveCollisions(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, const double startX,
                                      const double endX, bool isFlat, bool isStartDictator) const;
    int getBeamCount(std::vector<ChordRest*> chordRests) const;
    bool is64thBeamPositionException(int& yPos, int staffLines) const;
    int findValidBeamOffset(int outer, int beamCount, int staffLines, bool isStart, bool isAscending, bool isFlat) const;
    void setValidBeamPositions(int& dictator, int& pointer, int beamCountD, int beamCountP, int staffLines, bool isStartDictator,
                               bool isFlat, bool isAscending);
    void addMiddleLineSlant(int& dictator, int& pointer, int beamCount, int middleLine, int interval, int desiredSlant);
    void add8thSpaceSlant(mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount, int interval, int middleLine, bool Flat);
    bool noSlope();
    int strokeCount(ChordRest* cr) const;
    bool calculateAnchorsCross();
    bool computeTremoloUp();
};
} // namespace mu::engraving
#endif
