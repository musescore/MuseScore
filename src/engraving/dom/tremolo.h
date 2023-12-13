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

class TremoloTwoChord;
class TremoloSingleChord;
class TremoloDispatcher final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TremoloDispatcher)
    DECLARE_CLASSOF(ElementType::TREMOLO)

public:

    TremoloDispatcher(Chord* parent);
    TremoloDispatcher(const TremoloDispatcher&);

    TremoloDispatcher& operator=(const TremoloDispatcher&) = delete;
    TremoloDispatcher* clone() const override { return new TremoloDispatcher(*this); }
    ~TremoloDispatcher() override;

    void setTrack(track_idx_t val) override;

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return m_tremoloType; }
    int subtype() const override { return static_cast<int>(m_tremoloType); }
    TranslatableString subtypeUserName() const override;
    bool isBuzzRoll() const { return m_tremoloType == TremoloType::BUZZ_ROLL; }
    bool twoNotes() const;

    Chord* chord() const { return toChord(explicitParent()); }
    void setParent(Chord* ch);

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    DirectionV direction() const;
    void setDirection(DirectionV val);

    double minHeight() const;
    void reset() override;

    PointF chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const;

    double chordMag() const;
    RectF drag(EditData&) override;

    void layout2();

    Chord* chord1() const;
    void setChord1(Chord* ch);
    Chord* chord2() const;
    void setChord2(Chord* ch);
    void setChords(Chord* c1, Chord* c2);

    PairF beamPos() const;
    double beamWidth() const;

    TDuration durationType() const;
    void setDurationType(TDuration d);

    bool userModified() const;
    void setUserModified(bool val);
    void setUserModified(DirectionV d, bool val);

    Fraction tremoloLen() const;

    int lines() const;
    bool up() const;
    void setUp(bool up);

    bool placeMidStem() const;

    bool crossStaffBeamBetween() const;

    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;
    void styleChanged() override;
    PointF pagePos() const override;
    String accessibleInfo() const override;
    void triggerLayout() const override;

    TremoloStyle tremoloStyle() const;
    void setTremoloStyle(TremoloStyle v);
    void setBeamFragment(const BeamFragment& bf);
    const BeamFragment& beamFragment() const;
    BeamFragment& beamFragment();

    bool playTremolo() const;
    void setPlayTremolo(bool v);

    bool customStyleApplicable() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    void setColor(const mu::draw::Color& c) override;
    mu::draw::Color color() const override;

    const ElementStyle* styledProperties() const override;
    PropertyFlags* propertyFlagsList() const override;
    PropertyFlags propertyFlags(Pid) const override;
    void setPropertyFlags(Pid, PropertyFlags) override;

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
    const mu::draw::PainterPath& path() const;
    void setPath(const mu::draw::PainterPath& p);

    const mu::PointF& startAnchor() const;
    mu::PointF& startAnchor();
    void setStartAnchor(const mu::PointF& p);
    const mu::PointF& endAnchor() const;
    mu::PointF& endAnchor();
    void setEndAnchor(const mu::PointF& p);

    const std::vector<BeamSegment*>& beamSegments() const;
    std::vector<BeamSegment*>& beamSegments();
    void clearBeamSegments();

    void computeShape();

    std::shared_ptr<rendering::dev::BeamTremoloLayout> layoutInfo();
    void setLayoutInfo(std::shared_ptr<rendering::dev::BeamTremoloLayout> info);

    TremoloTwoChord* twoChord = nullptr;
    TremoloSingleChord* singleChord = nullptr;

private:
    friend class Factory;
    friend class TremoloSingleChord;
    friend class TremoloTwoChord;

    void setParentInternal(EngravingObject* p) override;
    LayoutData* createLayoutData() const override;
    const LayoutData* ldataInternal() const override;
    LayoutData* mutldataInternal() override;

    void setBeamPos(const PairF& bp);

    TremoloType m_tremoloType = TremoloType::INVALID_TREMOLO;
};
} // namespace mu::engraving
#endif
