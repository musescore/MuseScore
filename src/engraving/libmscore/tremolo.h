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

    TremoloType _tremoloType { TremoloType::R8 };
    Chord* _chord1 { nullptr };
    Chord* _chord2 { nullptr };
    TDuration _durationType;
    bool _up{ true };
    DirectionV _direction;
    mu::draw::PainterPath path;

    int _lines = 0;         // derived from _subtype
    TremoloStyle _style { TremoloStyle::DEFAULT };

    friend class Factory;
    Tremolo(Chord* parent);
    Tremolo(const Tremolo&);

    mu::draw::PainterPath basePath(double stretch = 0) const;
    void computeShape();
    void layoutOneNoteTremolo(double x, double y, double h, double spatium);
    void layoutTwoNotesTremolo(double x, double y, double h, double spatium);

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

    double minHeight() const;

    double chordMag() const;
    double mag() const override;
    void draw(mu::draw::Painter*) const override;
    void layout() override;
    void layout2();
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    Chord* chord1() const { return _chord1; }
    Chord* chord2() const { return _chord2; }

    TDuration durationType() const;
    void setDurationType(TDuration d);

    void setChords(Chord* c1, Chord* c2)
    {
        _chord1 = c1;
        _chord2 = c2;
    }

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

    String accessibleInfo() const override;

    TremoloStyle style() const { return _style; }
    void setStyle(TremoloStyle v) { _style = v; }
    void setBeamDirection(DirectionV v);

    double defaultStemLengthStart();
    double defaultStemLengthEnd();
    bool customStyleApplicable() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
};
} // namespace mu::engraving
#endif
