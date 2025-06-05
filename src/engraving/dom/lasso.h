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

namespace mu::engraving {
class Lasso : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Lasso)
    DECLARE_CLASSOF(ElementType::LASSO)

public:
    Lasso(Score*);
    virtual Lasso* clone() const override { return new Lasso(*this); }

    bool isEmpty() const { return ldata()->bbox().isEmpty(); }

    bool isEditable() const override { return true; }
    void editDrag(EditData&) override;
    void endDrag(EditData&) override {}

    int gripsCount() const override { return 8; }
    Grip initialEditModeGrip() const override { return Grip(7); }
    Grip defaultGrip() const override { return Grip(7); }
    std::vector<PointF> gripsPositions(const EditData&) const override;
};
}
