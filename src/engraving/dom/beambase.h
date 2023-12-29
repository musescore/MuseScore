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
#ifndef MU_ENGRAVING_BEAMBASE_H
#define MU_ENGRAVING_BEAMBASE_H

#include "engravingitem.h"

namespace mu::engraving::rendering::dev {
class BeamTremoloLayout;
}

namespace mu::engraving {
class BeamBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BeamBase)
    DECLARE_CLASSOF(ElementType::INVALID) // dummy
public:

    std::shared_ptr<rendering::dev::BeamTremoloLayout> layoutInfo() const { return m_layoutInfo; }
    void setLayoutInfo(std::shared_ptr<rendering::dev::BeamTremoloLayout> info) { m_layoutInfo = info; }

    std::shared_ptr<rendering::dev::BeamTremoloLayout> m_layoutInfo;

protected:
    BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags = ElementFlag::NOTHING);
    BeamBase(const BeamBase&);
};
}

#endif // MU_ENGRAVING_BEAMBASE_H
