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

#ifndef MU_ENGRAVING_ORGANPEDALMARK_H
#define MU_ENGRAVING_ORGANPEDALMARK_H

#include "textbase.h"

#include "../types/types.h"

namespace mu::engraving {
class Note;

class OrganPedalMark final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, OrganPedalMark)
    DECLARE_CLASSOF(ElementType::ORGAN_PEDAL_MARK)

public:
    OrganPedalMark(Note* parent);

    OrganPedalMark* clone() const override { return new OrganPedalMark(*this); }

    Note* note() const { return toNote(explicitParent()); }

    bool isEditAllowed(EditData&) const override { return false; }

    SymId symId() const { return m_symId; }
    void setSymId(SymId id) { m_symId = id; }

    TranslatableString typeUserName() const override;
    String accessibleInfo() const override;

    PropertyValue propertyDefault(Pid id) const override;

private:
    SymId m_symId = SymId::noSym;
};
} // namespace mu::engraving
#endif
