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

#include "pdfwriter.h"

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/draw/qpainterprovider.h"

#include <QPdfWriter>

using namespace mu::iex::imagesexport;
using namespace mu::system;
using namespace Ms;

mu::Ret PdfWriter::write(const notation::INotationPtr notation, IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    Ms::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    score->setPrinting(true);
    MScore::pdfPrinting = true;

    QPdfWriter pdfWriter(&destinationDevice);
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    pdfWriter.setCreator("MuseScore Version: " VERSION);
    pdfWriter.setTitle(documentTitle(*score));
    pdfWriter.setPageMargins(QMarginsF());

    mu::draw::Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

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

    painter.endDraw();
    score->setPrinting(false);
    MScore::pixelRatio = pixelRationBackup;
    MScore::pdfPrinting = false;

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
