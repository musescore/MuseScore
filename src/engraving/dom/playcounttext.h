/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "textbase.h"

namespace mu::engraving {
class PlayCountText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, PlayCountText)
    DECLARE_CLASSOF(ElementType::PLAY_COUNT_TEXT)

public:
    PlayCountText* clone() const override { return new PlayCountText(*this); }

    BarLine* barline() const { return toBarLine(parent()); }

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void endEdit(EditData&) override;

private:
    friend class Factory;
    PlayCountText(BarLine* parent, TextStyleType tid = TextStyleType::REPEAT_PLAY_COUNT);
};
} // namespace mu::engraving
