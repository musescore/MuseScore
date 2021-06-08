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

#include <QList>
#include <QCursor>

#include "draw/painter.h"

namespace Ms {
class Element;
class Score;
class Slur;
class Note;
class Page;
class ChordRest;

enum class Grip : int;
enum class HairpinType : signed char;

//---------------------------------------------------------
//   MuseScoreView
//---------------------------------------------------------

class MuseScoreView
{
protected:
    Score* _score;

public:
    virtual ~MuseScoreView() = default;
    Page* point2page(const mu::PointF&);
    Element* elementAt(const mu::PointF& p);
    const QList<Element*> elementsAt(const mu::PointF&);
    virtual Element* elementNear(mu::PointF) { return 0; }

    virtual void layoutChanged() {}
    virtual void dataChanged(const mu::RectF&) = 0;
    virtual void updateAll() = 0;

    virtual void moveCursor() {}
    virtual void showLoopCursors(bool) {}

    virtual void adjustCanvasPosition(const Element*, bool /*playBack*/, int /*staffIdx*/ = -1) {}
    virtual void setScore(Score* s) { _score = s; }
    Score* score() const { return _score; }
    virtual void removeScore() {}

    virtual void changeEditElement(Element*) {}
    virtual QCursor cursor() const { return QCursor(); }
    virtual void setCursor(const QCursor&) {}
    virtual void setDropRectangle(const mu::RectF&) {}
    virtual void addSlur(ChordRest*, ChordRest*, const Slur* /* slurTemplate */) {}
    virtual void startEdit(Element*, Grip /*startGrip*/) {}
    virtual void startNoteEntryMode() {}
    virtual void drawBackground(mu::draw::Painter*, const mu::RectF&) const = 0;
    virtual void setDropTarget(const Element*) {}

    virtual void textTab(bool /*back*/) {}
    virtual void lyricsTab(bool /*back*/, bool /*end*/, bool /*moveOnly*/) {}
    virtual void lyricsReturn() {}
    virtual void lyricsEndEdit() {}
    virtual void lyricsUpDown(bool /*up*/, bool /*end*/) {}
    virtual void lyricsMinus() {}
    virtual void lyricsUnderscore() {}

    virtual void onElementDestruction(Element*) {}

    virtual const mu::Rect geometry() const = 0;
};
}     // namespace Ms

#endif
