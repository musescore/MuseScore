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

#include <QScreen>

#include "engraving/libmscore/score.h"
#include "engraving/infrastructure/paint.h"
#include "engraving/infrastructure/debugpaint.h"

#include "notation.h"
#include "notationinteraction.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::draw;

NotationPainting::NotationPainting(Notation* notation)
    : m_notation(notation)
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

    m_notation->notifyAboutNotationChanged();
}

ViewMode NotationPainting::viewMode() const
{
    if (!score()) {
        return ViewMode::PAGE;
    }

    return score()->layoutMode();
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
    if (!score()) {
        return SizeF();
    }

    //! NOTE If now it is not PAGE view mode,
    //! then the page sizes will differ from the standard sizes (in PAGE view mode)
    if (score()->npages() > 0) {
        const mu::engraving::Page* page = score()->pages().front();
        return SizeF(page->bbox().width() / mu::engraving::DPI, page->bbox().height() / mu::engraving::DPI);
    }

    return SizeF(score()->styleD(mu::engraving::Sid::pageWidth), score()->styleD(mu::engraving::Sid::pageHeight));
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

void NotationPainting::doPaint(draw::Painter* painter, const Options& opt)
{
    TRACEFUNC;
    if (!score()) {
        return;
    }

    Options myopt = opt;
    bool printPageBackground = myopt.printPageBackground;
    myopt.onPaintPageSheet
        = [this, printPageBackground](draw::Painter* painter, const RectF& pageRect, const RectF& pageContentRect, bool isOdd) {
        paintPageSheet(painter, pageRect, pageContentRect, isOdd, printPageBackground);
    };

    engraving::Paint::paintScore(painter, score(), myopt);

    if (!myopt.isPrinting) {
        static_cast<NotationInteraction*>(m_notation->interaction().get())->paint(painter);
    }
}

void NotationPainting::paintPageSheet(Painter* painter, const RectF& pageRect, const RectF& pageContentRect, bool isOdd,
                                      bool printPageBackground) const
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

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(Pen(configuration()->borderColor(), configuration()->borderWidth()));
    painter->drawRect(pageRect);

    if (!score()->showPageborders()) {
        return;
    }

    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(engravingConfiguration()->formattingMarksColor());
    painter->drawRect(pageContentRect);

    if (!isOdd) {
        painter->drawLine(pageContentRect.right(), 0.0, pageContentRect.right(), pageContentRect.bottom());
    }
}

void NotationPainting::paintView(Painter* painter, const RectF& frameRect, bool isPrinting)
{
    Options opt;
    opt.isSetViewport = false;
    opt.isMultiPage = true;
    opt.frameRect = frameRect;
    opt.deviceDpi = uiConfiguration()->logicalDpi();
    opt.isPrinting = isPrinting;
    doPaint(painter, opt);
}

void NotationPainting::paintPdf(draw::Painter* painter, const Options& opt)
{
    Q_ASSERT(opt.deviceDpi > 0);
    Options myopt = opt;
    myopt.isSetViewport = true;
    myopt.isMultiPage = false;
    myopt.isPrinting = true;
    doPaint(painter, myopt);
}

void NotationPainting::paintPrint(draw::Painter* painter, const Options& opt)
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
