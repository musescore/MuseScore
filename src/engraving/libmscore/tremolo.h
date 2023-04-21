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

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

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

    TremoloType _tremoloType { TremoloType::R8 };
    Chord* _chord1 { nullptr };
    Chord* _chord2 { nullptr };
    TDuration _durationType;
    bool _up{ true };
    bool _userModified[2]{ false };                // 0: auto/down  1: up
    DirectionV _direction;
    mu::draw::PainterPath path;
    std::vector<BeamSegment*> _beamSegments;
    BeamTremoloLayout _layoutInfo;
    mu::PointF _startAnchor;
    mu::PointF _endAnchor;

    int _lines = 0;         // derived from _subtype
    TremoloStyle _style { TremoloStyle::DEFAULT };
    // for _startAnchor and _slope, once we decide to change trems so that they can
    // continue from one system to the other (or indeed, one measure to the other)
    // we will want a vector of fragments similar to Beam's _beamFragments structure.
    // for now, a single fragment is sufficient
    BeamFragment _beamFragment;

    friend class Factory;
    Tremolo(Chord* parent);
    Tremolo(const Tremolo&);

    mu::draw::PainterPath basePath(double stretch = 0) const;
    void computeShape();
    void layoutOneNoteTremolo(double x, double y, double h, double spatium);
    void layoutTwoNotesTremolo(double x, double y, double h, double spatium);
    void createBeamSegments();
    void setBeamPos(const PairF& bp);

public:

    Tremolo& operator=(const Tremolo&) = delete;
    Tremolo* clone() const override { return new Tremolo(*this); }

    Chord* chord() const { return toChord(explicitParent()); }
    void setParent(Chord* ch);

    int subtype() const override { return static_cast<int>(_tremoloType); }
    TranslatableString subtypeUserName() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return _tremoloType; }

    DirectionV direction() const { return _direction; }

    void setUserModified(DirectionV d, bool val);

    double minHeight() const;
    void reset() override;

    PointF chordBeamAnchor(const ChordRest* chord, BeamTremoloLayout::ChordBeamAnchorType anchorType) const;

    double chordMag() const;
    double mag() const override;
    RectF drag(EditData&) override;
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    void layout2();

    Chord* chord1() const { return _chord1; }
    Chord* chord2() const { return _chord2; }
    PairF beamPos() const;
    double beamWidth() const;

    TDuration durationType() const;
    void setDurationType(TDuration d);

    void setChords(Chord* c1, Chord* c2)
    {
        _chord1 = c1;
        _chord2 = c2;
    }

    bool userModified() const;
    void setUserModified(bool val);

    Fraction tremoloLen() const;
    bool isBuzzRoll() const { return _tremoloType == TremoloType::BUZZ_ROLL; }
    bool twoNotes() const { return _tremoloType >= TremoloType::C8; }    // is it a two note tremolo?
    int lines() const { return _lines; }
    bool up() const { return _up; }

    bool placeMidStem() const;

    bool crossStaffBeamBetween() const;

    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;
    void styleChanged() override;
    PointF pagePos() const override;      ///< position in page coordinates
    String accessibleInfo() const override;
    void triggerLayout() const override;

    TremoloStyle style() const { return _style; }
    void setStyle(TremoloStyle v) { _style = v; }
    void setBeamDirection(DirectionV v);
    void setBeamFragment(const BeamFragment& bf) { _beamFragment = bf; }
    const BeamFragment& beamFragment() const { return _beamFragment; }

    double defaultStemLengthStart();
    double defaultStemLengthEnd();
    bool customStyleApplicable() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    void setUp(bool up) { _up = up; }

    // only need grips for two-note trems
    bool needStartEditingAfterSelecting() const override { return twoNotes(); }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
    bool isMovable() const override { return true; }
    void startDrag(EditData&) override {}
    bool isEditable() const override { return true; }
    void startEdit(EditData&) override {}
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;
};
} // namespace mu::engraving
#endif
