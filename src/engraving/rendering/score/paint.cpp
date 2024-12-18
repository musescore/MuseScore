/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "paint.h"

#include "draw/painter.h"
#include "dom/score.h"
#include "dom/page.h"
#include "dom/engravingitem.h"

#include "tdraw.h"
#include "debugpaint.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void Paint::paintScore(Painter* painter, Score* score, const IScoreRenderer::PaintOptions& opt)
{
    TRACEFUNC;
    if (!score) {
        return;
    }

    const std::vector<Page*>& pages = score->pages();
    if (pages.empty()) {
        return;
    }

    //! NOTE This is DPI of paint device,  ex screen, image, printer and etc.
    //! Should be set, but if not set, we will use our default DPI.
    const int DEVICE_DPI = opt.deviceDpi > 0 ? opt.deviceDpi : mu::engraving::DPI;

    //! NOTE Depending on the view mode,
    //! if the view mode is PAGE, then this is one page size (ex A4),
    //! if someone a continuous mode, then this is the size of the entire score.
    SizeF pageSize = pageSizeInch(score);

    // Setup Painter
    painter->setAntialiasing(true);

    //! NOTE To draw on the screen, no need to adjust the viewport,
    //! to draw on others (pdf, png, printer), we need to set the viewport
    if (opt.isSetViewport) {
        painter->setViewport(RectF(0.0, 0.0, std::lrint(pageSize.width() * DEVICE_DPI), std::lrint(pageSize.height() * DEVICE_DPI)));
        painter->setWindow(RectF(0.0, 0.0, std::lrint(pageSize.width() * engraving::DPI), std::lrint(pageSize.height() * engraving::DPI)));
    }

    // Setup score draw system
    mu::engraving::MScore::pixelRatio = mu::engraving::DPI / DEVICE_DPI;
    score->setPrinting(opt.isPrinting);
    mu::engraving::MScore::pdfPrinting = opt.isPrinting;

    // Setup page counts
    int fromPage = opt.fromPage >= 0 ? opt.fromPage : 0;
    int toPage = (opt.toPage >= 0 && opt.toPage < int(pages.size())) ? opt.toPage : (int(pages.size()) - 1);

    for (int copy = 0; copy < opt.copyCount; ++copy) {
        bool firstPage = true;
        for (int pi = fromPage; pi <= toPage; ++pi) {
            Page* page = pages.at(pi);

            PointF pagePos = page->pos();
            RectF pageRect = page->ldata()->bbox();

            //! NOTE Trim page margins, if need
            if (opt.trimMarginPixelSize >= 0) {
                double trimMargin = static_cast<double>(opt.trimMarginPixelSize);
                pageRect = page->tbbox().adjusted(-trimMargin, -trimMargin, trimMargin, trimMargin);
            }

            //! NOTE Check draw rect, usually for optimisation drawing on screen (draw only what we see)
            RectF drawRect;
            RectF pageAbsRect = pageRect.translated(pagePos);
            if (opt.frameRect.isValid()) {
                if (pageAbsRect.right() < opt.frameRect.left()) {
                    continue;
                }

                if (pageAbsRect.left() > opt.frameRect.right()) {
                    break;
                }

                drawRect = opt.frameRect;
            } else {
                drawRect = pageAbsRect;
            }

            //! NOTE Notify about new page (usually for paged paint device, ex pdf, printer)
            if (!firstPage) {
                if (opt.onNewPage) {
                    opt.onNewPage();
                }
            }
            firstPage = false;

            painter->beginObject("page_" + std::to_string(pi));

            if (opt.isPrinting && painter->hasClipping()) {
                // prevent elements from being drawn off the edge of the page (e.g. too many staves),
                // for now only in "Publish" mode (opt.isPrinting == true)
                painter->setClipRect(pageAbsRect.intersected(drawRect));
            }

            if (opt.isMultiPage) {
                painter->translate(pagePos);
            } else if (opt.trimMarginPixelSize >= 0) {
                painter->translate(-pageRect.topLeft());
            }

            // Draw page sheet
            if (opt.onPaintPageSheet) {
                opt.onPaintPageSheet(painter, page, pageRect);
            } else if (opt.printPageBackground) {
                painter->fillRect(pageRect, Color::WHITE);
            }

            // Draw page elements
            bool disableClipping = false;

            if (!painter->hasClipping()) {
                painter->setClipping(true);
                painter->setClipRect(pageRect);
                disableClipping = true;
            }

            std::vector<EngravingItem*> elements = page->items(drawRect.translated(-pagePos));
            paintItems(*painter, elements);
            //DebugPaint::paintPageTree(*painter, page);

            if (disableClipping) {
                painter->setClipping(false);
            }

            if (!opt.isPrinting) {
                DebugPaint::paintPageDebug(*painter, page, elements);
            }

            painter->endObject(); // page

            if (opt.isMultiPage) {
                painter->translate(-pagePos);
            } else if (opt.trimMarginPixelSize >= 0) {
                painter->translate(pageRect.topLeft());
            }

            if ((copy + 1) < opt.copyCount) {
                //! NOTE Notify about new page (usually for paged paint device, ex pdf, printer)
                //! for next copy
                if (opt.onNewPage) {
                    opt.onNewPage();
                }
            }
        }
    }
}

SizeF Paint::pageSizeInch(const Score* score)
{
    if (!score) {
        return SizeF();
    }

    //! NOTE If now it is not PAGE view mode,
    //! then the page sizes will differ from the standard sizes (in PAGE view mode)
    if (score->npages() > 0) {
        const Page* page = score->pages().front();
        return SizeF(page->ldata()->bbox().width() / mu::engraving::DPI, page->ldata()->bbox().height() / mu::engraving::DPI);
    }

    return SizeF(score->style().styleD(Sid::pageWidth), score->style().styleD(Sid::pageHeight));
}

SizeF Paint::pageSizeInch(const Score* score, const IScoreRenderer::PaintOptions& opt)
{
    if (!score) {
        return SizeF();
    }

    int pageNo = opt.fromPage >= 0 ? opt.fromPage : 0;
    if (pageNo >= int(score->npages())) {
        return SizeF();
    }

    const Page* page = score->pages().at(pageNo);

    RectF pageRect = page->ldata()->bbox();

    //! NOTE Trim page margins, if need
    if (opt.trimMarginPixelSize >= 0) {
        double trimMargin = static_cast<double>(opt.trimMarginPixelSize);
        pageRect = page->tbbox().adjusted(-trimMargin, -trimMargin, trimMargin, trimMargin);
    }

    return pageRect.size() / mu::engraving::DPI;
}

void Paint::paintItem(Painter& painter, const EngravingItem* item)
{
    TRACEFUNC;
    if (item->ldata()->isSkipDraw()) {
        return;
    }
    item->itemDiscovered = false;
    PointF itemPosition(item->pagePos());

    painter.translate(itemPosition);
    TDraw::drawItem(item, &painter);
    painter.translate(-itemPosition);
}

void Paint::paintItems(Painter& painter, const std::vector<EngravingItem*>& items)
{
    TRACEFUNC;
    std::vector<EngravingItem*> sortedItems(items.begin(), items.end());

    std::sort(sortedItems.begin(), sortedItems.end(), mu::engraving::elementLessThan);

    for (const EngravingItem* item : sortedItems) {
        if (!item->isInteractionAvailable()) {
            continue;
        }

        paintItem(painter, item);
    }
}
