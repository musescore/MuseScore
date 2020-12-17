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
#include <QScreen>

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

static const QString WORK_TITLE_TAG("workTitle");
static const QString WORK_NUMBER_TAG("workNumber");
static const QString SUBTITLE_TAG("subtitle");
static const QString COMPOSER_TAG("composer");
static const QString LYRICIST_TAG("lyricist");
static const QString POET_TAG("poet");
static const QString SOURCE_TAG("source");
static const QString COPYRIGHT_TAG("copyright");
static const QString TRANSLATOR_TAG("translator");
static const QString ARRANGER_TAG("arranger");
static const QString CREATION_DATE_TAG("creationDate");
static const QString PLATFORM_TAG("platform");
static const QString MOVEMENT_TITLE_TAG("movementTitle");
static const QString MOVEMENT_NUMBER_TAG("movementNumber");

static bool isStandardTag(const QString& tag)
{
    static const QSet<QString> standardTags {
        WORK_TITLE_TAG,
        WORK_NUMBER_TAG,
        SUBTITLE_TAG,
        COMPOSER_TAG,
        LYRICIST_TAG,
        POET_TAG,
        SOURCE_TAG,
        COPYRIGHT_TAG,
        TRANSLATOR_TAG,
        ARRANGER_TAG,
        CREATION_DATE_TAG,
        PLATFORM_TAG,
        MOVEMENT_NUMBER_TAG,
        MOVEMENT_TITLE_TAG
    };

    return standardTags.contains(tag);
}

Notation::Notation(Ms::Score* score)
{
    m_scoreGlobal = new Ms::MScore(); //! TODO May be static?
    m_opened.val = false;

    m_undoStack = std::make_shared<NotationUndoStack>(this, m_notationChanged);
    m_interaction = std::make_shared<NotationInteraction>(this, m_undoStack);
    m_playback = std::make_shared<NotationPlayback>(this);
    m_midiInput = std::make_shared<NotationMidiInput>(this, m_undoStack);
    m_accessibility = std::make_shared<NotationAccessibility>(this, m_interaction->selectionChanged());
    m_parts = std::make_shared<NotationParts>(this, m_interaction, m_undoStack);
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

    setScore(score);
}

Notation::~Notation()
{
    delete m_score;
}

void Notation::init()
{
    Ms::MScore::init(); // initialize libmscore

    Ms::MScore::setNudgeStep(.1); // cursor key (default 0.1)
    Ms::MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    Ms::MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)

    Ms::MScore::pixelRatio = Ms::DPI / QGuiApplication::primaryScreen()->logicalDotsPerInch();
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

Meta Notation::metaInfo() const
{
    Meta meta;
    auto allTags = score()->metaTags();

    meta.title = score()->title();
    meta.subtitle = allTags[SUBTITLE_TAG];
    meta.composer = allTags[COMPOSER_TAG];
    meta.lyricist = allTags[LYRICIST_TAG];
    meta.copyright = allTags[COPYRIGHT_TAG];
    meta.translator = allTags[TRANSLATOR_TAG];
    meta.arranger = allTags[ARRANGER_TAG];
    meta.source = allTags[SOURCE_TAG];
    meta.creationDate = QDate::fromString(allTags[CREATION_DATE_TAG], Qt::ISODate);
    meta.platform = allTags[PLATFORM_TAG];
    meta.musescoreVersion = score()->mscoreVersion();
    meta.musescoreRevision = score()->mscoreRevision();
    meta.mscVersion = score()->mscVersion();

    for (const QString& tag : allTags.keys()) {
        if (isStandardTag(tag)) {
            continue;
        }

        meta.additionalTags[tag] = allTags[tag];
    }

    return meta;
}

void Notation::setMetaInfo(const Meta& meta)
{
    QMap<QString, QString> tags {
        { SUBTITLE_TAG, meta.subtitle },
        { COMPOSER_TAG, meta.composer },
        { LYRICIST_TAG, meta.lyricist },
        { COPYRIGHT_TAG, meta.copyright },
        { TRANSLATOR_TAG, meta.translator },
        { ARRANGER_TAG, meta.arranger },
        { SOURCE_TAG, meta.source },
        { PLATFORM_TAG, meta.platform },
        { CREATION_DATE_TAG, meta.creationDate.toString(Qt::ISODate) }
    };

    for (const QString& tag : meta.additionalTags.keys()) {
        tags[tag] = meta.additionalTags[tag].toString();
    }

    score()->setMetaTags(tags);
}

INotationPtr Notation::clone() const
{
    return std::make_shared<Notation>(score()->clone());
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

void Notation::paint(QPainter* painter, const QRectF& frameRect)
{
    const QList<Ms::Page*>& pages = score()->pages();
    if (pages.empty()) {
        return;
    }

    switch (score()->layoutMode()) {
    case Ms::LayoutMode::LINE:
    case Ms::LayoutMode::SYSTEM: {
        bool paintBorders = false;
        paintPages(painter, frameRect, { pages.first() }, paintBorders);
        break;
    }
    case Ms::LayoutMode::FLOAT:
    case Ms::LayoutMode::PAGE: {
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

void Notation::paintPageBorder(QPainter* painter, const Ms::Page* page) const
{
    QRectF boundingRect(page->canvasBoundingRect());

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(configuration()->borderColor(), configuration()->borderWidth()));
    painter->drawRect(boundingRect);

    if (!score()->showPageborders()) {
        return;
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(Ms::MScore::frameMarginColor);
    boundingRect.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
    painter->drawRect(boundingRect);

    if (!page->isOdd()) {
        painter->drawLine(boundingRect.right(), 0.0, boundingRect.right(), boundingRect.bottom());
    }
}

void Notation::paintElements(QPainter* painter, const QList<Element*>& elements) const
{
    QList<Ms::Element*> sortedElements = elements;
    std::sort(sortedElements.begin(), sortedElements.end(), [](Ms::Element* e1, Ms::Element* e2) {
        if (e1->z() == e2->z()) {
            if (e1->selected()) {
                return false;
            } else if (e2->selected()) {
                return true;
            } else if (!e1->visible()) {
                return true;
            } else if (!e2->visible()) {
                return false;
            } else {
                return e1->track() > e2->track();
            }
        }
        return e1->z() <= e2->z();
    });

    for (const Ms::Element* element : sortedElements) {
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
