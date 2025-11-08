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

#include <cmath>
#include <QImage>
#include <QBuffer>

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;

std::vector<INotationWriter::UnitType> PngWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PAGE };
}

Ret PngWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    const float CANVAS_DPI = configuration()->exportPngDpiResolution();

    // Check if we're exporting a capture rectangle
    bool hasCaptureRect = options.find(OptionKey::CAPTURE_RECT_X) != options.end()
                          && options.find(OptionKey::CAPTURE_RECT_Y) != options.end()
                          && options.find(OptionKey::CAPTURE_RECT_W) != options.end()
                          && options.find(OptionKey::CAPTURE_RECT_H) != options.end();
    muse::RectF captureRect;
    if (hasCaptureRect) {
        double x = muse::value(options, OptionKey::CAPTURE_RECT_X, Val(0.0)).toDouble();
        double y = muse::value(options, OptionKey::CAPTURE_RECT_Y, Val(0.0)).toDouble();
        double w = muse::value(options, OptionKey::CAPTURE_RECT_W, Val(0.0)).toDouble();
        double h = muse::value(options, OptionKey::CAPTURE_RECT_H, Val(0.0)).toDouble();
        captureRect = muse::RectF(x, y, w, h);
        LOGI() << "Capture rectangle: " << captureRect.x() << ", " << captureRect.y()
               << " size: " << captureRect.width() << " x " << captureRect.height();
    }

    INotationPainting::Options opt;
    opt.fromPage = muse::value(options, OptionKey::PAGE_NUMBER, Val(0)).toInt();
    opt.toPage = opt.fromPage;
    opt.trimMarginPixelSize = configuration()->trimMarginPixelSize();
    opt.deviceDpi = CANVAS_DPI;
    opt.printPageBackground = false; // Printed by us using image.fill

    // Set the viewport to the capture rectangle if provided
    if (hasCaptureRect && !captureRect.isEmpty()) {
        opt.frameRect = captureRect;
        opt.isSetViewport = true;
        LOGI() << "Set frameRect: " << opt.frameRect.x() << ", " << opt.frameRect.y()
               << " size: " << opt.frameRect.width() << " x " << opt.frameRect.height();
    }

    int width, height;
    if (hasCaptureRect && !captureRect.isEmpty()) {
        // Convert capture rectangle from staff spaces to pixels
        // The capture rectangle is in score coordinates (staff spaces)
        width = std::lrint(captureRect.width() * CANVAS_DPI / mu::engraving::DPI);
        height = std::lrint(captureRect.height() * CANVAS_DPI / mu::engraving::DPI);
        LOGI() << "Image size for capture: " << width << " x " << height << " pixels";
    } else {
        // Normal page export
        const SizeF pageSizeInch = notation->painting()->pageSizeInch(opt);
        width = std::lrint(pageSizeInch.width() * CANVAS_DPI);
        height = std::lrint(pageSizeInch.height() * CANVAS_DPI);
        LOGI() << "Image size for full page: " << width << " x " << height << " pixels";
    }

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.setDotsPerMeterX(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));
    image.setDotsPerMeterY(std::lrint((CANVAS_DPI * 1000) / mu::engraving::INCH));

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
                                                    Val(configuration()->exportPngWithTransparentBackground())).toBool();
    image.fill(TRANSPARENT_BACKGROUND ? Qt::transparent : Qt::white);

    muse::draw::Painter painter(&image, "pngwriter");

    // Translate painter so capture rectangle starts at origin
    if (hasCaptureRect && !captureRect.isEmpty()) {
        painter.translate(-captureRect.x(), -captureRect.y());
        LOGI() << "Translated painter by " << -captureRect.x() << ", " << -captureRect.y();
    }

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
