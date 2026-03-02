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

#include "pdfwriter.h"

#include "project/projectutils.h"

#include <QPdfWriter>
#include <QBuffer>

#include "engraving/dom/masterscore.h"
#include "global/types/id.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;

std::vector<WriteUnitType> PdfWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PART, WriteUnitType::MULTI_PART };
}

bool PdfWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret PdfWriter::write(INotationProjectPtr project, muse::io::IODevice& destinationDevice, const WriteOptions& options)
{
    WriteUnitType unitType = static_cast<WriteUnitType>(value(options, WriteOptionKey::UNIT_TYPE, Val(static_cast<int>(WriteUnitType::PER_PART))).toInt());
    IF_ASSERT_FAILED(supportsUnitType(unitType)) {
        return Ret(Ret::Code::NotSupported);
    }

    notation::INotationPtrList notations = project::resolveNotations(project, options);
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    if (unitType == WriteUnitType::PER_PART) {
        IF_ASSERT_FAILED(notations.size() == 1) {
            return Ret(Ret::Code::NotSupported);
        }

        INotationPtr notation = notations.front();

        QByteArray qdata;
        QBuffer buf(&qdata);
        buf.open(QIODevice::WriteOnly);

        QPdfWriter pdfWriter(&buf);
        preparePdfWriter(pdfWriter, notation->projectWorkTitleAndPartName(), notation->painting()->pageSizeInch().toQSizeF());

        Painter painter(&pdfWriter, "pdfwriter");
        if (!painter.isActive()) {
            return false;
        }

        const bool TRANSPARENT_BACKGROUND = value(options, WriteOptionKey::TRANSPARENT_BACKGROUND,
                                                         Val(configuration()->exportPdfWithTransparentBackground())).toBool();

        INotationPainting::Options opt;
        opt.deviceDpi = pdfWriter.logicalDpiX();
        opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };
        opt.printPageBackground = !TRANSPARENT_BACKGROUND;

        if (contains(options, WriteOptionKey::PAGE_NUMBER)) {
            opt.fromPage = options.at(WriteOptionKey::PAGE_NUMBER).toInt();
            opt.toPage = opt.fromPage;
        }

        notation->painting()->paintPdf(&painter, opt);

        painter.endDraw();

        ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
        destinationDevice.write(data);

        return true;
    }

    // MULTI_PART
    IF_ASSERT_FAILED(unitType == WriteUnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    INotationPtr firstNotation = notations.front();

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    QPdfWriter pdfWriter(&buf);
    preparePdfWriter(pdfWriter, firstNotation->projectWorkTitle(), firstNotation->painting()->pageSizeInch().toQSizeF());

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    const bool TRANSPARENT_BACKGROUND = value(options, WriteOptionKey::TRANSPARENT_BACKGROUND,
                                                     Val(configuration()->exportPdfWithTransparentBackground())).toBool();

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };
    opt.printPageBackground = !TRANSPARENT_BACKGROUND;

    for (const auto& notation : notations) {
        IF_ASSERT_FAILED(notation) {
            return make_ret(Ret::Code::UnknownError);
        }

        if (notation != firstNotation) {
            QSizeF size = notation->painting()->pageSizeInch().toQSizeF();
            pdfWriter.setPageSize(QPageSize(size, QPageSize::Inch));
            pdfWriter.newPage();
        }

        notation->painting()->paintPdf(&painter, opt);
    }

    painter.endDraw();

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

    return true;
}

Ret PdfWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();

    return ret;
}

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, const QString& title, const QSizeF& size) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator(QString("MuseScore Studio Version: ") + application()->version().toString().toQString());
    pdfWriter.setTitle(title);
    pdfWriter.setPageMargins(QMarginsF());
    pdfWriter.setPageLayout(QPageLayout(QPageSize(size, QPageSize::Inch), QPageLayout::Orientation::Portrait, QMarginsF()));

    if (configuration()->exportPdfWithGrayscale()) {
        pdfWriter.setColorModel(QPdfWriter::ColorModel::Grayscale);
    } else {
        pdfWriter.setColorModel(QPdfWriter::ColorModel::Auto);
    }
}
