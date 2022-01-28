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
}

namespace Ms {
//---------------------------------------------------------
//   BreathType
//---------------------------------------------------------

struct BreathType {
    SymId id;
    bool isCaesura;
    qreal pause;
};

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath final : public EngravingItem
{
    qreal _pause;
    SymId _symId;

    friend class mu::engraving::Factory;
    Breath(Segment* parent);

public:

    Breath* clone() const override { return new Breath(*this); }

    qreal mag() const override;

    void setSymId(SymId id) { _symId = id; }
    SymId symId() const { return _symId; }
    qreal pause() const { return _pause; }
    void setPause(qreal v) { _pause = v; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;
    mu::PointF pagePos() const override;        ///< position in page coordinates

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    QString accessibleInfo() const override;

    bool isCaesura() const;

    static const std::vector<BreathType> breathList;

protected:
    void added() override;
    void removed() override;
};
}     // namespace Ms
#endif
