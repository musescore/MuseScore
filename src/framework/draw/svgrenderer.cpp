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
#include "svgrenderer.h"

#ifndef DRAW_NO_QSVGRENDER
#include <QPainter>
#include <QSvgRenderer>

#include "internal/qpainterprovider.h"
#endif

#include "log.h"

using namespace muse::draw;

//! NOTE Perhaps in the future we need to add something like ISvgRenderer

SvgRenderer::SvgRenderer(const ByteArray& data)
{
#ifndef DRAW_NO_QSVGRENDER
    m_qSvgRenderer = new QSvgRenderer(data.toQByteArray());
#else
    NOT_SUPPORTED;
    UNUSED(data);
#endif
}

SvgRenderer::~SvgRenderer()
{
#ifndef DRAW_NO_QSVGRENDER
    delete m_qSvgRenderer;
#endif
}

muse::SizeF SvgRenderer::defaultSize() const
{
#ifndef DRAW_NO_QSVGRENDER
    return SizeF::fromQSizeF(m_qSvgRenderer->defaultSize());
#else
    NOT_SUPPORTED;
    return SizeF();
#endif
}

void SvgRenderer::render(Painter* painter, const RectF& rect)
{
#ifndef DRAW_NO_QSVGRENDER
    IPaintProviderPtr paintProvider = painter->provider();
    std::shared_ptr<QPainterProvider> qPaintProvider = std::dynamic_pointer_cast<QPainterProvider>(paintProvider);
    if (qPaintProvider) {
        m_qSvgRenderer->render(qPaintProvider->qpainter(), rect.toQRectF());
    }
#else
    NOT_SUPPORTED;
    UNUSED(painter);
    UNUSED(rect);
#endif
}
