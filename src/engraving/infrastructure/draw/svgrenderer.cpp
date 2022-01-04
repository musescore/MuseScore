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

#ifndef NO_ENGRAVING_QSVGRENDER
#include <QPainter>
#include <QSvgRenderer>

#include "internal/qpainterprovider.h"
#endif

#include "log.h"

using namespace mu::draw;

//! NOTE Perhaps in the future we need to add something like ISvgRenderer

SvgRenderer::SvgRenderer(const QByteArray& data)
{
#ifndef NO_ENGRAVING_QSVGRENDER
    m_qSvgRenderer = new QSvgRenderer(data);
#else
    NOT_SUPPORTED;
    UNUSED(data);
#endif
}

SvgRenderer::~SvgRenderer()
{
#ifndef NO_ENGRAVING_QSVGRENDER
    delete m_qSvgRenderer;
#endif
}

mu::SizeF SvgRenderer::defaultSize() const
{
#ifndef NO_ENGRAVING_QSVGRENDER
    return SizeF::fromQSizeF(m_qSvgRenderer->defaultSize());
#else
    NOT_SUPPORTED;
    return SizeF();
#endif
}

void SvgRenderer::render(Painter* painter, const RectF& rect)
{
#ifndef NO_ENGRAVING_QSVGRENDER
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
