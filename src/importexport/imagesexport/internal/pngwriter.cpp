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
#include <QImage>

#include "libmscore/masterscore.h"
#include "libmscore/page.h"
#include "engraving/infrastructure/paint.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;

std::vector<INotationWriter::UnitType> PngWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

mu::Ret PngWriter::write(INotationPtr notation, QIODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    const float CANVAS_DPI = configuration()->exportPngDpiResolution();

    INotationPainting::Options opt;
    opt.fromPage = options.value(OptionKey::PAGE_NUMBER, Val(0)).toInt();
    opt.toPage = opt.fromPage;
    opt.trimMarginPixelSize = configuration()->trimMarginPixelSize();
    opt.deviceDpi = CANVAS_DPI;
    opt.printPageBackground = false; // Printed by us using image.fill

    const SizeF pageSizeInch = notation->painting()->pageSizeInch(opt);

    int width = std::lrint(pageSizeInch.width() * CANVAS_DPI);
    int height = std::lrint(pageSizeInch.height() * CANVAS_DPI);

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));
    image.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));

    const bool TRANSPARENT_BACKGROUND = options.value(OptionKey::TRANSPARENT_BACKGROUND, Val(false)).toBool();
    image.fill(TRANSPARENT_BACKGROUND ? Qt::transparent : Qt::white);

    mu::draw::Painter painter(&image, "pngwriter");

    notation->painting()->paintPng(&painter, opt);

    image.save(&destinationDevice, "png");

    return true;
}
