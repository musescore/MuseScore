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

#ifndef MU_ENGRAVING_STEM_H
#define MU_ENGRAVING_STEM_H

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

    Spatium userLength() const { return m_userLength; }
    void setUserLength(Spatium userLength) { m_userLength = userLength; }

    Spatium lineWidth() const { return m_lineWidth; }
    double lineWidthMag() const;
    void setLineWidth(Spatium lineWidth) { m_lineWidth = lineWidth; }

    PointF flagPosition() const;
    double length() const { return m_baseLength + m_userLength.toMM(spatium()); }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    struct LayoutData : public EngravingItem::LayoutData {
        LineF line;
        double beamCorrection = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(Stem)

private:
    friend class Factory;
    Stem(Chord* parent = 0);

    Millimetre m_baseLength = Millimetre(0.0);

    Spatium m_userLength = Spatium(0.0);
    Spatium m_lineWidth = Spatium(0.0);
};
}
#endif
