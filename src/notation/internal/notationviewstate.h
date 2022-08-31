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
#ifndef MU_NOTATION_NOTATIONVIEWSTATE_H
#define MU_NOTATION_NOTATIONVIEWSTATE_H

#include "../inotationviewstate.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "../inotationconfiguration.h"

namespace mu::notation {
class Notation;
class NotationViewState : public INotationViewState, public async::Asyncable
{
    INJECT_STATIC(notation, INotationConfiguration, configuration)

public:
    explicit NotationViewState(Notation* notation);

    bool isMatrixInited() const override;
    void setMatrixInited(bool inited) override;

    draw::Transform matrix() const override;
    async::Channel<draw::Transform, NotationPaintView*> matrixChanged() const override;
    void setMatrix(const draw::Transform& matrix, NotationPaintView* sender) override;

    ValCh<int> zoomPercentage() const override;

    ValCh<ZoomType> zoomType() const override;
    void setZoomType(ZoomType type) override;

private:
    bool m_isMatrixInited = false;
    draw::Transform m_matrix;
    async::Channel<draw::Transform, NotationPaintView*> m_matrixChanged;
    ValCh<int> m_zoomPercentage;
    ValCh<ZoomType> m_zoomType;
};
}

#endif // MU_NOTATION_NOTATIONVIEWSTATE_H
