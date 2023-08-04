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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "engravingitem.h"

#include "groups.h"

namespace mu::engraving {
class Segment;

//---------------------------------------------------------
//   TimeSigType
//---------------------------------------------------------

enum class TimeSigType : char {
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

public:

    void setParent(Segment* parent);

    String ssig() const;
    void setSSig(const String&);

    TimeSig* clone() const override { return new TimeSig(*this); }

    TimeSigType timeSigType() const { return m_timeSigType; }

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

    const mu::ScaleF& scale() const { return m_scale; }
    void setScale(const mu::ScaleF& s) { m_scale = s; }

    void setFrom(const TimeSig*);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    const Groups& groups() const { return m_groups; }
    void setGroups(const Groups& e) { m_groups = e; }

    Fraction globalSig() const { return (m_sig * m_stretch).reduced(); }
    void setGlobalSig(const Fraction& f) { m_stretch = (m_sig / f).reduced(); }

    bool isLocal() const { return m_stretch != Fraction(1, 1); }

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    struct DrawArgs {
        SymIdList ns;
        SymIdList ds;
        mu::PointF pz;
        mu::PointF pn;
        mu::PointF pointLargeLeftParen;
        mu::PointF pointLargeRightParen;
    };

    const DrawArgs& drawArgs() const { return m_drawArgs; }
    void setDrawArgs(const DrawArgs& args) { m_drawArgs = args; }

protected:
    void added() override;
    void removed() override;

private:

    friend class Factory;
    TimeSig(Segment* parent = 0);

    String m_numeratorString;       // calculated from actualSig() if !customText
    String m_denominatorString;

    DrawArgs m_drawArgs;

    Fraction m_sig;
    Fraction m_stretch;        // localSig / globalSig
    Groups m_groups;

    mu::ScaleF m_scale;
    TimeSigType m_timeSigType = TimeSigType::NORMAL;
    bool m_showCourtesySig = false;
    bool m_largeParentheses = false;
};
} // namespace mu::engraving
#endif
