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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

namespace mu::engraving {
class Trill;
class Accidental;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, TrillSegment)
    DECLARE_CLASSOF(ElementType::TRILL_SEGMENT)

public:
    TrillSegment(Trill* sp, System* parent);
    TrillSegment(System* parent);

    Trill* trill() const { return (Trill*)spanner(); }

    TrillSegment* clone() const override { return new TrillSegment(*this); }
    void draw(mu::draw::Painter*) const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all) override;

    EngravingItem* propertyDelegate(Pid) override;

    void remove(EngravingItem*) override;
    Shape shape() const override;

    SymIdList symbols() const { return _symbols; }
    void setSymbols(const SymIdList& s) { _symbols = s; }

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);

private:
    Sid getPropertyStyle(Pid) const override;

    SymIdList _symbols;
};

//---------------------------------------------------------
//   @@ Trill
//   @P trillType  enum (Trill.DOWNPRALL_LINE, .PRALLPRALL_LINE, .PURE_LINE, .TRILL_LINE, .UPPRALL_LINE)
//---------------------------------------------------------

class Trill final : public SLine
{
    OBJECT_ALLOCATOR(engraving, Trill)
    DECLARE_CLASSOF(ElementType::TRILL)

    Sid getPropertyStyle(Pid) const override;

private:
    TrillType _trillType = TrillType::TRILL_LINE;
    Accidental* _accidental = nullptr;
    Chord* _cueNoteChord = nullptr;
    OrnamentStyle _ornamentStyle = OrnamentStyle::DEFAULT;   // for use in ornaments such as trill
    bool _playArticulation = true;
    Ornament* _ornament = nullptr;

public:
    Trill(EngravingItem* parent);
    Trill(const Trill& t);
    ~Trill();

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Trill* clone() const override { return new Trill(*this); }
    EngravingItem* linkedClone() override;

    LineSegment* createLineSegment(System* parent) override;
    void remove(EngravingItem*) override;

    void setTrack(track_idx_t n) override;

    void setTrillType(TrillType tt);
    TrillType trillType() const { return _trillType; }
    void setOrnamentStyle(OrnamentStyle val) { _ornamentStyle = val; }
    OrnamentStyle ornamentStyle() const { return _ornamentStyle; }
    void setPlayArticulation(bool val) { _playArticulation = val; }
    bool playArticulation() const { return _playArticulation; }
    String trillTypeUserName() const;
    Accidental* accidental() const { return _accidental; }
    void setAccidental(Accidental* a) { _accidental = a; }
    Chord* cueNoteChord() const { return _cueNoteChord; }
    void setCueNoteChord(Chord* c) { _cueNoteChord = c; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;

    Ornament* ornament() const { return _ornament; }
    void setOrnament(Ornament* o) { _ornament = o; }
};
} // namespace mu::engraving

#endif
