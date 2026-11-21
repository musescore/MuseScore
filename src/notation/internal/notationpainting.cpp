/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "notationpainting.h"

#include <QScreen>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/playback/practiceemphasisdata.h"

#include "notation.h"
#include "notationinteraction.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

namespace {
const Color kEditedNoteColor(224, 32, 48);
const Color kOtherLoudNoteColor(200, 128, 64);

Color editedAddedHighlightColor(bool invertColors)
{
    // Pale highlighter behind manually added notes. Dark (inverted) paper needs a
    // slightly stronger tint so the mark stays visible without covering the note.
    return invertColors ? Color(255, 110, 130, 120)
           : Color(255, 96, 112, 78);
}

void paintEditedNoteHighlights(Painter* painter, const Page* page, bool invertColors)
{
    if (!painter || !page) {
        return;
    }
    const Score* score = page->score();
    const MasterScore* master = score ? score->masterScore() : nullptr;
    if (!master || !master->practiceEmphasis().document().emphasisMode) {
        return;
    }

    const PracticeEmphasisData& emphasis = master->practiceEmphasis();
    painter->save();
    painter->setNoPen();
    painter->setBrush(Brush(editedAddedHighlightColor(invertColors)));

    for (EngravingItem* item : page->elements()) {
        if (!item || !item->isNote() || item->ldata()->isSkipDraw()) {
            continue;
        }
        const Note* note = toNote(item);
        if (emphasis.noteHighlight(note) != PracticeEmphasisData::Highlight::EditedAdded) {
            continue;
        }
        const double pad = 0.5 * note->spatium();
        const double radius = 0.45 * note->spatium();
        const RectF halo = note->pageBoundingRect().adjusted(-pad, -pad, pad, pad);
        painter->drawRoundedRect(halo, radius, radius);
    }

    painter->restore();
}
}

NotationPainting::NotationPainting(Notation* notation)
    : muse::Contextable(notation->iocContext())
    , m_notation(notation)
{
}

mu::engraving::Score* NotationPainting::score() const
{
    return m_notation->score();
}

void NotationPainting::setViewMode(const ViewMode& viewMode)
{
    if (!score()) {
        return;
    }

    if (score()->layoutMode() == viewMode) {
        return;
    }

    score()->setLayoutMode(viewMode);
    score()->doLayout();

    m_viewModeChanged.notify();
    m_notation->notifyAboutNotationChanged();
}

ViewMode NotationPainting::viewMode() const
{
    if (!score()) {
        return ViewMode::PAGE;
    }

    return score()->layoutMode();
}

muse::async::Notification NotationPainting::viewModeChanged() const
{
    return m_viewModeChanged;
}

int NotationPainting::pageCount() const
{
    if (!score()) {
        return 0;
    }

    return static_cast<int>(score()->npages());
}

SizeF NotationPainting::pageSizeInch() const
{
    return scoreRenderer()->pageSizeInch(score());
}

SizeF NotationPainting::pageSizeInch(const Options& opt) const
{
    return scoreRenderer()->pageSizeInch(score(), opt);
}

bool NotationPainting::isPaintPageBorder() const
{
    switch (score()->layoutMode()) {
    case engraving::LayoutMode::LINE:
    case engraving::LayoutMode::HORIZONTAL_FIXED:
    case engraving::LayoutMode::SYSTEM:
        return false;
    case engraving::LayoutMode::FLOAT:
    case engraving::LayoutMode::PAGE: {
        return !score()->printing();
    }
    }
    return false;
}

void NotationPainting::doPaint(Painter* painter, const Options& opt)
{
    TRACEFUNC;
    if (!score()) {
        return;
    }

    Options myopt = opt;
    bool printPageBackground = myopt.printPageBackground;
    auto extraPagePaint = myopt.onPaintPageSheet;
    myopt.onPaintPageSheet = [this, printPageBackground, extraPagePaint](Painter* painter, const Page* page, const RectF& pageRect) {
        paintPageSheet(painter, page, pageRect, printPageBackground);
        if (extraPagePaint) {
            extraPagePaint(painter, page, pageRect);
        }
    };

    scoreRenderer()->paintScore(painter, score(), myopt);

    if (!myopt.isPrinting) {
        rendering::PaintOptions eopt;
        eopt.isPrinting = myopt.isPrinting;
        eopt.invertColors = myopt.invertColors;

        static_cast<NotationInteraction*>(m_notation->interaction().get())->paint(painter, eopt);
    }
}

