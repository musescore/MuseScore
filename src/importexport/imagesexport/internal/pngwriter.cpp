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

#include "pngwriter.h"

#include "project/projectutils.h"

#include <cmath>
#include <QImage>
#include <QBuffer>

#include "global/types/id.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;

std::vector<WriteUnitType> PngWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PAGE };
}

bool PngWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret PngWriter::write(INotationProjectPtr project, muse::io::IODevice& destinationDevice, const WriteOptions& options)
{
    WriteUnitType unitType = static_cast<WriteUnitType>(value(options, WriteOptionKey::UNIT_TYPE, Val(static_cast<int>(WriteUnitType::PER_PAGE))).toInt());
    IF_ASSERT_FAILED(supportsUnitType(unitType)) {
        return Ret(Ret::Code::NotSupported);
    }

    notation::INotationPtrList notations = project::resolveNotations(project, options);
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    INotationPtr notation = notations.front();

    const float CANVAS_DPI = configuration()->exportPngDpiResolution();

    INotationPainting::Options opt;
    opt.fromPage = value(options, WriteOptionKey::PAGE_NUMBER, Val(0)).toInt();
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

    const bool TRANSPARENT_BACKGROUND = value(options, WriteOptionKey::TRANSPARENT_BACKGROUND,
                                                      Val(configuration()->exportPngWithTransparentBackground())).toBool();
    image.fill(TRANSPARENT_BACKGROUND ? Qt::transparent : Qt::white);

    muse::draw::Painter painter(&image, "pngwriter");

    notation->painting()->paintPng(&painter, opt);

    if (configuration()->exportPngWithGrayscale()) {
        convertImageToGrayscale(image);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    image.save(&buf, "png");

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

    return true;
}

Ret PngWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();

    return ret;
}

void PngWriter::convertImageToGrayscale(QImage& image)
{
    // We convert every pixel to gray, preserving alpha channel (necessary for transparent background)
    for (int y = 0; y < image.height(); ++y) {
        QRgb* scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            QRgb& pixel = scanLine[x];
            uint ci = uint(qGray(pixel));
            pixel = qRgba(ci, ci, ci, qAlpha(pixel));
        }
    }
}
