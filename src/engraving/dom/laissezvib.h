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

#include "tie.h"

namespace mu::engraving {
class LaissezVibSegment : public TieSegment
{
    OBJECT_ALLOCATOR(engraving, LaissezVibSegment)
    DECLARE_CLASSOF(ElementType::LAISSEZ_VIB_SEGMENT)

public:
    LaissezVibSegment(System* parent);
    LaissezVibSegment(const LaissezVibSegment& s);

    LaissezVibSegment* clone() const override { return new LaissezVibSegment(*this); }

    LaissezVib* laissezVib() const { return (LaissezVib*)spanner(); }
    int subtype() const override { return static_cast<int>(spanner()->type()); }

    int gripsCount() const override { return 0; }
    void editDrag(EditData&) override;

    struct LayoutData : public TieSegment::LayoutData {
        SymId symbol = SymId::noSym;
        ld_field<PointF> posRelativeToNote = { "[LaissezVibSegment] posRelativeToNote", PointF() };
    };
    DECLARE_LAYOUTDATA_METHODS(LaissezVibSegment)
private:
    String formatBarsAndBeats() const override;
};

class LaissezVib : public Tie
{
    OBJECT_ALLOCATOR(engraving, LaissezVib)
    DECLARE_CLASSOF(ElementType::LAISSEZ_VIB);

public:
    LaissezVib(Note* parent);

    LaissezVib* clone() const override { return new LaissezVib(*this); }

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    Spatium minLength() const { return m_minLength; }
    void setMinLength(Spatium val) { m_minLength = val; }

    SymId symId() const;

    SlurTieSegment* newSlurTieSegment(System* parent) override { return new LaissezVibSegment(parent); }

    void setEndNote(Note* note) override;
    void setEndElement(EngravingItem*) override;

    LaissezVibSegment* frontSegment() { return toLaissezVibSegment(Spanner::frontSegment()); }
    const LaissezVibSegment* frontSegment() const { return toLaissezVibSegment(Spanner::frontSegment()); }
    LaissezVibSegment* backSegment() { return toLaissezVibSegment(Spanner::backSegment()); }
    const LaissezVibSegment* backSegment() const { return toLaissezVibSegment(Spanner::backSegment()); }
    LaissezVibSegment* segmentAt(int n) { return toLaissezVibSegment(Spanner::segmentAt(n)); }
    const LaissezVibSegment* segmentAt(int n) const { return toLaissezVibSegment(Spanner::segmentAt(n)); }
private:
    Spatium m_minLength = Spatium(2.0);
};
}
