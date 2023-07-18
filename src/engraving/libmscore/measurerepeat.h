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

#ifndef __MEASUREREPEAT_H__
#define __MEASUREREPEAT_H__

#include "rest.h"

#include "utils.h"

namespace mu::engraving {
class Segment;

class MeasureRepeat final : public Rest
{
    OBJECT_ALLOCATOR(engraving, MeasureRepeat)
    DECLARE_CLASSOF(ElementType::MEASURE_REPEAT)

public:
    MeasureRepeat(Segment* parent);
    MeasureRepeat(const MeasureRepeat&) = default;
    MeasureRepeat& operator=(const MeasureRepeat&) = delete;

    MeasureRepeat* clone() const override { return new MeasureRepeat(*this); }
    EngravingItem* linkedClone() override { return EngravingItem::linkedClone(); }

    static constexpr int MAX_NUM_MEASURES = 4;

    void setNumMeasures(int n);
    int numMeasures() const { return m_numMeasures; }
    void setSymId(SymId id) { m_symId = id; }
    SymId symId() const { return m_symId; }
    void setNumberSym(int n) { m_numberSym = timeSigSymIdsFromString(String::number(n)); }
    void setNumberSym(const String& s) { m_numberSym = timeSigSymIdsFromString(s); }
    const SymIdList& numberSym() const { return m_numberSym; }
    void clearNumberSym() { m_numberSym.clear(); }
    void setNumberPos(double d) { m_numberPos = d; }
    double numberPos() const { return m_numberPos; }

    Measure* firstMeasureOfGroup() const;
    const Measure* referringMeasure(const Measure* measure) const;

    void draw(mu::draw::Painter*) const override;

    Fraction ticks() const override;
    Fraction actualTicks() const { return Rest::ticks(); }

    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue getProperty(Pid) const override;

    Shape shape() const override;

    String accessibleInfo() const override;

    bool placeMultiple() const override { return numMeasures() == 1; }     // prevent overlapping additions with range selection

    mu::RectF numberRect() const override;

private:

    Sid getPropertyStyle(Pid) const override;

    mu::PointF numberPosition(const mu::RectF& numberBbox) const;

    int m_numMeasures = 0;
    SymIdList m_numberSym;
    double m_numberPos = 0.0;
    SymId m_symId = SymId::noSym;
};
} // namespace mu::engraving
#endif
