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
#ifndef MU_ENGRAVING_EXPRESSION_H
#define MU_ENGRAVING_EXPRESSION_H

#include "textbase.h"

namespace mu::engraving {
class Dynamic;

class Expression final : public TextBase
{
    M_PROPERTY(bool, snapToDynamics, setSnapToDynamics)
    DECLARE_CLASSOF(ElementType::EXPRESSION)

public:
    Expression(Segment* parent);
    Expression(const Expression& expression);
    Expression* clone() const override { return new Expression(*this); }

    Segment* segment() const { return toSegment(explicitParent()); }

    PropertyValue propertyDefault(Pid id) const override;

    double computeDynamicExpressionDistance() const;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;

    bool acceptDrop(EditData& ed) const override;
    EngravingItem* drop(EditData& ed) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    void mapPropertiesFromOldExpressions(StaffText* staffText);

    Dynamic* snappedDynamic() const { return _snappedDynamic; }

private:
    friend class layout::v0::TLayout;

    Dynamic* _snappedDynamic = nullptr;
};
} // namespace mu::engraving
#endif // MU_ENGRAVING_EXPRESSION_H
