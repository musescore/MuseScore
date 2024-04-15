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

#ifndef MU_ENGRAVING_HOOK_H
#define MU_ENGRAVING_HOOK_H

#include "symbol.h"

#include "global/allocator.h"

namespace mu::engraving {
class Chord;

class Hook final : public Symbol
{
    OBJECT_ALLOCATOR(engraving, Hook)
    DECLARE_CLASSOF(ElementType::HOOK)

public:
    Hook(Chord* parent = 0);

    Hook* clone() const override { return new Hook(*this); }
    double mag() const override { return parentItem()->mag(); }
    EngravingItem* elementBase() const override;

    void setHookType(int v);
    int hookType() const { return m_hookType; }

    Chord* chord() const { return toChord(explicitParent()); }
    PointF smuflAnchor() const;

    //! @p index: the number of flags (positive: upwards, negative: downwards)
    //! @p straight: whether to use straight flags
    static SymId symIdForHookIndex(int index, bool straight);

private:
    int m_hookType = 0;
};
} // namespace mu::engraving
#endif
