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
#include "notation.h"

#include <QPointF>
#include <QPainter>
#include <QGuiApplication>

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/page.h"

#include "notationinteraction.h"
#include "notationundostackcontroller.h"
#include "notationstyle.h"
#include "notationaccessibility.h"

using namespace mu::domain::notation;
using namespace Ms;

Notation::Notation(Score *score)
{
    m_scoreGlobal = new MScore(); //! TODO May be static?

    m_interaction = new NotationInteraction(this);
    m_accessibility = new NotationAccessibility(this, m_interaction->selectionChanged());

    m_interaction->noteAdded().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dragChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->textEditingChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dropChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_undoStackController = new NotationUndoStackController(this);
    m_style = new NotationStyle(this);
    m_playback = new NotationPlayback(this);

    if (score) {
        setScore(score);
    }
}

Notation::~Notation()
{
    delete m_score;
}

void Notation::init()
{
    MScore::init(); // initialize libmscore

    MScore::setNudgeStep(.1); // cursor key (default 0.1)
    MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)

    MScore::pixelRatio = DPI / QGuiApplication::primaryScreen()->logicalDotsPerInch();
}

void Notation::setScore(Ms::Score* score)
{
    m_score = score;
    m_interaction->init();
    m_playback->init();
}

MScore* Notation::scoreGlobal() const
{
    return m_scoreGlobal;
}

void Notation::setViewSize(const QSizeF& vs)
{
    m_viewSize = vs;
}

void Notation::paint(QPainter* p, const QRect&)
{
    const QList<Ms::Page*>& pages = m_score->pages();

    if (pages.isEmpty()) {
        p->drawText(10, 10, "no pages");
        return;
    }

    Ms::Page* page = pages.first();
    page->draw(p);

    p->fillRect(page->bbox(), QColor("#ffffff"));

    QList<Ms::Element*> ell = page->elements();
    for (const Ms::Element* e : ell) {
        if (!e->visible()) {
            continue;
        }

        e->itemDiscovered = false;
        QPointF pos(e->pagePos());

        p->translate(pos);

        e->draw(p);

        p->translate(-pos);
    }

    m_interaction->paint(p);
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

INotationInteraction* Notation::interaction() const
{
    return m_interaction;
}

INotationUndoStack* Notation::undoStack() const
{
    return m_undoStackController;
}

INotationStyle* Notation::style() const
{
    return m_style;
}

INotationPlayback* Notation::playback() const
{
    return m_playback;
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

INotationAccessibility* Notation::accessibility() const
{
    return m_accessibility;
}

Ms::Score* Notation::score() const
{
    return m_score;
}

QSizeF Notation::viewSize() const
{
    return m_viewSize;
}
