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
#include "notationpaintview.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::notation;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : AbstractNotationPaintView(parent)
{
}

void NotationPaintView::onLoadNotation(INotationPtr notation)
{
    m_isLoadingNotation = true;
    setMatrix(notation->viewState()->matrix());
    m_isLoadingNotation = false;

    notation->viewState()->matrixChanged().onReceive(this, [this](const Transform& matrix, NotationPaintView* sender) {
        if (sender != this) {
            setMatrix(matrix);
        }
    });

    AbstractNotationPaintView::onLoadNotation(notation);
}

void NotationPaintView::onUnloadNotation(INotationPtr notation)
{
    AbstractNotationPaintView::onUnloadNotation(notation);

    notation->viewState()->matrixChanged().resetOnReceive(this);
}

void NotationPaintView::onMatrixChanged(const Transform& matrix, bool overrideZoomType)
{
    AbstractNotationPaintView::onMatrixChanged(matrix, overrideZoomType);

    if (!m_isLoadingNotation && notation()) {
        notation()->viewState()->setMatrix(matrix, this);

        if (overrideZoomType) {
            notation()->viewState()->setZoomType(ZoomType::Percentage);
        }
    }
}
