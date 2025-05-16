/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_TUPLET_H
#define MU_ENGRAVING_TUPLET_H

#include <set>

#include "durationelement.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
class Text;
class Spanner;

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

    Text* number() const { return m_number; }
    void setNumber(Text* t) { m_number = t; }
    void resetNumberProperty();
    static void resetNumberProperty(Text* number);

    bool isEditable() const override;
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    void setSelected(bool f) override;

    Measure* measure() const override { return toMeasure(explicitParent()); }

    TupletNumberType numberType() const { return m_numberType; }
    TupletBracketType bracketType() const { return m_bracketType; }
    void setNumberType(TupletNumberType val) { m_numberType = val; }
    void setBracketType(TupletBracketType val) { m_bracketType = val; }
    bool hasBracket() const { return m_hasBracket; }
    void setHasBracket(bool b) { m_hasBracket = b; }
    Spatium bracketWidth() const { return m_bracketWidth; }
    void setBracketWidth(Spatium s) { m_bracketWidth = s; }

    const Fraction& ratio() const { return m_ratio; }
    void setRatio(const Fraction& r) { m_ratio = r; }

    void setUserPoint1(PointF p) { m_userP1 = p; }
    void setUserPoint2(PointF p) { m_userP2 = p; }

    const std::vector<DurationElement*>& elements() const { return m_currentElements; }
    void clear() { m_currentElements.clear(); }
    bool contains(const DurationElement* el) const
    {
        return std::find(m_currentElements.begin(), m_currentElements.end(), el) != m_currentElements.end();
    }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void reset() override;

    int id() const { return m_id; }
    void setId(int i) const { m_id = i; }

    TDuration baseLen() const { return m_baseLen; }
    void setBaseLen(const TDuration& d) { m_baseLen = d; }

    void dump() const override;

    void setDirection(DirectionV d) { m_direction = d; }
    DirectionV direction() const { return m_direction; }
    bool isUp() const { return m_isUp; }
    void setIsUp(bool val) { m_isUp = val; }
    bool isSmall() const { return m_isSmall; }
    void setIsSmall(bool val) { m_isSmall = val; }
    Fraction tick() const override { return m_tick; }
    Fraction rtick() const override;
    void setTick(const Fraction& v) { m_tick = v; }
    bool isInRange(const Fraction& startTick, const Fraction& endTick) const;
    Fraction elementsDuration();
    void sortElements();
    bool cross() const;
    staff_idx_t vStaffIdx() const override;

    const PointF& p1() const { return m_p1; }
    PointF& p1() { return m_p1; }
    void setP1(const PointF& p) { m_p1 = p; }
    const PointF& p2() const { return m_p2; }
    PointF& p2() { return m_p2; }
    void setP2(const PointF& p) { m_p2 = p; }

    const PointF& userP1() const { return m_userP1; }
    const PointF& userP2() const { return m_userP2; }

    void setVisible(bool f) override;
    void setColor(const Color& col) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    void sanitizeTuplet();
    void addMissingElements();

    bool calcHasBracket(const DurationElement* cr1, const DurationElement* cr2) const;

    static int computeTupletDenominator(int numerator, Fraction totalDuration);

    PointF bracketL[4];
    PointF bracketR[3];

    EngravingItem* nextElement() override;
    EngravingItem* prevElement() override;

private:

    friend class DurationElement;

    void addDurationElement(DurationElement* de);
    void removeDurationElement(DurationElement* de);

    Fraction addMissingElement(const Fraction& startTick, const Fraction& endTick);

    // All DurationElements where `tuplet()` returns this tuplet
    std::set<DurationElement*> m_allElements;

    // Those DurationElements that are currently really part of this tuplet
    std::vector<DurationElement*> m_currentElements;

    bool m_beingDestructed = false;

    DirectionV m_direction = DirectionV::AUTO;
    TupletNumberType m_numberType = TupletNumberType::SHOW_NUMBER;
    TupletBracketType m_bracketType = TupletBracketType::AUTO_BRACKET;
    Spatium m_bracketWidth;

    bool m_hasBracket = false;
    Fraction m_ratio;
    TDuration m_baseLen;        // 1/8 for a triplet of 1/8

    bool m_isUp = true;
    bool m_isSmall = true;

    Fraction m_tick;

    PointF m_p1, m_p2;
    PointF m_userP1, m_userP2;      // user offset
    mutable int m_id;                   // used during read/write

    Text* m_number = nullptr;
};
} // namespace mu::engraving
#endif