void NotationPainting::paintPageSheet(Painter* painter, const Page* page, const RectF& pageRect, bool printPageBackground) const
{
    TRACEFUNC;
    if (score()->printing()) {
        if (!printPageBackground) {
            return;
        }

        painter->fillRect(pageRect, Color::WHITE);
        return;
    }

    if (configuration()->foregroundUseColor()) {
        painter->fillRect(pageRect, configuration()->foregroundColor());
    } else {
        const QPixmap& wallpaper = configuration()->foregroundWallpaper();
        if (!wallpaper.isNull()) {
            painter->drawTiledPixmap(pageRect, wallpaper);
        } else {
            //! NOTE We can use the color from the configuration,
            //! but in this case I believe it is better to use the "unassigned" color
            painter->fillRect(pageRect, Color::WHITE);
        }
    }

    if (!isPaintPageBorder()) {
        return;
    }

    const double strokeWidth = 0.07 * DPMM;
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(Pen(configuration()->borderColor(), strokeWidth));
    painter->drawRect(pageRect);

    if (!score()->showPageborders()) {
        return;
    }

    RectF pageContentRect = page->ldata()->bbox().adjusted(page->lm(), page->tm(), -page->rm(), -page->bm());

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(Pen(engravingConfiguration()->scoreGreyColor(), strokeWidth));
    painter->drawRect(pageContentRect);

    if (!page->isOdd()) {
        painter->drawLine(pageContentRect.right(), 0.0, pageContentRect.right(), pageContentRect.bottom());
    }
}

void NotationPainting::paintView(Painter* painter, const RectF& frameRect, bool isPrinting, bool isAutomation)
{
    Options opt;
    opt.isSetViewport = false;
    opt.isMultiPage = true;
    opt.frameRect = frameRect;
    opt.deviceDpi = uiConfiguration()->logicalDpi();
    opt.isPrinting = isPrinting;
    opt.invertColors = configuration()->shouldInvertScore();

    if (isAutomation) {
        opt.overrideItemColor = [](const EngravingItem* item, Color defaultColor) {
            if (item->isDynamic() || item->isHairpinSegment()) {
                return defaultColor;
            }
            defaultColor.setAlpha(defaultColor.alpha() * 0.4f);
            return defaultColor;
        };
    } else {
        // Screen only. Printing returns the notation color before this hook runs.
        opt.overrideItemColor = [](const EngravingItem* item, Color defaultColor) {
            if (!item->isNote()) {
                return defaultColor;
            }
            if (item->selected()) {
                return defaultColor;
            }
            const Note* note = toNote(item);
            const MasterScore* master = note->score() ? note->score()->masterScore() : nullptr;
            if (!master || !master->practiceEmphasis().document().emphasisMode) {
                return defaultColor;
            }
            switch (master->practiceEmphasis().noteHighlight(note)) {
            case PracticeEmphasisData::Highlight::EditedAdded:
            case PracticeEmphasisData::Highlight::Edited:
                return kEditedNoteColor;
            case PracticeEmphasisData::Highlight::OtherLoud:
                return kOtherLoudNoteColor;
            case PracticeEmphasisData::Highlight::None:
                break;
            }
            return defaultColor;
        };

        if (!isPrinting) {
            const bool invertColors = opt.invertColors;
            opt.onPaintPageSheet = [invertColors](Painter* painter, const Page* page, const RectF&) {
                paintEditedNoteHighlights(painter, page, invertColors);
            };
        }
    }

    doPaint(painter, opt);
}

void NotationPainting::paintPdf(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}

void NotationPainting::paintPrint(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}

void NotationPainting::paintPng(Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}
