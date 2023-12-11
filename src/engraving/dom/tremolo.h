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

#ifndef MU_ENGRAVING_TREMOLO_H
#define MU_ENGRAVING_TREMOLO_H

#include <memory>

#include "engravingitem.h"

#include "durationtype.h"
#include "draw/types/painterpath.h"
#include "types/types.h"
#include "beam.h"
#include "chord.h"

namespace mu::engraving {
class Chord;

// only applicable to minim two-note tremolo in non-TAB staves
enum class TremoloStyle : signed char {
    DEFAULT = 0, TRADITIONAL, TRADITIONAL_ALTERNATE
};

//---------------------------------------------------------
//   @@ Tremolo
//---------------------------------------------------------

class Tremolo final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Tremolo)
    DECLARE_CLASSOF(ElementType::TREMOLO)

public:

    Tremolo& operator=(const Tremolo&) = delete;
    Tremolo* clone() const override { return new Tremolo(*this); }
    ~Tremolo() override;

    Chord* chord() const { return toChord(explicitParent()); }
    void setParent(Chord* ch);

    int subtype() const override { return static_cast<int>(m_tremoloType); }
    TranslatableString subtypeUserName() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return m_tremoloType; }

    DirectionV direction() const { return m_direction; }
    void setDirection(DirectionV val) { m_direction = val; }

    void setUserModified(DirectionV d, bool val);

    double minHeight() const;
    void reset() override;

    PointF chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const;

    double chordMag() const;
    RectF drag(EditData&) override;

    void layout2();

    Chord* chord1() const { return m_chord1; }
    void setChord1(Chord* ch) { m_chord1 = ch; }
    Chord* chord2() const { return m_chord2; }
    void setChord2(Chord* ch) { m_chord2 = ch; }
    void setChords(Chord* c1, Chord* c2)
    {
        m_chord1 = c1;
        m_chord2 = c2;
    }

    PairF beamPos() const;
    double beamWidth() const;

    TDuration durationType() const;
    void setDurationType(TDuration d);

    bool userModified() const;
    void setUserModified(bool val);

    Fraction tremoloLen() const;
    bool isBuzzRoll() const { return m_tremoloType == TremoloType::BUZZ_ROLL; }
    bool twoNotes() const { return m_tremoloType >= TremoloType::C8; }    // is it a two note tremolo?
    int lines() const { return m_lines; }
    bool up() const { return m_up; }
    void setUp(bool up) { m_up = up; }

    bool placeMidStem() const;

    bool crossStaffBeamBetween() const;

    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;
    void styleChanged() override;
    PointF pagePos() const override;      ///< position in page coordinates
    String accessibleInfo() const override;
    void triggerLayout() const override;

    TremoloStyle tremoloStyle() const { return m_style; }
    void setTremoloStyle(TremoloStyle v) { m_style = v; }
    void setBeamDirection(DirectionV v);
    void setBeamFragment(const BeamFragment& bf) { m_beamFragment = bf; }
    const BeamFragment& beamFragment() const { return m_beamFragment; }
    BeamFragment& beamFragment() { return m_beamFragment; }

    bool playTremolo() const { return m_playTremolo; }
    void setPlayTremolo(bool v) { m_playTremolo = v; }

    bool customStyleApplicable() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    // only need grips for two-note trems
    bool needStartEditingAfterSelecting() const override;
    int gripsCount() const override;
    Grip initialEditModeGrip() const override;
    Grip defaultGrip() const override;
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
    bool isMovable() const override { return true; }
    bool isEditable() const override { return true; }
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;

    mu::draw::PainterPath basePath(double stretch = 0) const;
    const mu::draw::PainterPath& path() const { return m_path; }
    void setPath(const mu::draw::PainterPath& p) { m_path = p; }

    const mu::PointF& startAnchor() const { return m_startAnchor; }
    mu::PointF& startAnchor() { return m_startAnchor; }
    void setStartAnchor(const mu::PointF& p) { m_startAnchor = p; }
    const mu::PointF& endAnchor() const { return m_endAnchor; }
    mu::PointF& endAnchor() { return m_endAnchor; }
    void setEndAnchor(const mu::PointF& p) { m_endAnchor = p; }

    const std::vector<BeamSegment*>& beamSegments() const { return m_beamSegments; }
    std::vector<BeamSegment*>& beamSegments() { return m_beamSegments; }
    void clearBeamSegments();

    void computeShape();

    std::shared_ptr<rendering::dev::BeamTremoloLayout> layoutInfo;

private:
    friend class Factory;

    Tremolo(Chord* parent);
    Tremolo(const Tremolo&);

    void setBeamPos(const PairF& bp);

    TremoloType m_tremoloType = TremoloType::R8;
    Chord* m_chord1 = nullptr;
    Chord* m_chord2 = nullptr;
    TDuration m_durationType;
    bool m_up = true;
    bool m_userModified[2]{ false };                // 0: auto/down  1: up
    DirectionV m_direction = DirectionV::AUTO;
    mu::draw::PainterPath m_path;
    std::vector<BeamSegment*> m_beamSegments;
    bool m_playTremolo = true;

    mu::PointF m_startAnchor;
    mu::PointF m_endAnchor;

    int m_lines = 0;         // derived from _subtype
    TremoloStyle m_style = TremoloStyle::DEFAULT;
    // for _startAnchor and _slope, once we decide to change trems so that they can
    // continue from one system to the other (or indeed, one measure to the other)
    // we will want a vector of fragments similar to Beam's _beamFragments structure.
    // for now, a single fragment is sufficient
    BeamFragment m_beamFragment;
};
} // namespace mu::engraving
#endif
