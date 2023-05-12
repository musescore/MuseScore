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

#ifndef __TUPLET_H__
#define __TUPLET_H__

#include <set>

#include "durationelement.h"
#include "property.h"

namespace mu::engraving {
class Text;
class Spanner;
enum class TupletNumberType : char;
enum class TupletBracketType : char;

//------------------------------------------------------------------------
//   @@ Tuplet
//!     Example of 1/8 triplet:
//!       _baseLen     = 1/8  (tuplet is measured in eighth notes)
//!       _ratio       = 3/2  (3 eighth tuplet notes played in the space of 2 regular eighth notes)
//!
//!    Entire tuplet has a duration of _baseLen * _ratio.denominator().
//!    A single tuplet note has duration of _baseLen * _ratio.denominator() / _ratio.numerator().
//------------------------------------------------------------------------

class Tuplet final : public DurationElement
{
    OBJECT_ALLOCATOR(engraving, Tuplet)
    DECLARE_CLASSOF(ElementType::TUPLET)

public:
    Tuplet(Measure* parent);
    Tuplet(const Tuplet&);
    ~Tuplet();

    void setParent(Measure* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Tuplet* clone() const override { return new Tuplet(*this); }
    void setTrack(track_idx_t val) override;

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    Text* number() const { return _number; }
    void setNumber(Text* t) { _number = t; }
    void resetNumberProperty();
    static void resetNumberProperty(Text* number);

    bool isEditable() const override;
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    void setSelected(bool f) override;

    Measure* measure() const override { return toMeasure(explicitParent()); }

    TupletNumberType numberType() const { return _numberType; }
    TupletBracketType bracketType() const { return _bracketType; }
    void setNumberType(TupletNumberType val) { _numberType = val; }
    void setBracketType(TupletBracketType val) { _bracketType = val; }
    bool hasBracket() const { return _hasBracket; }
    void setHasBracket(bool b) { _hasBracket = b; }
    Millimetre bracketWidth() const { return _bracketWidth; }
    void setBracketWidth(Millimetre s) { _bracketWidth = s; }

    Fraction ratio() const { return _ratio; }
    void setRatio(const Fraction& r) { _ratio = r; }

    void setUserPoint1(PointF p) { _p1 = p; }
    void setUserPoint2(PointF p) { _p2 = p; }

    const std::vector<DurationElement*>& elements() const { return _currentElements; }
    void clear() { _currentElements.clear(); }
    bool contains(const DurationElement* el) const
    {
        return std::find(_currentElements.begin(), _currentElements.end(), el) != _currentElements.end();
    }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void reset() override;

    void draw(mu::draw::Painter*) const override;
    int id() const { return _id; }
    void setId(int i) const { _id = i; }

    TDuration baseLen() const { return _baseLen; }
    void setBaseLen(const TDuration& d) { _baseLen = d; }

    void dump() const override;

    void setDirection(DirectionV d) { _direction = d; }
    DirectionV direction() const { return _direction; }
    bool isUp() const { return _isUp; }
    bool isSmall() const { return _isSmall; }
    Fraction tick() const override { return _tick; }
    Fraction rtick() const override;
    void setTick(const Fraction& v) { _tick = v; }
    Fraction elementsDuration();
    void sortElements();
    bool cross() const;

    void setVisible(bool f) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    Shape shape() const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    void sanitizeTuplet();
    void addMissingElements();

    static int computeTupletDenominator(int numerator, Fraction totalDuration);

private:
    friend class TupletLayout;
    friend class DurationElement;

    void addDurationElement(DurationElement* de);
    void removeDurationElement(DurationElement* de);

    Fraction addMissingElement(const Fraction& startTick, const Fraction& endTick);

    bool calcHasBracket(const DurationElement* cr1, const DurationElement* cr2) const;

    // All DurationElements where `tuplet()` returns this tuplet
    std::set<DurationElement*> _allElements;

    // Those DurationElements that are currently really part of this tuplet
    std::vector<DurationElement*> _currentElements;

    bool _beingDestructed = false;

    DirectionV _direction;
    TupletNumberType _numberType;
    TupletBracketType _bracketType;
    Millimetre _bracketWidth;

    bool _hasBracket;
    Fraction _ratio;
    TDuration _baseLen;        // 1/8 for a triplet of 1/8

    bool _isUp;
    bool _isSmall;

    Fraction _tick;

    mu::PointF p1, p2;
    mu::PointF _p1, _p2;         // user offset
    mutable int _id;          // used during read/write

    Text* _number = nullptr;
    mu::PointF bracketL[4];
    mu::PointF bracketR[3];
};
} // namespace mu::engraving
#endif
