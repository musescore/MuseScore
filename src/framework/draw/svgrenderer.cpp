/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <algorithm>
#include <cmath>

#ifndef DRAW_NO_QSVGRENDER
#include <QPainter>
#include <QImage>
#include <QCache>
#include <QSvgRenderer>

#include "internal/qpainterprovider.h"
#endif

#include "log.h"

using namespace muse::draw;
#ifndef DRAW_NO_QSVGRENDER
static QCache<const SvgRenderer*, QImage>& drawingScreenCache()
{
    static QCache<const SvgRenderer*, QImage> cache(64 * 1024); // KiB, across all pages
    return cache;
}

#endif

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
    drawingScreenCache().remove(this);
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

void SvgRenderer::renderCached(Painter* painter, const RectF& rect)
{
#ifndef DRAW_NO_QSVGRENDER
    auto provider = std::dynamic_pointer_cast<QPainterProvider>(painter->provider());
    if (!provider || !rect.isValid()) {
        render(painter, rect);
        return;
    }
    QPainter* target = provider->qpainter();
    const auto transform = target->deviceTransform();
    const double sx = std::hypot(transform.m11(), transform.m12());
    const double sy = std::hypot(transform.m21(), transform.m22());
    // One bounded screen texture per drawing fragment. Printing remains vector.
    const double limit = std::min(1.0, 2048.0 / std::max(1.0, std::max(rect.width() * sx, rect.height() * sy)));
    const QSize size(std::max(1, int(std::ceil(rect.width() * sx * limit))),
                     std::max(1, int(std::ceil(rect.height() * sy * limit))));
    QImage* image = drawingScreenCache().object(this);
    if (!image || image->size() != size) {
        image = new QImage(size, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::transparent);
        QPainter buffer(image);
        buffer.setRenderHint(QPainter::Antialiasing);
        m_qSvgRenderer->render(&buffer, QRectF(QPointF(0, 0), QSizeF(size)));
        buffer.end();
        drawingScreenCache().insert(this, image, std::max(1, int(image->sizeInBytes() / 1024)));
    }
    target->drawImage(rect.toQRectF(), *image);
#else
    render(painter, rect);
#endif
}
