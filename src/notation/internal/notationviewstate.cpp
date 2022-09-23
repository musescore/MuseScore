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
#include "notationviewstate.h"

#include "notation.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::notation;

NotationViewState::NotationViewState(Notation* notation)
{
    notation->openChanged().onNotify(this, [this, notation]() {
        if (!notation->isOpen()) {
            m_isMatrixInited = false;
            setMatrix(Transform(), nullptr);
        }
    });
}

bool NotationViewState::isMatrixInited() const
{
    return m_isMatrixInited;
}

void NotationViewState::setMatrixInited(bool inited)
{
    m_isMatrixInited = inited;
}

Transform NotationViewState::matrix() const
{
    return m_matrix;
}

async::Channel<Transform, NotationPaintView*> NotationViewState::matrixChanged() const
{
    return m_matrixChanged;
}

void NotationViewState::setMatrix(const Transform& matrix, NotationPaintView* sender)
{
    int newZoomPercentage = configuration()->zoomPercentageFromScaling(matrix.m11());
    if (m_matrix == matrix && m_zoomPercentage.val == newZoomPercentage) {
        return;
    }

    m_matrix = matrix;
    m_matrixChanged.send(matrix, sender);
    m_zoomPercentage.set(newZoomPercentage);
}

ValCh<int> NotationViewState::zoomPercentage() const
{
    return m_zoomPercentage;
}

ValCh<ZoomType> NotationViewState::zoomType() const
{
    return m_zoomType;
}

void NotationViewState::setZoomType(ZoomType type)
{
    if (m_zoomType.val != type) {
        m_zoomType.set(type);
    }
}
