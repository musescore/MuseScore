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

#include "pngwriter.h"

#include <cmath>

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/page.h"

#include "engraving/draw/qpainterprovider.h"

#include <QImage>

using namespace mu::iex::imagesexport;
using namespace mu::notation;
using namespace mu::io;

std::vector<INotationWriter::UnitType> PngWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

mu::Ret PngWriter::write(INotationPtr notation, Device& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    score->setPrinting(true); // don’t print page break symbols etc.

    double pixelRatioBackup = Ms::MScore::pixelRatio;

    const int PAGE_NUMBER = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    const QList<Ms::Page*>& pages = score->pages();

    if (PAGE_NUMBER < 0 || PAGE_NUMBER >= pages.size()) {
        return false;
    }

    Ms::Page* page = pages[PAGE_NUMBER];

    const int TRIM_MARGIN_SIZE = configuration()->trimMarginPixelSize();
    RectF pageRect = page->abbox();

    if (TRIM_MARGIN_SIZE >= 0) {
        QMarginsF margins(TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE, TRIM_MARGIN_SIZE);
        pageRect = page->tbbox().toQRectF() + margins;
    }

    const float CANVAS_DPI = configuration()->exportPngDpiResolution();
    int width = std::lrint(pageRect.width() * CANVAS_DPI / Ms::DPI);
    int height = std::lrint(pageRect.height() * CANVAS_DPI / Ms::DPI);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / Ms::INCH));
    image.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / Ms::INCH));

    const bool TRANSPARENT_BACKGROUND = options.value(OptionKey::TRANSPARENT_BACKGROUND, Val(false)).toBool();
    image.fill(TRANSPARENT_BACKGROUND ? 0 : Qt::white);

    double scaling = CANVAS_DPI / Ms::DPI;
    Ms::MScore::pixelRatio = 1.0 / scaling;

    mu::draw::Painter painter(&image, "pngwriter");
    painter.setAntialiasing(true);
    painter.scale(scaling, scaling);
    if (TRIM_MARGIN_SIZE >= 0) {
        painter.translate(-pageRect.topLeft());
    }

    QList<Ms::Element*> elements = page->elements();
    std::stable_sort(elements.begin(), elements.end(), Ms::elementLessThan);

    Ms::paintElements(painter, elements);
    image.save(&destinationDevice, "png");

    score->setPrinting(false);
    Ms::MScore::pixelRatio = pixelRatioBackup;

    return true;
}
