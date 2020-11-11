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

#include "pngwriter.h"

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/page.h"

#include <QImage>
#include <QPainter>

using namespace mu::importexport;
using namespace mu::framework;
using namespace Ms;

mu::Ret PngWriter::write(const Score& score, IODevice& destinationDevice, const Options& options)
{
    const_cast<Score&>(score).setPrinting(true); // don’t print page break symbols etc.
    double pixelRatioBackup = MScore::pixelRatio;

    const int PAGE_NUMBER = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    const QList<Page*>& pages = score.pages();

    if (PAGE_NUMBER < 0 || PAGE_NUMBER >= pages.size()) {
        return false;
    }

    Page* page = pages[PAGE_NUMBER];

    const int TRIM_MARGIN_SIZE = options.value(OptionKey::TRIM_MARGINS_SIZE, Val(0)).toInt();
    QRectF pageRect = page->abbox();

    if (TRIM_MARGIN_SIZE >= 0) {
        QMarginsF margins(TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE);
        pageRect = page->tbbox() + margins;
    }

    const double CANVAS_DPI = configuration()->exportPngDpiResolution();
    int width = std::lrint(pageRect.width() * CANVAS_DPI / DPI);
    int height = std::lrint(pageRect.height() * CANVAS_DPI / DPI);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / INCH));
    image.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / INCH));

    const bool TRANSPARENT_BACKGROUND = options.value(OptionKey::TRANSPARENT_BACKGROUND, Val(false)).toBool();
    image.fill(TRANSPARENT_BACKGROUND ? 0 : Qt::white);

    double scaling = CANVAS_DPI / DPI;
    MScore::pixelRatio = 1.0 / scaling;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.scale(scaling, scaling);
    if (TRIM_MARGIN_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    QList<Element*> elements = page->elements();
    std::stable_sort(elements.begin(), elements.end(), elementLessThan);

    paintElements(painter, elements);
    image.save(&destinationDevice, "png");

    const_cast<Score&>(score).setPrinting(false);
    MScore::pixelRatio = pixelRatioBackup;

    return true;
}
