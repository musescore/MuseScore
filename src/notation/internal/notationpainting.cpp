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
#include "notationpainting.h"

#include "engraving/libmscore/score.h"

#include "notation.h"
#include "notationinteraction.h"

using namespace mu::notation;
using namespace mu::draw;

NotationPainting::NotationPainting(Notation* notation)
    : m_notation(notation)
{
}

Ms::Score* NotationPainting::score() const
{
    return m_notation->score();
}

void NotationPainting::setViewMode(const ViewMode& viewMode)
{
    if (!score()) {
        return;
    }

    score()->setLayoutMode(viewMode);
    score()->doLayout();
    m_notation->notifyAboutNotationChanged();
}

ViewMode NotationPainting::viewMode() const
{
    if (!score()) {
        return ViewMode::PAGE;
    }

    return score()->layoutMode();
}

void NotationPainting::paint(Painter* painter, const RectF& frameRect)
{
    const QList<Ms::Page*>& pages = score()->pages();
    if (pages.empty()) {
        return;
    }

    switch (score()->layoutMode()) {
    case engraving::LayoutMode::LINE:
    case engraving::LayoutMode::HORIZONTAL_FIXED:
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

    static_cast<NotationInteraction*>(m_notation->interaction().get())->paint(painter);
}

void NotationPainting::paintPages(draw::Painter* painter, const RectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const
{
    for (Ms::Page* page : pages) {
        RectF drawRect;
        RectF pageRect(page->abbox().translated(page->pos()));
        if (frameRect.isValid()) {
            if (pageRect.right() < frameRect.left()) {
                continue;
            }

            if (pageRect.left() > frameRect.right()) {
                break;
            }

            drawRect = frameRect;
        } else {
            drawRect = page->abbox();
        }

        if (paintBorders) {
            paintPageBorder(painter, page);
        }

        PointF pagePosition(page->pos());
        painter->translate(pagePosition);
        paintForeground(painter, page->bbox());
        painter->translate(-pagePosition);

        engraving::Paint::paintPage(*painter, page, drawRect.translated(-page->pos()));
    }
}

void NotationPainting::paintPageBorder(draw::Painter* painter, const Ms::Page* page) const
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

void NotationPainting::paintForeground(mu::draw::Painter* painter, const RectF& pageRect) const
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
