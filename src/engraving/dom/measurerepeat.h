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

#ifndef MU_ENGRAVING_MEASUREREPEAT_H
#define MU_ENGRAVING_MEASUREREPEAT_H

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

    void setNumberPos(double d) { m_numberPos = d; }
    double numberPos() const { return m_numberPos; }

    Measure* firstMeasureOfGroup() const;
    const Measure* referringMeasure(const Measure* measure) const;

    Fraction ticks() const override;
    Fraction actualTicks() const { return Rest::ticks(); }

    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue getProperty(Pid) const override;

    String accessibleInfo() const override;

    int subtype() const override { return m_numMeasures; }
    muse::TranslatableString subtypeUserName() const override;

    bool placeMultiple() const override { return numMeasures() == 1; }     // prevent overlapping additions with range selection

    RectF numberRect() const override;

    PointF numberPosition(const RectF& numberBbox) const;

    struct LayoutData : public Rest::LayoutData {
        SymId symId = SymId::noSym;
        SymIdList numberSym;

        void setSymId(SymId id) { symId = id; }

        void setNumberSym(int n) { numberSym = timeSigSymIdsFromString(String::number(n)); }
        void setNumberSym(const String& s) { numberSym = timeSigSymIdsFromString(s); }
        void clearNumberSym() { numberSym.clear(); }

        LineF extenderLineLeft = LineF();
        LineF extenderLineRight = LineF();
    };
    DECLARE_LAYOUTDATA_METHODS(MeasureRepeat)

private:

    Sid getPropertyStyle(Pid) const override;

    int m_numMeasures = 0;
    double m_numberPos = 0.0;
};
} // namespace mu::engraving
#endif
