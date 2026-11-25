/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include "notationpaintview.h"

#include "notation/inotationviewstate.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::notation;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : AbstractNotationPaintView(parent)
{
}

void NotationPaintView::onNotationSetup()
{
    // Clip automation lines to the view bounds
    m_automationLinesContainer = new QQuickItem(this);
    m_automationLinesContainer->setClip(true);
    m_automationLinesContainer->setWidth(width());
    m_automationLinesContainer->setHeight(height());
    connect(this, &QQuickItem::widthChanged, m_automationLinesContainer, [this]() {
        m_automationLinesContainer->setWidth(width());
    });
    connect(this, &QQuickItem::heightChanged, m_automationLinesContainer, [this]() {
        m_automationLinesContainer->setHeight(height());
    });

    m_notationAutomationController = std::make_unique<NotationAutomationController>(m_automationLinesContainer, iocContext());
    m_notationAutomationController->showRectRequested().onReceive(this, [this](const muse::RectF& logicRect) {
        adjustCanvasPosition(logicRect);
    });

    AbstractNotationPaintView::onNotationSetup();
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

    m_notationAutomationController->init();
}

void NotationPaintView::onUnloadNotation(INotationPtr notation)
{
    AbstractNotationPaintView::onUnloadNotation(notation);

    notation->viewState()->matrixChanged().disconnect(this);
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
    if (m_notationAutomationController) {
        m_notationAutomationController->setViewMatrix(newMatrix);
    }

    AbstractNotationPaintView::onMatrixChanged(oldMatrix, newMatrix, overrideZoomType);

    if (!m_isLocalMatrixUpdate && notation()) {
        notation()->viewState()->setMatrix(newMatrix, this);

        if (overrideZoomType) {
            notation()->viewState()->setZoomType(ZoomType::Percentage);
        }
    }
}
