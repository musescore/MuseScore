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

#include <unordered_set>

#include "engraving/dom/score.h"
#include "engraving/dom/spanner.h"

#include "notation.h"
#include "notationinteraction.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::draw;

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
    myopt.onPaintPageSheet = [this, printPageBackground](Painter* painter, const Page* page, const RectF& pageRect) {
        paintPageSheet(painter, page, pageRect, printPageBackground);
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

namespace notationpainting_detail {
//! Default for the playback influence highlight (60% opaque mild orange); becomes a preference in Phase 4
const Color PLAYBACK_INFLUENCE_HIGHLIGHT_COLOR(0xdb, 0x91, 0x37, 0x99);

//! An item is highlighted if it, its spanner, or anything it belongs to (e.g. the chord of a
//! highlighted grace chord's notes) is in the set. Stops at the segment: nothing above it is ever highlighted.
bool isPlaybackInfluenceHighlighted(const EngravingItem* item, const std::unordered_set<const EngravingItem*>& highlight)
{
    const EngravingObject* current = item;

    while (current && current->isEngravingItem() && !current->isSegment() && !current->isMeasure() && !current->isSystem()) {
        const EngravingItem* currentItem = static_cast<const EngravingItem*>(current);

        if (highlight.contains(currentItem)) {
            return true;
        }

        if (currentItem->isSpannerSegment() && highlight.contains(toSpannerSegment(currentItem)->spanner())) {
            return true;
        }

        current = current->parent();
    }

    return false;
}
} // namespace notationpainting_detail

void NotationPainting::paintView(
    Painter* painter,
    const RectF& frameRect,
    bool isPrinting,
    bool isAutomation,
    bool showPlaybackInfluence)
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
    }

    const std::unordered_set<const EngravingItem*>& highlight = m_notation->interaction()->playbackInfluenceHighlight();

    if (showPlaybackInfluence && !highlight.empty()) {
        const auto fallback = opt.overrideItemColor;

        // Only used during doPaint() below, so capturing the set by reference is safe
        opt.overrideItemColor = [&highlight, fallback](const EngravingItem* item, Color defaultColor) {
            if (notationpainting_detail::isPlaybackInfluenceHighlighted(item, highlight)) {
                return notationpainting_detail::PLAYBACK_INFLUENCE_HIGHLIGHT_COLOR;
            }

            return fallback
                   ? fallback(item, defaultColor)
                   : defaultColor;
        };
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
