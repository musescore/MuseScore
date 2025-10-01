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

#include <QPdfWriter>
#include <QBuffer>
#include <QDateTime>

#include "engraving/dom/masterscore.h"
#include "project/types/projectmeta.h"
#include "project/inotationproject.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;

std::vector<INotationWriter::UnitType> PdfWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART, UnitType::MULTI_PART };
}

Ret PdfWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options& options)
{
    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    QPdfWriter pdfWriter(&buf);
    preparePdfWriter(pdfWriter, notation, notation->painting()->pageSizeInch().toQSizeF());

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
                                                    Val(configuration()->exportPdfWithTransparentBackground())).toBool();

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };
    opt.printPageBackground = !TRANSPARENT_BACKGROUND;

    auto pageNumIt = options.find(OptionKey::PAGE_NUMBER);
    if (pageNumIt != options.end()) {
        opt.fromPage = pageNumIt->second.toInt();
        opt.toPage = opt.fromPage;
    }

    notation->painting()->paintPdf(&painter, opt);

    painter.endDraw();

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

    return true;
}

Ret PdfWriter::writeList(const INotationPtrList& notations, io::IODevice& destinationDevice, const Options& options)
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

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    QPdfWriter pdfWriter(&buf);
    preparePdfWriter(pdfWriter, firstNotation, firstNotation->painting()->pageSizeInch().toQSizeF());

    Painter painter(&pdfWriter, "pdfwriter");
    if (!painter.isActive()) {
        return false;
    }

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
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

// Extract project metadata from notation's project or notation fallback
ProjectMeta PdfWriter::getProjectMetadata(INotationPtr notation) const
{
    if (notation) {
        auto project = notation->project();
        if (project) {
            return project->metaInfo();
        }

        // Fallback to notation title if project not available
        ProjectMeta meta;
        meta.title = notation->projectWorkTitle();
        return meta;
    }

    return ProjectMeta();
}

// Generate XMP metadata for PDF embedding with project information
QByteArray PdfWriter::generateXmpMetadata(const ProjectMeta& meta) const
{
    QString xmpTemplate
        =
            R"(<?xml version="1.0" encoding="UTF-8"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/">
  <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
    <rdf:Description rdf:about=""
        xmlns:dc="http://purl.org/dc/elements/1.1/"
        xmlns:xmp="http://ns.adobe.com/xap/1.0/"
        xmlns:pdf="http://ns.adobe.com/pdf/1.3/"
        xmlns:pdfx="http://ns.adobe.com/pdfx/1.3/">
      <dc:title>%1</dc:title>
      <dc:creator>%2</dc:creator>
      <dc:subject>%3</dc:subject>
      <pdf:Copyright>%4</pdf:Copyright>
      <xmp:CreateDate>%5</xmp:CreateDate>
      <xmp:ModifyDate>%5</xmp:ModifyDate>
      <pdf:Author>%6</pdf:Author>
      <pdfx:Composer>%7</pdfx:Composer>
      <pdfx:Arranger>%8</pdfx:Arranger>
      <pdfx:Translator>%9</pdfx:Translator>
      <pdfx:Lyricist>%10</pdfx:Lyricist>
    </rdf:Description>
  </rdf:RDF>
</x:xmpmeta>)";

    QString author = meta.composer;
    QString creator = QString("MuseScore Studio Version: ") + application()->version().toString().toQString();
    QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Populate XMP template with project metadata
    QString xmpData = xmpTemplate.arg(meta.title.toHtmlEscaped(),
                                      creator.toHtmlEscaped(),
                                      meta.subtitle.toHtmlEscaped(),
                                      meta.copyright.toHtmlEscaped(),
                                      currentDateTime,
                                      author.toHtmlEscaped(),
                                      meta.composer.toHtmlEscaped(),
                                      meta.arranger.toHtmlEscaped(),
                                      meta.translator.toHtmlEscaped(),
                                      meta.lyricist.toHtmlEscaped());

    return xmpData.toUtf8();
}

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, INotationPtr notation, const QSizeF& size) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());

    // Always set basic metadata (original behavior)
    QString title = notation->projectWorkTitleAndPartName();
    pdfWriter.setCreator(QString("MuseScore Studio Version: ") + application()->version().toString().toQString());
    pdfWriter.setTitle(title);

    // Check if user wants to embed extended metadata
    bool embedMetadata = configuration()->exportPdfWithEmbeddedMetadata();

    if (embedMetadata) {
        // Add comprehensive XMP metadata
        ProjectMeta meta = getProjectMetadata(notation);

        // Use project title if available, otherwise use the title already set
        if (!meta.title.isEmpty()) {
            pdfWriter.setTitle(meta.title);
        }

        QString author = meta.composer;

        // Set author metadata (Qt 6.9+ feature)
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        pdfWriter.setAuthor(author);
#endif

        // Embed comprehensive XMP metadata
        QByteArray xmpMetadata = generateXmpMetadata(meta);
        pdfWriter.setDocumentXmpMetadata(xmpMetadata);
    }

    pdfWriter.setPageMargins(QMarginsF());
    pdfWriter.setPageLayout(QPageLayout(QPageSize(size, QPageSize::Inch), QPageLayout::Orientation::Portrait, QMarginsF()));

    if (configuration()->exportPdfWithGrayscale()) {
        pdfWriter.setColorModel(QPdfWriter::ColorModel::Grayscale);
    } else {
        pdfWriter.setColorModel(QPdfWriter::ColorModel::Auto);
    }
}
