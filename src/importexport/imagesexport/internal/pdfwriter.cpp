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

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/draw/qpainterprovider.h"

#include <QPdfWriter>

using namespace mu::iex::imagesexport;
using namespace mu::notation;
using namespace mu::system;
using namespace Ms;

std::vector<WriterUnitType> PdfWriter::supportedUnitTypes() const
{
    return { WriterUnitType::PER_PART, WriterUnitType::MULTI_PART };
}

mu::Ret PdfWriter::write(const INotationPtrList& notations, system::IODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    auto firstNotation = notations.front();
    IF_ASSERT_FAILED(firstNotation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* firstScore = firstNotation->elements()->msScore();
    IF_ASSERT_FAILED(firstScore) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator("MuseScore Version: " VERSION);
    pdfWriter.setTitle(documentTitle(*(firstScore->masterScore())));
    pdfWriter.setPageMargins(QMarginsF());

    mu::draw::Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    WriterUnitType unitType = unitTypeFromOptions(options);
    INotationPtrList notationsCorrected = notations;
    if (unitType == WriterUnitType::PER_PART && notations.size() > 1) {
        notationsCorrected = { notations.front() };
    }

    for (auto notation : notationsCorrected) {
        IF_ASSERT_FAILED(notation) {
            return make_ret(Ret::Code::UnknownError);
        }
        Ms::Score* score = notation->elements()->msScore();
        IF_ASSERT_FAILED(score) {
            return make_ret(Ret::Code::UnknownError);
        }

        if (score != firstScore) {
            pdfWriter.newPage();
        }

        score->setPrinting(true);
        MScore::pdfPrinting = true;

        QSizeF size(score->styleD(Sid::pageWidth), score->styleD(Sid::pageHeight));
        painter.setAntialiasing(true);
        painter.setViewport(QRect(0.0, 0.0, size.width() * pdfWriter.logicalDpiX(),
                                  size.height() * pdfWriter.logicalDpiY()));
        painter.setWindow(QRect(0.0, 0.0, size.width() * DPI, size.height() * DPI));

        double pixelRationBackup = MScore::pixelRatio;
        MScore::pixelRatio = DPI / pdfWriter.logicalDpiX();

        for (int pageNumber = 0; pageNumber < score->npages(); ++pageNumber) {
            if (pageNumber > 0) {
                pdfWriter.newPage();
            }

            score->print(&painter, pageNumber);
        }

        score->setPrinting(false);
        MScore::pixelRatio = pixelRationBackup;
        MScore::pdfPrinting = false;
    }

    painter.endDraw();

    return true;
}

QString PdfWriter::documentTitle(const Score& score) const
{
    QString title = score.metaTag("workTitle");
    if (title.isEmpty()) { // workTitle unset?
        title = score.masterScore()->title(); // fall back to (master)score's tab title
    }

    if (!score.isMaster()) { // excerpt?
        QString partName = score.metaTag("partName");
        if (partName.isEmpty()) { // partName unset?
            partName = score.title(); // fall back to excerpt's tab title
        }

        title += " - " + partName;
    }

    return title;
}
