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
#ifndef MU_NOTATION_SCORECALLBACKS_H
#define MU_NOTATION_SCORECALLBACKS_H

#include "libmscore/mscoreview.h"

#include "notation/inotationconfiguration.h"
#include "modularity/ioc.h"

namespace mu::notation {
class INotationInteraction;
class IGetScore;
class ScoreCallbacks : public mu::engraving::MuseScoreView
{
    INJECT(INotationConfiguration, configuration)

public:
    ScoreCallbacks() = default;

    void dataChanged(const mu::RectF&) override;
    void updateAll() override;
    void drawBackground(mu::draw::Painter*, const RectF&) const override;
    const mu::Rect geometry() const override;
    qreal selectionProximity() const override;
    void setDropTarget(const mu::engraving::EngravingItem* dropTarget) override;
    void setDropRectangle(const RectF& rect) override;
    void changeEditElement(mu::engraving::EngravingItem* newElement) override;
    void adjustCanvasPosition(const mu::engraving::EngravingItem*, int staffIdx = -1) override;

    void setSelectionProximity(qreal proximity);
    void setNotationInteraction(INotationInteraction* interaction);

private:
    qreal m_selectionProximity = 0.0f;

    INotationInteraction* m_interaction = nullptr;
};
}

#endif // MU_NOTATION_SCORECALLBACKS_H
