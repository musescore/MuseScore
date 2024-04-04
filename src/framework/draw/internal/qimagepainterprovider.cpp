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

#include "qimagepainterprovider.h"

#include <QPainter>

#include "log.h"

namespace muse::draw {
QImagePainterProvider::QImagePainterProvider(std::shared_ptr<Pixmap> px)
    : QPainterProvider(new QPainter()), m_px(px)
{
    m_image = Pixmap::toQPixmap(*px.get()).toImage();
    m_painter->begin(&m_image);
}

QImagePainterProvider::~QImagePainterProvider()
{
    delete m_painter;
}

bool QImagePainterProvider::endTarget(bool endDraw)
{
    UNUSED(endDraw)
    * m_px = Pixmap::fromQPixmap(QPixmap::fromImage(m_image));
    return true;
}

IPaintProviderPtr QImagePainterProvider::make(std::shared_ptr<Pixmap> px)
{
    return std::make_shared<QImagePainterProvider>(px);
}
}
