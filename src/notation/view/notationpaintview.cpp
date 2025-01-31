/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "notationpaintview.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::notation;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : AbstractNotationPaintView(parent)
{
}

void NotationPaintView::onLoadNotation(INotationPtr notation)
{
    m_isLocalMatrixUpdate = true;
    setMatrix(notation->viewState()->matrix());
    m_isLocalMatrixUpdate = false;

    notation->viewState()->matrixChanged().onReceive(this, [this](const Transform& matrix, NotationPaintView* sender) {
        if (sender != this) {
            m_isLocalMatrixUpdate = true;
            setMatrix(matrix);
            m_isLocalMatrixUpdate = false;
        }
    });

    AbstractNotationPaintView::onLoadNotation(notation);
}

void NotationPaintView::onUnloadNotation(INotationPtr notation)
{
    AbstractNotationPaintView::onUnloadNotation(notation);

    notation->viewState()->matrixChanged().resetOnReceive(this);
}

void NotationPaintView::initZoomAndPosition()
{
    if (notation() && !notation()->viewState()->isMatrixInited()) {
        inputController()->initZoom();
        inputController()->initCanvasPos();
    }
}

void NotationPaintView::onMatrixChanged(const Transform& oldMatrix, const Transform& newMatrix, bool overrideZoomType)
{
    AbstractNotationPaintView::onMatrixChanged(oldMatrix, newMatrix, overrideZoomType);

    if (!m_isLocalMatrixUpdate && notation()) {
        notation()->viewState()->setMatrix(newMatrix, this);

        if (overrideZoomType) {
            notation()->viewState()->setZoomType(ZoomType::Percentage);
        }
    }
}
