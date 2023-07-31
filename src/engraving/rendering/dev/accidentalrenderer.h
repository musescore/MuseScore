/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_ACCIDENTALRENDERER_H
#define MU_ENGRAVING_ACCIDENTALRENDERER_H

#include "iitemrenderer.h"

#include "libmscore/accidental.h"

namespace mu::engraving::rendering::dev {
class AccidentalRenderer : public IItemRenderer
{
public:
    AccidentalRenderer() = default;

    void layout(EngravingItem* item, LayoutContext& ctx) const override;
    void draw(const EngravingItem* item, draw::Painter* painter) const override;

    static void layoutAccidental(Accidental* item, const LayoutContext& ctx);
    static void layoutAccidental(const Accidental* item, const LayoutContext& ctx, Accidental::LayoutData& data);

    static void draw(const Accidental* item, draw::Painter* painter);
};
}

#endif // MU_ENGRAVING_ACCIDENTALRENDERER_H
