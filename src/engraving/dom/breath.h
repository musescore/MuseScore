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

#ifndef MU_ENGRAVING_BREATH_H
#define MU_ENGRAVING_BREATH_H

#include "engravingitem.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   BreathType
//---------------------------------------------------------

struct BreathType {
    SymId id = SymId::noSym;
    bool isCaesura = false;
    double pause = false;
};

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Breath)
    DECLARE_CLASSOF(ElementType::BREATH)

public:

    Breath* clone() const override { return new Breath(*this); }

    double mag() const override;

    void setSymId(SymId id) { m_symId = id; }
    SymId symId() const { return m_symId; }
    double pause() const { return m_pause; }
    void setPause(double v) { m_pause = v; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    PointF pagePos() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    int subtype() const override { return int(m_symId); }
    TranslatableString subtypeUserName() const override;

    bool isCaesura() const;

    static const std::vector<BreathType> BREATH_LIST;

protected:
    void added() override;
    void removed() override;

private:

    friend class Factory;
    Breath(Segment* parent);

    double m_pause = 0.0;
    SymId m_symId = SymId::breathMarkComma;
};
} // namespace mu::engraving
#endif
