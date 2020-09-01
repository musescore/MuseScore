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

#include <QPainter>
#include <QGuiApplication>

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/page.h"

#include "notationinteraction.h"
#include "notationplayback.h"
#include "notationundostackcontroller.h"
#include "notationstyle.h"
#include "notationelements.h"
#include "notationaccessibility.h"
#include "notationmidiinput.h"
#include "notationparts.h"

using namespace mu::notation;
using namespace Ms;

Notation::Notation(Score* score)
{
    m_scoreGlobal = new MScore(); //! TODO May be static?

    m_interaction = new NotationInteraction(this);
    m_midiInput = new NotationMidiInput(this);
    m_accessibility = new NotationAccessibility(this, m_interaction->selectionChanged());
    m_parts = new NotationParts(this, m_interaction->selectionChanged());
    m_undoStackController = new NotationUndoStackController(this);
    m_style = new NotationStyle(this);
    m_playback = new NotationPlayback(this);
    m_elements = new NotationElements(this);

    m_interaction->noteAdded().onNotify(this, [this]() { notifyAboutNotationChanged(); });
    m_interaction->dragChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });
    m_interaction->textEditingChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });
    m_interaction->dropChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });

    m_midiInput->noteChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });
    m_style->styleChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });
    m_parts->partsChanged().onNotify(this, [this]() { notifyAboutNotationChanged(); });

    if (score) {
        setScore(score);
    }
}

Notation::~Notation()
{
    delete m_interaction;
    delete m_midiInput;
    delete m_accessibility;
    delete m_undoStackController;
    delete m_style;
    delete m_playback;
    delete m_elements;

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

Meta Notation::metaInfo() const
{
    Meta meta;

    meta.title = score()->title();
    meta.subtitle = score()->metaTag("subtitle");
    meta.composer = score()->metaTag("composer");
    meta.lyricist = score()->metaTag("lyricist");
    meta.copyright = score()->metaTag("copyright");
    meta.translator = score()->metaTag("translator");
    meta.arranger = score()->metaTag("arranger");
    meta.creationDate = QDate::fromString(score()->metaTag("creationDate"), Qt::ISODate);

    return meta;
}

void Notation::setViewSize(const QSizeF& vs)
{
    m_viewSize = vs;
}

QRectF Notation::previewRect() const
{
    const QList<Ms::Page*>& pages = m_score->pages();

    if (pages.isEmpty()) {
        return QRect();
    }

    return pages.first()->bbox();
}

void Notation::paint(QPainter* painter)
{
    const QList<Ms::Page*>& mspages = m_score->pages();

    if (mspages.isEmpty()) {
        painter->drawText(10, 10, "no pages");
        return;
    }

    Ms::Page* page = mspages.first();

    page->draw(painter);

    painter->fillRect(page->bbox(), QColor("#ffffff"));

    for (Ms::Element* element : page->elements()) {
        if (!element->visible()) {
            continue;
        }

        element->itemDiscovered = false;
        QPointF pos(element->pagePos());

        painter->translate(pos);

        element->draw(painter);

        painter->translate(-pos);
    }

    m_interaction->paint(painter);
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

INotationInteraction* Notation::interaction() const
{
    return m_interaction;
}

INotationMidiInput* Notation::midiInput() const
{
    return m_midiInput;
}

INotationUndoStack* Notation::undoStack() const
{
    return m_undoStackController;
}

INotationElements* Notation::elements() const
{
    return m_elements;
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

INotationParts* Notation::parts() const
{
    return m_parts;
}

Ms::Score* Notation::score() const
{
    return m_score;
}

QSizeF Notation::viewSize() const
{
    return m_viewSize;
}
