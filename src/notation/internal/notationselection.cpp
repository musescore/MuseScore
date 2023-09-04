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

#include <QMimeData>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"

#include "notationselectionrange.h"
#include "notationerrors.h"

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

mu::Ret NotationSelection::canCopy() const
{
    if (isNone()) {
        return make_ret(Err::EmptySelection);
    }

    if (!score()->selection().canCopy()) {
        return make_ret(Err::SelectCompleteTupletOrTremolo);
    }

    return make_ok();
}

QMimeData* NotationSelection::mimeData() const
{
    QString mimeType = score()->selection().mimeType();
    if (mimeType.isEmpty()) {
        return nullptr;
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(mimeType, score()->selection().mimeData().toQByteArray());

    return mimeData;
}

EngravingItem* NotationSelection::element() const
{
    return score()->selection().element();
}

std::vector<EngravingItem*> NotationSelection::elements() const
{
    std::vector<EngravingItem*> els;
    std::vector<mu::engraving::EngravingItem*> list = score()->selection().elements();
    els.reserve(list.size());
    for (mu::engraving::EngravingItem* e : list) {
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

    if (const mu::engraving::EngravingItem* element = score()->selection().element()) {
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

mu::engraving::Score* NotationSelection::score() const
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
