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
#include "sig.h"

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

    String _numeratorString;       // calculated from actualSig() if !customText
    String _denominatorString;

    SymIdList ns;
    SymIdList ds;

    mu::PointF pz;
    mu::PointF pn;
    mu::PointF pointLargeLeftParen;
    mu::PointF pointLargeRightParen;
    Fraction _sig;
    Fraction _stretch;        // localSig / globalSig
    Groups _groups;

    mu::ScaleF _scale;
    TimeSigType _timeSigType;
    bool _showCourtesySig;
    bool _largeParentheses;

    friend class Factory;
    TimeSig(Segment* parent = 0);

    bool neverKernable() const override { return true; }

public:

    void setParent(Segment* parent);

    String ssig() const;
    void setSSig(const String&);

    TimeSig* clone() const override { return new TimeSig(*this); }

    TimeSigType timeSigType() const { return _timeSigType; }

    bool operator==(const TimeSig&) const;
    bool operator!=(const TimeSig& ts) const { return !(*this == ts); }

    double mag() const override;
    void draw(mu::draw::Painter*) const override;
    void write(XmlWriter& xml) const override;
    void layout() override;

    Fraction sig() const { return _sig; }
    void setSig(const Fraction& f, TimeSigType st = TimeSigType::NORMAL);
    int numerator() const { return _sig.numerator(); }
    int denominator() const { return _sig.denominator(); }

    Fraction stretch() const { return _stretch; }
    void setStretch(const Fraction& s) { _stretch = s; }
    int numeratorStretch() const { return _stretch.numerator(); }
    int denominatorStretch() const { return _stretch.denominator(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    bool showCourtesySig() const { return _showCourtesySig; }
    void setShowCourtesySig(bool v) { _showCourtesySig = v; }

    String numeratorString() const { return _numeratorString; }
    void setNumeratorString(const String&);

    String denominatorString() const { return _denominatorString; }
    void setDenominatorString(const String&);

    void setLargeParentheses(bool v) { _largeParentheses = v; }

    void setScale(const mu::ScaleF& s) { _scale = s; }

    void setFrom(const TimeSig*);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    const Groups& groups() const { return _groups; }
    void setGroups(const Groups& e) { _groups = e; }

    Fraction globalSig() const { return (_sig * _stretch).reduced(); }
    void setGlobalSig(const Fraction& f) { _stretch = (_sig / f).reduced(); }

    bool isLocal() const { return _stretch != Fraction(1, 1); }

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

protected:
    void added() override;
    void removed() override;
};
} // namespace mu::engraving
#endif
