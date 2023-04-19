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

#ifndef __STEM_H__
#define __STEM_H__

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

class Stem final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Stem)
    DECLARE_CLASSOF(ElementType::STEM)

public:

    Stem(const Stem&) = default;
    Stem& operator=(const Stem&) = delete;

    Stem* clone() const override { return new Stem(*this); }

    void layout() override;
    void draw(mu::draw::Painter*) const override;
    void spatiumChanged(double oldValue, double newValue) override;
    EngravingItem* elementBase() const override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void reset() override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    staff_idx_t vStaffIdx() const override;

    Chord* chord() const { return toChord(explicitParent()); }
    bool up() const;

    Millimetre baseLength() const { return m_baseLength; }
    void setBaseLength(Millimetre baseLength);

    Millimetre userLength() const { return m_userLength; }
    void setUserLength(Millimetre userLength) { m_userLength = userLength; }

    Millimetre lineWidth() const { return m_lineWidth; }
    double lineWidthMag() const { return m_lineWidth * mag(); }
    void setLineWidth(Millimetre lineWidth) { m_lineWidth = lineWidth; }

    mu::PointF p2() const { return m_line.p2(); }
    mu::PointF flagPosition() const;
    double length() const { return m_baseLength + m_userLength; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

private:
    friend class Factory;
    Stem(Chord* parent = 0);

    mu::LineF m_line;

    Millimetre m_baseLength = Millimetre(0.0);
    Millimetre m_userLength = Millimetre(0.0);

    Millimetre m_lineWidth = Millimetre(0.0);

    bool sameVoiceKerningLimited() const override { return true; }
};
}
#endif
