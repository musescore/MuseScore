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
#include "notationselection.h"

#include "libmscore/masterscore.h"
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

SelectionState NotationSelection::state() const
{
    return score()->selection().state();
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

EngravingItem* NotationSelection::element() const
{
    return score()->selection().element();
}

std::vector<EngravingItem*> NotationSelection::elements() const
{
    std::vector<EngravingItem*> els;
    QList<Ms::EngravingItem*> list = score()->selection().elements();
    els.reserve(list.count());
    for (Ms::EngravingItem* e : list) {
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

mu::RectF NotationSelection::canvasBoundingRect() const
{
    if (isNone()) {
        return RectF();
    }

    if (const Ms::EngravingItem* element = score()->selection().element()) {
        return element->canvasBoundingRect();
    }

    RectF result;

    for (const RectF& rect : range()->boundingArea()) {
        result = result.united(rect);
    }

    return result;
}

INotationSelectionRangePtr NotationSelection::range() const
{
    return m_range;
}

Ms::Score* NotationSelection::score() const
{
    return m_getScore->score();
}

void NotationSelection::onElementHit(EngravingItem* el)
{
    m_lastElementHit = el;
}

EngravingItem* NotationSelection::lastElementHit() const
{
    return m_lastElementHit;
}
