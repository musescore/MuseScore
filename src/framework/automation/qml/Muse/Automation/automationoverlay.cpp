/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "automationoverlay.h"
#include "draw/painter.h"

using namespace muse::automation;

AutomationOverlay::AutomationOverlay(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent)
{
}

void AutomationOverlay::paint(QPainter* qp)
{
    // TODO: correctDrawRect (?)
    muse::draw::Painter mup(qp, objectName().toStdString());
    muse::draw::Painter* painter = &mup;

    // TODO: placeholder - draw a rect that moves with the canvas
    const QRectF r = m_viewMatrix.mapRect(QRectF(0, 0, 160, 160));
    const muse::Color red(Qt::red);
    painter->fillRect(muse::RectF::fromQRectF(r), red);
}

QVariant AutomationOverlay::viewMatrix() const
{
    return m_viewMatrix;
}

void AutomationOverlay::setViewMatrix(const QVariant& matrix)
{
    if (m_viewMatrix == matrix) {
        return;
    }
    m_viewMatrix = matrix.value<QTransform>();
    emit viewMatrixChanged();
    update(); // TODO: pass in an update rect
}
