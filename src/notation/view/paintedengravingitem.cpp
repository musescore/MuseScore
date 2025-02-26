/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include <QPainter>
#include "paintedengravingitem.h"

#include "notation/utilities/engravingitempreviewpainter.h"

using namespace mu::notation;

PaintedEngravingItem::PaintedEngravingItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

QVariant PaintedEngravingItem::engravingItemVariant() const
{
    return QVariant::fromValue(m_item);
}

void PaintedEngravingItem::setEngravingItemVariant(QVariant engravingItemVariant)
{
    mu::engraving::ElementPtr item = engravingItemVariant.value<mu::engraving::ElementPtr>();
    if (item == m_item) {
        return;
    }
    m_item = item;
    emit engravingItemVariantChanged();
}

double PaintedEngravingItem::spatium() const
{
    return m_spatium;
}

void PaintedEngravingItem::setSpatium(double spatium)
{
    if (spatium == m_spatium) {
        return;
    }
    m_spatium = spatium;
    emit spatiumChanged();
}

void PaintedEngravingItem::paint(QPainter* painter)
{
    qreal dpi = painter->device()->logicalDpiX();
    muse::draw::Painter p(painter, "paintedengravingitem");
    p.save();
    p.setAntialiasing(true);
    paintNotationPreview(p, dpi);
    p.restore();
}

void PaintedEngravingItem::paintNotationPreview(muse::draw::Painter& painter, qreal dpi) const
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    EngravingItemPreviewPainter::PaintParams params;
    params.painter = &painter;

    params.color = configuration()->defaultColor();

    params.rect = muse::RectF(0, 0, parentItem()->width(), parentItem()->height());
    params.dpi = dpi;

    params.spatium = m_spatium;

    params.drawStaff = true;

    painter.fillRect(params.rect, configuration()->noteBackgroundColor());

    EngravingItemPreviewPainter::paintPreview(m_item.get(), params);
}
