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

#include "libmscore/score.h"
#include "engraving/draw/qpainterprovider.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::io;
using namespace Ms;

std::vector<INotationWriter::UnitType> PdfWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART, UnitType::MULTI_PART };
}

mu::Ret PdfWriter::write(INotationPtr notation, io::Device& destinationDevice, const Options& options)
{
    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, documentTitle(*score));

    mu::draw::Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    doWrite(pdfWriter, painter, score);

    painter.endDraw();

    return true;
}

mu::Ret PdfWriter::writeList(const INotationPtrList& notations, io::Device& destinationDevice, const Options& options)
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

    Ms::Score* firstScore = firstNotation->elements()->msScore();
    IF_ASSERT_FAILED(firstScore) {
        return make_ret(Ret::Code::UnknownError);
    }

    QPdfWriter pdfWriter(&destinationDevice);
    preparePdfWriter(pdfWriter, documentTitle(*(firstScore->masterScore())));

    mu::draw::Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    for (auto notation : notations) {
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

        doWrite(pdfWriter, painter, score);
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

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, const QString& title) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator("MuseScore Version: " VERSION);
    pdfWriter.setTitle(title);
    pdfWriter.setPageMargins(QMarginsF());
}

void PdfWriter::doWrite(QPdfWriter& pdfWriter, mu::draw::Painter& painter, Score* score) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    score->setPrinting(true);
    MScore::pdfPrinting = true;

    QSizeF size(score->styleD(Sid::pageWidth), score->styleD(Sid::pageHeight));
    painter.setAntialiasing(true);
    painter.setViewport(RectF(0.0, 0.0, size.width() * pdfWriter.logicalDpiX(), size.height() * pdfWriter.logicalDpiY()));
    painter.setWindow(RectF(0.0, 0.0, size.width() * DPI, size.height() * DPI));

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
