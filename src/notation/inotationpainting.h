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

#include "draw/painter.h"
#include "engraving/rendering/iscorerenderer.h"

namespace mu::notation {
class INotationPainting
{
public:
    virtual ~INotationPainting() = default;

    using Options = engraving::rendering::IScoreRenderer::PaintOptions;

    virtual void setViewMode(const ViewMode& vm) = 0;
    virtual ViewMode viewMode() const = 0;

    virtual int pageCount() const = 0;
    virtual SizeF pageSizeInch() const = 0;
    virtual SizeF pageSizeInch(const Options& opt) const = 0;

    virtual void paintView(draw::Painter* painter, const RectF& frameRect, bool isPrinting) = 0;
    virtual void paintPdf(draw::Painter* painter, const Options& opt) = 0;
    virtual void paintPrint(draw::Painter* painter, const Options& opt) = 0;
    virtual void paintPng(draw::Painter* painter, const Options& opt) = 0;
};

using INotationPaintingPtr = std::shared_ptr<INotationPainting>;
}

#endif // MU_NOTATION_INOTATIONPAINTING_H
