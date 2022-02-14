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
#include "engraving/paint/paint.h"

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

Ms::Score* NotationPainting::score() const
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

    return score()->npages();
}

SizeF NotationPainting::pageSizeInch() const
{
    if (!score()) {
        return SizeF();
    }

    //! NOTE If now it is not PAGE view mode,
    //! then the page sizes will differ from the standard sizes (in PAGE view mode)
    if (score()->npages() > 0) {
        const Ms::Page* page = score()->pages().front();
        return SizeF(page->bbox().width() / Ms::DPI, page->bbox().height() / Ms::DPI);
    }

    return SizeF(score()->styleD(Ms::Sid::pageWidth), score()->styleD(Ms::Sid::pageHeight));
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

    const QList<Ms::Page*>& pages = score()->pages();
    if (pages.empty()) {
        return;
    }

    //! NOTE This is DPI of paint device,  ex screen, image, printer and etc.
    //! Should be set, but if not set, we will use our default DPI.
    const int DEVICE_DPI = opt.deviceDpi > 0 ? opt.deviceDpi : Ms::DPI;

    //! NOTE Depending on the view mode,
    //! if the view mode is PAGE, then this is one page size (ex A4),
    //! if someone a continuous mode, then this is the size of the entire score.
    SizeF pageSize = pageSizeInch();

    // Setup Painter
    painter->setAntialiasing(true);

    //! NOTE To draw on the screen, no need to adjust the viewport,
    //! to draw on others (pdf, png, printer), we need to set the viewport
    if (opt.isSetViewport) {
        painter->setViewport(RectF(0.0, 0.0, pageSize.width() * DEVICE_DPI, pageSize.height() * DEVICE_DPI));
        painter->setWindow(RectF(0.0, 0.0, pageSize.width() * Ms::DPI, pageSize.height() * Ms::DPI));
    }

    // Setup score draw system
    Ms::MScore::pixelRatio = Ms::DPI / DEVICE_DPI;
    score()->setPrinting(opt.isPrinting);
    Ms::MScore::pdfPrinting = opt.isPrinting;

    // Setup page counts
    int fromPage = opt.fromPage >= 0 ? opt.fromPage : 0;
    int toPage = (opt.toPage >= 0 && opt.toPage < pages.count()) ? opt.toPage : (pages.count() - 1);

    for (int copy = 0; copy < opt.copyCount; ++copy) {
        bool firstPage = true;
        for (int pi = fromPage; pi <= toPage; ++pi) {
            Ms::Page* page = pages.at(pi);

            PointF pagePos = page->pos();
            RectF pageRect = page->bbox();
            RectF pageContentRect = pageRect.adjusted(page->lm(), page->tm(), -page->rm(), -page->bm());

            //! NOTE Trim page margins, if need
            if (opt.trimMarginPixelSize >= 0) {
                qreal trimSize = static_cast<qreal>(opt.trimMarginPixelSize);
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
            paintPageSheet(painter, pageRect, pageContentRect, page->isOdd());

            // Draw page elements
            painter->setClipping(true);
            painter->setClipRect(pageRect);
            QList<EngravingItem*> elements = page->items(drawRect.translated(-pagePos));
            engraving::Paint::paintElements(*painter, elements);
            painter->setClipping(false);

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

        if (!opt.isPrinting) {
            static_cast<NotationInteraction*>(m_notation->interaction().get())->paint(painter);
        }
    }
}

void NotationPainting::paintPageSheet(Painter* painter, const RectF& pageRect, const RectF& pageContentRect, bool isOdd) const
{
    TRACEFUNC;
    if (score()->printing()) {
        painter->fillRect(pageRect, Color::white);
        return;
    }

    if (configuration()->foregroundUseColor()) {
        painter->fillRect(pageRect, configuration()->foregroundColor());
    } else {
        io::path wallpaperPath = configuration()->foregroundWallpaperPath();
        if (!wallpaperPath.empty()) {
            static QPixmap px(wallpaperPath.toQString());
            static io::path lastPath(wallpaperPath);
            if (lastPath != wallpaperPath) {
                px = QPixmap(wallpaperPath.toQString());
                lastPath = wallpaperPath;
            }
            painter->drawTiledPixmap(pageRect, px);
        } else {
            //! NOTE We can use the color from the configuration,
            //! but in this case I believe it is better to use the "unassigned" color
            painter->fillRect(pageRect, Color::white);
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
    opt.deviceDpi = uiConfiguration()->dpi();
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
