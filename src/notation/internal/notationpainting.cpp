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
#include "engraving/paint/paint.h"

#include "notation.h"
#include "notationinteraction.h"

#include "log.h"

using namespace mu;
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

int NotationPainting::pageCount() const
{
    if (!score()) {
        return 0;
    }

    return score()->npages();
}

SizeF NotationPainting::pageSizeInch() const
{
    if (!score()) {
        return SizeF();
    }

    return SizeF(score()->styleD(Ms::Sid::pageWidth), score()->styleD(Ms::Sid::pageHeight));
}

void NotationPainting::paintView(Painter* painter, const RectF& frameRect)
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

void NotationPainting::paintPdf(draw::PagedPaintDevice* dev, draw::Painter* painter, const Options& opt)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setPrinting(true);
    Ms::MScore::pdfPrinting = true;

    QSizeF size(score()->styleD(Ms::Sid::pageWidth), score()->styleD(Ms::Sid::pageHeight));
    painter->setAntialiasing(true);
    painter->setViewport(RectF(0.0, 0.0, size.width() * dev->logicalDpiX(), size.height() * dev->logicalDpiY()));
    painter->setWindow(RectF(0.0, 0.0, size.width() * Ms::DPI, size.height() * Ms::DPI));

    double pixelRationBackup = Ms::MScore::pixelRatio;
    Ms::MScore::pixelRatio = Ms::DPI / dev->logicalDpiX();

    for (int pageNumber = 0; pageNumber < score()->npages(); ++pageNumber) {
        if (pageNumber > 0) {
            dev->newPage();
        }

        score()->print(painter, pageNumber);
    }

    score()->setPrinting(false);
    Ms::MScore::pixelRatio = pixelRationBackup;
    Ms::MScore::pdfPrinting = false;
}

void NotationPainting::paintPrint(draw::PagedPaintDevice* dev, draw::Painter* painter, const Options& opt)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    if (opt.fromPage >= score()->npages()) {
        return;
    }

    score()->setPrinting(true);
    Ms::MScore::pdfPrinting = true;

    QSizeF size(score()->styleD(Ms::Sid::pageWidth), score()->styleD(Ms::Sid::pageHeight));
    painter->setAntialiasing(true);
    painter->setViewport(RectF(0.0, 0.0, size.width() * dev->logicalDpiX(), size.height() * dev->logicalDpiY()));
    painter->setWindow(RectF(0.0, 0.0, size.width() * Ms::DPI, size.height() * Ms::DPI));

    double pixelRationBackup = Ms::MScore::pixelRatio;
    Ms::MScore::pixelRatio = Ms::DPI / dev->logicalDpiX();

    int fromPage = opt.fromPage >= 0 ? opt.fromPage : 0;
    int toPage = (opt.toPage >= 0 && opt.toPage < score()->npages()) ? opt.toPage : (score()->npages() - 1);

    for (int copy = 0; copy < opt.copyCount; ++copy) {
        bool firstPage = true;
        for (int p = fromPage; p <= toPage; ++p) {
            if (!firstPage) {
                dev->newPage();
            }
            firstPage = false;

            score()->print(painter, p);

            if ((copy + 1) < opt.copyCount) {
                dev->newPage();
            }
        }
    }

    score()->setPrinting(false);
    Ms::MScore::pixelRatio = pixelRationBackup;
    Ms::MScore::pdfPrinting = false;
}

void NotationPainting::paintPng(Painter* painter, const Options& opt)
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setPrinting(true);

    double pixelRatioBackup = Ms::MScore::pixelRatio;

    const int PAGE_NUMBER = opt.fromPage;
    const QList<Ms::Page*>& pages = score()->pages();

    if (PAGE_NUMBER < 0 || PAGE_NUMBER >= pages.size()) {
        return;
    }

    Ms::Page* page = pages[PAGE_NUMBER];

    const int TRIM_MARGIN_SIZE = opt.trimMarginPixelSize;
    RectF pageRect = page->abbox();

    if (TRIM_MARGIN_SIZE >= 0) {
        pageRect = page->tbbox().adjusted(-TRIM_MARGIN_SIZE, -TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE);
    }

    double scaling = opt.deviceDpi / Ms::DPI;
    Ms::MScore::pixelRatio = 1.0 / scaling;

    painter->setAntialiasing(true);
    painter->scale(scaling, scaling);
    if (TRIM_MARGIN_SIZE >= 0) {
        painter->translate(-pageRect.topLeft());
    }

    QList<Ms::EngravingItem*> elements = page->elements();
    std::stable_sort(elements.begin(), elements.end(), Ms::elementLessThan);

    engraving::Paint::paintElements(*painter, elements);

    score()->setPrinting(false);
    Ms::MScore::pixelRatio = pixelRatioBackup;
}
