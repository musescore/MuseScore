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
#include "paint.h"

#include "draw/painter.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/engravingitem.h"

#include "debugpaint.h"

#include "log.h"
#include "config.h"

using namespace mu::engraving;

void Paint::paintScore(draw::Painter* painter, Score* score, const Options& opt)
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
            RectF pageRect = page->bbox();
            RectF pageContentRect = pageRect.adjusted(page->lm(), page->tm(), -page->rm(), -page->bm());

            //! NOTE Trim page margins, if need
            if (opt.trimMarginPixelSize >= 0) {
                double trimSize = static_cast<double>(opt.trimMarginPixelSize);
                pageRect = pageContentRect.adjusted(-trimSize, -trimSize, trimSize, trimSize);
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

            if (opt.isMultiPage) {
                painter->translate(pagePos);
            }

            // Draw page sheet
            if (opt.onPaintPageSheet) {
                opt.onPaintPageSheet(painter, pageRect, pageContentRect, page->isOdd());
            } else if (opt.printPageBackground) {
                painter->fillRect(pageRect, Color::WHITE);
            }

            // Draw page elements
            painter->setClipping(true);
            painter->setClipRect(pageRect);
            std::vector<EngravingItem*> elements = page->items(drawRect.translated(-pagePos));
            paintElements(*painter, elements, opt.isPrinting);
            painter->setClipping(false);

#ifdef ENGRAVING_PAINT_DEBUGGER_ENABLED
            if (!opt.isPrinting) {
                engraving::DebugPaint::paintPageDebug(*painter, page);
            }
#endif

            if (opt.isMultiPage) {
                painter->translate(-pagePos);
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

SizeF Paint::pageSizeInch(Score* score)
{
    if (!score) {
        return SizeF();
    }

    //! NOTE If now it is not PAGE view mode,
    //! then the page sizes will differ from the standard sizes (in PAGE view mode)
    if (score->npages() > 0) {
        const Page* page = score->pages().front();
        return SizeF(page->bbox().width() / mu::engraving::DPI, page->bbox().height() / mu::engraving::DPI);
    }

    return SizeF(score->styleD(Sid::pageWidth), score->styleD(Sid::pageHeight));
}

void Paint::paintElement(mu::draw::Painter& painter, const EngravingItem* element)
{
    if (element->skipDraw()) {
        return;
    }
    element->itemDiscovered = false;
    PointF elementPosition(element->pagePos());

    painter.translate(elementPosition);
    element->draw(&painter);
    painter.translate(-elementPosition);
}

void Paint::paintElements(mu::draw::Painter& painter, const std::vector<EngravingItem*>& elements, bool isPrinting)
{
    std::vector<EngravingItem*> sortedElements(elements.begin(), elements.end());

    std::sort(sortedElements.begin(), sortedElements.end(), mu::engraving::elementLessThan);

    for (const EngravingItem* element : sortedElements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElement(painter, element);
    }

#ifdef ENGRAVING_PAINT_DEBUGGER_ENABLED
    if (!isPrinting) {
        DebugPaint::paintElementsDebug(painter, sortedElements);
    }
#else
    UNUSED(isPrinting);
#endif
}
