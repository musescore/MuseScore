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

#ifndef __BREATH_H__
#define __BREATH_H__

#include "engravingitem.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   BreathType
//---------------------------------------------------------

struct BreathType {
    SymId id;
    bool isCaesura;
    double pause;
};

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Breath)
    DECLARE_CLASSOF(ElementType::BREATH)

    double _pause;
    SymId _symId;

    friend class Factory;
    Breath(Segment* parent);

    bool sameVoiceKerningLimited() const override { return true; }

public:

    Breath* clone() const override { return new Breath(*this); }

    double mag() const override;

    void setSymId(SymId id) { _symId = id; }
    SymId symId() const { return _symId; }
    double pause() const { return _pause; }
    void setPause(double v) { _pause = v; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;

    mu::PointF pagePos() const override;        ///< position in page coordinates

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    bool isCaesura() const;

    static const std::vector<BreathType> breathList;

protected:
    void added() override;
    void removed() override;
};
} // namespace mu::engraving
#endif
