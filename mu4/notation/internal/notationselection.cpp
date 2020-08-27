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
#include "log.h"

#include "libmscore/score.h"

using namespace mu::notation;

NotationSelection::NotationSelection(IGetScore* getScore)
    : m_getScore(getScore)
{
}

Ms::Score* NotationSelection::score() const
{
    return m_getScore->score();
}

bool NotationSelection::isNone() const
{
    return score()->selection().isNone();
}

bool NotationSelection::isRange() const
{
    return score()->selection().isRange();
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
    for (const Ms::Element* el: els) {
        if (rect.isNull()) {
            rect = el->canvasBoundingRect();
        } else {
            rect = rect.united(el->canvasBoundingRect());
        }
    }

    return rect;
}
