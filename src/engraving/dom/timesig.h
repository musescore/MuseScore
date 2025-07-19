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

#pragma once

#include "engravingitem.h"

#include "groups.h"

namespace mu::engraving {
class Segment;

//---------------------------------------------------------
//   TimeSigType
//---------------------------------------------------------

enum class TimeSigType : unsigned char {
    NORMAL,              // use sz/sn text
    FOUR_FOUR,           // common time (4/4)
    ALLA_BREVE,          // cut time (2/2)
    CUT_BACH,            // cut time (Bach)
    CUT_TRIPLE,          // cut triple time (9/8)
};

//---------------------------------------------------------------------------------------
//   @@ TimeSig
///    This class represents a time signature.
//---------------------------------------------------------------------------------------

class TimeSig final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TimeSig)
    DECLARE_CLASSOF(ElementType::TIMESIG)

    M_PROPERTY2(bool, isCourtesy, setIsCourtesy, false)

public:

    void setParent(Segment* parent);

    String ssig() const;
    void setSSig(const String&);

    TimeSig* clone() const override { return new TimeSig(*this); }

    TimeSigType timeSigType() const { return m_timeSigType; }

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    bool operator==(const TimeSig&) const;
    bool operator!=(const TimeSig& ts) const { return !(*this == ts); }

    double mag() const override;

    Fraction sig() const { return m_sig; }
    void setSig(const Fraction& f, TimeSigType st = TimeSigType::NORMAL);
    int numerator() const { return m_sig.numerator(); }
    int denominator() const { return m_sig.denominator(); }

    Fraction stretch() const { return m_stretch; }
    void setStretch(const Fraction& s) { m_stretch = s; }
    int numeratorStretch() const { return m_stretch.numerator(); }
    int denominatorStretch() const { return m_stretch.denominator(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    bool showCourtesySig() const { return m_showCourtesySig; }
    void setShowCourtesySig(bool v) { m_showCourtesySig = v; }

    const String& numeratorString() const { return m_numeratorString; }
    void setNumeratorString(const String&);

    const String& denominatorString() const { return m_denominatorString; }
    void setDenominatorString(const String&);

    bool largeParentheses() const { return m_largeParentheses; }
    void setLargeParentheses(bool v) { m_largeParentheses = v; }

    void setFrom(const TimeSig*);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    const Groups& groups() const { return m_groups; }
    void setGroups(const Groups& e) { m_groups = e; }

    bool isLocal() const { return m_stretch != Fraction(1, 1); }

    PointF staffOffset() const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    void initElementStyle(const ElementStyle*) override;
    void styleChanged() override;
    Sid getPropertyStyle(Pid id) const override;

    bool showOnThisStaff() const;
    bool isAboveStaves() const;
    bool isAcrossStaves() const;
    TimeSigPlacement timeSigPlacement() const;
    TimeSigStyle timeSigStyle() const;
    double numDist() const;
    double yPos() const;
    const ScaleF& scale() const { return m_scale; }
    void setScale(const ScaleF& s) { m_scale = s; } // TODO: think about what to do with this

    struct LayoutData : public EngravingItem::LayoutData {
        SymIdList ns;
        SymIdList ds;
        PointF pz;
        PointF pn;
        PointF pointLargeLeftParen;
        PointF pointLargeRightParen;
    };
    DECLARE_LAYOUTDATA_METHODS(TimeSig)

protected:
    void added() override;
    void removed() override;

private:

    friend class Factory;
    TimeSig(Segment* parent = 0);

    String m_numeratorString;       // calculated from actualSig() if !customText
    String m_denominatorString;

    Fraction m_sig;
    Fraction m_stretch;        // localSig / globalSig
    Groups m_groups;

    ScaleF m_scale = ScaleF(1.0, 1.0);
    TimeSigType m_timeSigType = TimeSigType::NORMAL;
    bool m_showCourtesySig = false;
    bool m_largeParentheses = false;
};
} // namespace mu::engraving
