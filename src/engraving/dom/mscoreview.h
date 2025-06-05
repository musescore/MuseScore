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

#ifndef MU_ENGRAVING_MSCOREVIEW_H
#define MU_ENGRAVING_MSCOREVIEW_H

#include "../types/types.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class EngravingItem;
class Page;
class Score;

class MuseScoreView
{
public:
    virtual ~MuseScoreView() = default;

    virtual double selectionProximity() const { return 0.0f; }

    virtual void layoutChanged() {}
    virtual void dataChanged(const RectF&) = 0;
    virtual void updateAll() = 0;

    virtual void moveCursor() {}

    virtual Score* score() const { return m_score; }
    virtual void setScore(Score* s) { m_score = s; }
    virtual void removeScore() {}

    virtual void changeEditElement(EngravingItem*) {}
    virtual void setDropRectangle(const RectF&) {}
    virtual void drawBackground(muse::draw::Painter*, const RectF&) const = 0;
    virtual void setDropTarget(EngravingItem*) {}

    virtual void textTab(bool /*back*/) {}

    const std::vector<EngravingItem*> elementsAt(const PointF&) const;
    EngravingItem* elementNear(const PointF& pos) const;
    virtual void adjustCanvasPosition(const EngravingItem*, int /*staffIdx*/ = -1) {}

protected:
    Score* m_score = nullptr;

private:
    Page* point2page(const PointF&) const;
    EngravingItem* elementAt(const PointF& p) const;
    const std::vector<EngravingItem*> elementsNear(const PointF& pos) const;
};
} // namespace mu::engraving

#endif
