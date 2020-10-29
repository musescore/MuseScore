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
#include "notationundostack.h"
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
    m_opened.val = false;

    m_interaction = std::make_shared<NotationInteraction>(this);
    m_playback = std::make_shared<NotationPlayback>(this);
    m_midiInput = std::make_shared<NotationMidiInput>(this);
    m_accessibility = std::make_shared<NotationAccessibility>(this, m_interaction->selectionChanged());
    m_parts = std::make_shared<NotationParts>(this, m_interaction->selectionChanged());
    m_undoStack = std::make_shared<NotationUndoStack>(this);
    m_style = std::make_shared<NotationStyle>(this);
    m_elements = std::make_shared<NotationElements>(this);

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

    m_midiInput->noteChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_style->styleChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_parts->partsChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    setScore(score);
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

    if (score) {
        static_cast<NotationInteraction*>(m_interaction.get())->init();
        static_cast<NotationPlayback*>(m_playback.get())->init();
    }
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

void Notation::setViewMode(const ViewMode& viewMode)
{
    if (!m_score) {
        return;
    }

    score()->setLayoutMode(viewMode);
    score()->doLayout();
    notifyAboutNotationChanged();
}

ViewMode Notation::viewMode() const
{
    if (!m_score) {
        return ViewMode::PAGE;
    }

    return score()->layoutMode();
}

QRectF Notation::previewRect() const
{
    const QList<Ms::Page*>& pages = m_score->pages();

    if (pages.isEmpty()) {
        return QRect();
    }

    return pages.first()->bbox();
}

void Notation::paint(QPainter* painter, const QRectF& frameRect)
{
    const QList<Ms::Page*>& pages = score()->pages();
    if (pages.empty()) {
        return;
    }

    switch (score()->layoutMode()) {
    case LayoutMode::LINE:
    case LayoutMode::SYSTEM: {
        bool paintBorders = false;
        paintPages(painter, frameRect, { pages.first() }, paintBorders);
        break;
    }
    case LayoutMode::FLOAT:
    case LayoutMode::PAGE: {
        bool paintBorders = !score()->printing();
        paintPages(painter, frameRect, pages, paintBorders);
    }
    }

    static_cast<NotationInteraction*>(m_interaction.get())->paint(painter);
}

void Notation::paintPages(QPainter* painter, const QRectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const
{
    for (Ms::Page* page : pages) {
        QRectF pageRect(page->abbox().translated(page->pos()));

        if (pageRect.right() < frameRect.left()) {
            continue;
        }

        if (pageRect.left() > frameRect.right()) {
            break;
        }

        if (paintBorders) {
            paintPageBorder(painter, page);
        }

        QPointF pagePosition(page->pos());
        painter->translate(pagePosition);
        painter->fillRect(page->bbox(), configuration()->pageColor());
        paintElements(painter, page->elements());
        painter->translate(-pagePosition);
    }
}

void Notation::paintPageBorder(QPainter* painter, const Page* page) const
{
    QRectF boundingRect(page->canvasBoundingRect());

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(configuration()->borderColor(), configuration()->borderWidth()));
    painter->drawRect(boundingRect);

    if (!score()->showPageborders()) {
        return;
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(MScore::frameMarginColor);
    boundingRect.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
    painter->drawRect(boundingRect);

    if (!page->isOdd()) {
        painter->drawLine(boundingRect.right(), 0.0, boundingRect.right(), boundingRect.bottom());
    }
}

void Notation::paintElements(QPainter* painter, const QList<Element*>& elements) const
{
    for (const Ms::Element* element : elements) {
        if (!element->visible()) {
            continue;
        }

        element->itemDiscovered = false;
        QPointF elementPosition(element->pagePos());

        painter->translate(elementPosition);
        element->draw(painter);
        painter->translate(-elementPosition);
    }
}

mu::ValCh<bool> Notation::opened() const
{
    return m_opened;
}

void Notation::setOpened(bool opened)
{
    if (m_opened.val == opened) {
        return;
    }

    m_opened.set(opened);
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

INotationInteractionPtr Notation::interaction() const
{
    return m_interaction;
}

INotationMidiInputPtr Notation::midiInput() const
{
    return m_midiInput;
}

INotationUndoStackPtr Notation::undoStack() const
{
    return m_undoStack;
}

INotationElementsPtr Notation::elements() const
{
    return m_elements;
}

INotationStylePtr Notation::style() const
{
    return m_style;
}

INotationPlaybackPtr Notation::playback() const
{
    return m_playback;
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

INotationAccessibilityPtr Notation::accessibility() const
{
    return m_accessibility;
}

INotationPartsPtr Notation::parts() const
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
