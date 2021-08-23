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
#include "notation.h"

#include <QGuiApplication>
#include <QScreen>

#include "log.h"

#include "libmscore/masterscore.h"
#include "libmscore/scorefont.h"
#include "libmscore/page.h"
#include "libmscore/rendermidi.h"
#include "engraving/accessibility/accessibleelement.h"

#include "notationinteraction.h"
#include "masternotationmididata.h"
#include "notationplayback.h"
#include "notationundostack.h"
#include "notationstyle.h"
#include "notationelements.h"
#include "notationaccessibility.h"
#include "notationmidiinput.h"
#include "notationparts.h"
#include "notationtypes.h"
#include "draw/pen.h"

using namespace mu::notation;

Notation::Notation(Ms::Score* score)
{
    m_scoreGlobal = new Ms::MScore(); //! TODO May be static?
    m_opened.val = false;

    m_undoStack = std::make_shared<NotationUndoStack>(this, m_notationChanged);
    m_interaction = std::make_shared<NotationInteraction>(this, m_undoStack);
    m_midiInput = std::make_shared<NotationMidiInput>(this, m_undoStack);
    m_accessibility = std::make_shared<NotationAccessibility>(this, m_interaction->selectionChanged());
    m_parts = std::make_shared<NotationParts>(this, m_interaction, m_undoStack);
    m_playback = std::make_shared<NotationPlayback>(this, m_notationChanged);
    m_style = std::make_shared<NotationStyle>(this);
    m_elements = std::make_shared<NotationElements>(this);

    m_interaction->noteInput()->noteAdded().onNotify(this, [this]() {
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

    engravingConfiguration()->selectionColorChanged().onReceive(this, [this](int, const mu::draw::Color&) {
        notifyAboutNotationChanged();
    });

    configuration()->canvasOrientation().ch.onReceive(this, [this](framework::Orientation) {
        m_score->doLayout();
        for (Ms::Score* score : m_score->scoreList()) {
            score->doLayout();
        }
    });

    setScore(score);
}

Notation::~Notation()
{
    //! Note Dereference internal pointers before the deallocation of Ms::Score* in order to prevent access to dereferenced object
    //! Makes sense to use std::shared_ptr<Ms::Score*> ubiquitous instead of the raw pointers
    m_parts = nullptr;
    m_playback = nullptr;
    m_undoStack = nullptr;
    m_interaction = nullptr;
    m_midiInput = nullptr;
    m_accessibility = nullptr;
    m_style = nullptr;
    m_elements = nullptr;

    delete m_score;
}

void Notation::init()
{
    Ms::MScore::pixelRatio = Ms::DPI / QGuiApplication::primaryScreen()->logicalDotsPerInch();

    bool isVertical = configuration()->canvasOrientation().val == framework::Orientation::Vertical;
    Ms::MScore::setVerticalOrientation(isVertical);

    Ms::MScore::panPlayback = configuration()->isAutomaticallyPanEnabled();
    Ms::MScore::playRepeats = configuration()->isPlayRepeatsEnabled();
}

void Notation::setScore(Ms::Score* score)
{
    m_score = score;

    if (score) {
        static_cast<NotationInteraction*>(m_interaction.get())->init();
        static_cast<NotationPlayback*>(m_playback.get())->init();
    }
}

Ms::MScore* Notation::scoreGlobal() const
{
    return m_scoreGlobal;
}

QString Notation::title() const
{
    return m_score ? m_score->title() : QString();
}

mu::notation::ScoreOrder Notation::scoreOrder() const
{
    return m_score ? m_score->scoreOrder() : ScoreOrder();
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

void Notation::paint(mu::draw::Painter* painter, const RectF& frameRect)
{
    const QList<Ms::Page*>& pages = score()->pages();
    if (pages.empty()) {
        return;
    }

    switch (score()->layoutMode()) {
    case engraving::LayoutMode::LINE:
    case engraving::LayoutMode::SYSTEM: {
        bool paintBorders = false;
        paintPages(painter, frameRect, { pages.first() }, paintBorders);
        break;
    }
    case engraving::LayoutMode::FLOAT:
    case engraving::LayoutMode::PAGE: {
        bool paintBorders = !score()->printing();
        paintPages(painter, frameRect, pages, paintBorders);
    }
    }

    static_cast<NotationInteraction*>(m_interaction.get())->paint(painter);
}

void Notation::paintPages(draw::Painter* painter, const RectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const
{
    for (Ms::Page* page : pages) {
        RectF pageRect(page->abbox().translated(page->pos()));

        if (pageRect.right() < frameRect.left()) {
            continue;
        }

        if (pageRect.left() > frameRect.right()) {
            break;
        }

        if (paintBorders) {
            paintPageBorder(painter, page);
        }

        PointF pagePosition(page->pos());
        painter->translate(pagePosition);
        paintForeground(painter, page->bbox());
        painter->setClipping(true);
        painter->setClipRect(page->bbox());

        QList<Element*> elements = page->items(frameRect.translated(-page->pos()));
        Ms::paintElements(*painter, elements);

        painter->translate(-pagePosition);
        painter->setClipping(false);
    }
}

void Notation::paintPageBorder(draw::Painter* painter, const Ms::Page* page) const
{
    using namespace mu::draw;
    RectF boundingRect(page->canvasBoundingRect());

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(Pen(configuration()->borderColor(), configuration()->borderWidth()));
    painter->drawRect(boundingRect);

    if (!score()->showPageborders()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(engravingConfiguration()->formattingMarksColor());
    boundingRect.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
    painter->drawRect(boundingRect);

    if (!page->isOdd()) {
        painter->drawLine(boundingRect.right(), 0.0, boundingRect.right(), boundingRect.bottom());
    }
}

void Notation::paintForeground(mu::draw::Painter* painter, const RectF& pageRect) const
{
    if (score()->printing()) {
        painter->fillRect(pageRect, mu::draw::Color::white);
        return;
    }

    QString wallpaperPath = configuration()->foregroundWallpaperPath().toQString();

    if (configuration()->foregroundUseColor() || wallpaperPath.isEmpty()) {
        painter->fillRect(pageRect, configuration()->foregroundColor());
    } else {
        QPixmap pixmap(wallpaperPath);
        painter->drawTiledPixmap(pageRect, pixmap);
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
