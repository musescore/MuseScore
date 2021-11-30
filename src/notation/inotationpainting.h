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
#ifndef MU_NOTATION_INOTATIONPAINTING_H
#define MU_NOTATION_INOTATIONPAINTING_H

#include <memory>
#include "notationtypes.h"

namespace mu::notation {
class INotationPainting
{
public:
    virtual ~INotationPainting() = default;

    virtual void setViewMode(const ViewMode& vm) = 0;
    virtual ViewMode viewMode() const = 0;
    virtual void paint(mu::draw::Painter* painter, const RectF& frameRect) = 0;
};

using INotationPaintingPtr = std::shared_ptr<INotationPainting>;
}

#endif // MU_NOTATION_INOTATIONPAINTING_H
