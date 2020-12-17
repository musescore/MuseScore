//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationselection.h"

#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"

#include "notationselectionrange.h"

#include "log.h"

using namespace mu::notation;

NotationSelection::NotationSelection(IGetScore* getScore)
    : m_getScore(getScore)
{
    m_range = std::make_shared<NotationSelectionRange>(getScore);
}

bool NotationSelection::isNone() const
{
    return score()->selection().isNone();
}

bool NotationSelection::isRange() const
{
    return score()->selection().isRange();
}

bool NotationSelection::canCopy() const
{
    return score()->selection().canCopy();
}

QMimeData* NotationSelection::mimeData() const
{
    QString mimeType = score()->selection().mimeType();
    if (mimeType.isEmpty()) {
        return nullptr;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mimeType, score()->selection().mimeData());

    return mimeData;
}

Element* NotationSelection::element() const
{
    return score()->selection().element();
}

std::vector<Element*> NotationSelection::elements() const
{
    std::vector<Element*> els;
    QList<Ms::Element*> list = score()->selection().elements();
    els.reserve(list.count());
    for (Ms::Element* e : list) {
        els.push_back(e);
    }
    return els;
}

std::vector<Note*> NotationSelection::notes(NoteFilter filter) const
{
    switch (filter) {
    case NoteFilter::All: return score()->selection().noteList();
    case NoteFilter::WithTie: return score()->cmdTieNoteList(score()->selection(), false);
    case NoteFilter::WithSlur: {
        NOT_IMPLEMENTED;
        return {};
    }
    }

    return {};
}

QRectF NotationSelection::canvasBoundingRect() const
{
    if (isNone()) {
        return QRectF();
    }

    Ms::Element* el = score()->selection().element();
    if (el) {
        return el->canvasBoundingRect();
    }

    QList<Ms::Element*> els = score()->selection().elements();
    if (els.isEmpty()) {
        LOGW() << "selection not none, but no elements";
        return QRectF();
    }

    QRectF rect;
    for (const Ms::Element* elm: els) {
        if (rect.isNull()) {
            rect = elm->canvasBoundingRect();
        } else {
            rect = rect.united(elm->canvasBoundingRect());
        }
    }

    return rect;
}

INotationSelectionRangePtr NotationSelection::range() const
{
    return m_range;
}

Ms::Score* NotationSelection::score() const
{
    return m_getScore->score();
}
