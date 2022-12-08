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

#include "pdfwriter.h"

#include <QPdfWriter>

#include "libmscore/masterscore.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;

std::vector<INotationWriter::UnitType> PdfWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART, UnitType::MULTI_PART };
}

mu::Ret PdfWriter::write(INotationPtr notation, QIODevice& destinationDevice, const Options& options)
{
    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, notation->projectWorkTitleAndPartName(), notation->painting()->pageSizeInch().toQSizeF());

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };

    notation->painting()->paintPdf(&painter, opt);

    painter.endDraw();

    return true;
}

mu::Ret PdfWriter::writeList(const INotationPtrList& notations, QIODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    INotationPtr firstNotation = notations.front();
    IF_ASSERT_FAILED(firstNotation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, firstNotation->projectWorkTitle(), firstNotation->painting()->pageSizeInch().toQSizeF());

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };

    for (auto notation : notations) {
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

    return true;
}

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, const QString& title, const QSizeF& size) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator("MuseScore Version: " VERSION);
    pdfWriter.setTitle(title);
    pdfWriter.setPageMargins(QMarginsF());
    pdfWriter.setPageLayout(QPageLayout(QPageSize(size, QPageSize::Inch), QPageLayout::Orientation::Portrait, QMarginsF()));
}
