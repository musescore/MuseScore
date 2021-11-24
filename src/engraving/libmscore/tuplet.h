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

#include "duration.h"
#include "property.h"

namespace Ms {
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
    std::vector<DurationElement*> _elements;
    Direction _direction;
    TupletNumberType _numberType;
    TupletBracketType _bracketType;
    Milimetre _bracketWidth;

    bool _hasBracket;
    Fraction _ratio;
    TDuration _baseLen;        // 1/8 for a triplet of 1/8

    bool _isUp;
    bool _isSmall;

    Fraction _tick;

    mu::PointF p1, p2;
    mu::PointF _p1, _p2;         // user offset
    mutable int _id;          // used during read/write

    Text* _number;
    mu::PointF bracketL[4];
    mu::PointF bracketR[3];

    Fraction addMissingElement(const Fraction& startTick, const Fraction& endTick);

public:
    Tuplet(Measure* parent);
    Tuplet(const Tuplet&);
    ~Tuplet();

    void setParent(Measure* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int idx) const override;
    int scanChildCount() const override;

    Tuplet* clone() const override { return new Tuplet(*this); }
    void setTrack(int val) override;

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    Text* number() const { return _number; }
    void setNumber(Text* t) { _number = t; }
    void resetNumberProperty();

    bool isEditable() const override;
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    void setSelected(bool f) override;

    Measure* measure() const override { return toMeasure(parent()); }

    TupletNumberType numberType() const { return _numberType; }
    TupletBracketType bracketType() const { return _bracketType; }
    void setNumberType(TupletNumberType val) { _numberType = val; }
    void setBracketType(TupletBracketType val) { _bracketType = val; }
    bool hasBracket() const { return _hasBracket; }
    void setHasBracket(bool b) { _hasBracket = b; }
    Milimetre bracketWidth() const { return _bracketWidth; }
    void setBracketWidth(Milimetre s) { _bracketWidth = s; }

    Fraction ratio() const { return _ratio; }
    void setRatio(const Fraction& r) { _ratio = r; }

    const std::vector<DurationElement*>& elements() const { return _elements; }
    void clear() { _elements.clear(); }
    bool contains(const DurationElement* el) const
    {
        return std::find(_elements.begin(), _elements.end(), el) != _elements.end();
    }

    void layout() override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void read(XmlReader&) override;
    void write(XmlWriter&) const override;
    bool readProperties(XmlReader&) override;

    void reset() override;

    void draw(mu::draw::Painter*) const override;
    int id() const { return _id; }
    void setId(int i) const { _id = i; }

    TDuration baseLen() const { return _baseLen; }
    void setBaseLen(const TDuration& d) { _baseLen = d; }

    void dump() const override;

    void setDirection(Direction d) { _direction = d; }
    Direction direction() const { return _direction; }
    bool isUp() const { return _isUp; }
    bool isSmall() const { return _isSmall; }
    Fraction tick() const override { return _tick; }
    Fraction rtick() const override;
    void setTick(const Fraction& v) { _tick = v; }
    Fraction elementsDuration();
    void sortElements();
    bool cross() const;

    void setVisible(bool f) override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue& v) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;

    Shape shape() const override;

    EngravingItem::EditBehavior normalModeEditBehavior() const override { return EngravingItem::EditBehavior::Edit; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    void sanitizeTuplet();
    void addMissingElements();
};
}     // namespace Ms
#endif
