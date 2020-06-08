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
#include "notationinputcontroller.h"

#include <QRectF>
#include "libmscore/page.h"
#include "libmscore/notedot.h"

using namespace mu::domain::notation;

NotationInputController::NotationInputController(IGetScore* getScore)
    : m_getScore(getScore)
{
}

Ms::Score* NotationInputController::score() const
{
    return m_getScore->score();
}

Element* NotationInputController::hitElement(const QPointF& pos, float width) const
{
    QList<Ms::Element*> ll = hitElements(pos, width);
    if (ll.isEmpty()) {
        return nullptr;
    }
    return ll.first();
}

Ms::Page* NotationInputController::point2page(const QPointF& p) const
{
    if (score()->layoutMode() == Ms::LayoutMode::LINE) {
        return score()->pages().isEmpty() ? 0 : score()->pages().front();
    }
    foreach (Ms::Page* page, score()->pages()) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return nullptr;
}

QList<Ms::Element*> NotationInputController::hitElements(const QPointF& p_in, float w) const
{
    Ms::Page* page = point2page(p_in);
    if (!page) {
        return QList<Ms::Element*>();
    }

    QList<Ms::Element*> ll;

    QPointF p = p_in - page->pos();

    QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

    QList<Ms::Element*> el = page->items(r);
    //! TODO
    //    for (int i = 0; i < MAX_HEADERS; i++)
    //        if (score()->headerText(i) != nullptr)      // gives the ability to select the header
    //            el.push_back(score()->headerText(i));
    //    for (int i = 0; i < MAX_FOOTERS; i++)
    //        if (score()->footerText(i) != nullptr)      // gives the ability to select the footer
    //            el.push_back(score()->footerText(i));
    //! -------

    for (Ms::Element* e : el) {
        e->itemDiscovered = 0;
        if (!e->selectable() || e->isPage()) {
            continue;
        }
        if (e->contains(p)) {
            ll.append(e);
        }
    }

    int n = ll.size();
    if ((n == 0) || ((n == 1) && (ll[0]->isMeasure()))) {
        //
        // if no relevant element hit, look nearby
        //
        for (Ms::Element* e : el) {
            if (e->isPage() || !e->selectable()) {
                continue;
            }
            if (e->intersects(r)) {
                ll.append(e);
            }
        }
    }

    if (!ll.empty()) {
        std::sort(ll.begin(), ll.end(), NotationInputController::elementIsLess);
    }

    return ll;
}

bool NotationInputController::elementIsLess(const Ms::Element* e1, const Ms::Element* e2)
{
    if (!e1->selectable()) {
        return false;
    }
    if (!e2->selectable()) {
        return true;
    }
    if (e1->isNote() && e2->isStem()) {
        return true;
    }
    if (e2->isNote() && e1->isStem()) {
        return false;
    }
    if (e1->z() == e2->z()) {
        // same stacking order, prefer non-hidden elements
        if (e1->type() == e2->type()) {
            if (e1->type() == Ms::ElementType::NOTEDOT) {
                const Ms::NoteDot* n1 = static_cast<const Ms::NoteDot*>(e1);
                const Ms::NoteDot* n2 = static_cast<const Ms::NoteDot*>(e2);
                if (n1->note() && n1->note()->hidden()) {
                    return false;
                } else if (n2->note() && n2->note()->hidden()) {
                    return true;
                }
            } else if (e1->type() == Ms::ElementType::NOTE) {
                const Ms::Note* n1 = static_cast<const Ms::Note*>(e1);
                const Ms::Note* n2 = static_cast<const Ms::Note*>(e2);
                if (n1->hidden()) {
                    return false;
                } else if (n2->hidden()) {
                    return true;
                }
            }
        }
        // different types, or same type but nothing hidden - use track
        return e1->track() <= e2->track();
    }

    // default case, use stacking order
    return e1->z() <= e2->z();
}
