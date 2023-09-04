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

#ifndef __MSCOREVIEW_H__
#define __MSCOREVIEW_H__

#include <list>

#include "draw/painter.h"

namespace mu::engraving {
class EngravingItem;
class Page;
class Score;

//---------------------------------------------------------
//   MuseScoreView
//---------------------------------------------------------

class MuseScoreView
{
public:
    virtual ~MuseScoreView() = default;

    virtual double selectionProximity() const { return 0.0f; }

    virtual void layoutChanged() {}
    virtual void dataChanged(const mu::RectF&) = 0;
    virtual void updateAll() = 0;

    virtual void moveCursor() {}

    virtual Score* score() const { return m_score; }
    virtual void setScore(Score* s) { m_score = s; }
    virtual void removeScore() {}

    virtual void changeEditElement(EngravingItem*) {}
    virtual void setDropRectangle(const mu::RectF&) {}
    virtual void startNoteEntryMode() {}
    virtual void drawBackground(mu::draw::Painter*, const mu::RectF&) const = 0;
    virtual void setDropTarget(const EngravingItem*) {}

    virtual void textTab(bool /*back*/) {}

    virtual const mu::Rect geometry() const = 0;

    const std::vector<EngravingItem*> elementsAt(const mu::PointF&) const;
    EngravingItem* elementNear(const mu::PointF& pos) const;
    virtual void adjustCanvasPosition(const EngravingItem*, int /*staffIdx*/ = -1) {}

protected:
    Score* m_score = nullptr;

private:
    Page* point2page(const mu::PointF&) const;
    EngravingItem* elementAt(const mu::PointF& p) const;
    const std::vector<EngravingItem*> elementsNear(const mu::PointF& pos) const;
};
} // namespace mu::engraving

#endif
