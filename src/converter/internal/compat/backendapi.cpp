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
#include "backendapi.h"

#include <QString>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "log.h"
#include "backendjsonwriter.h"

using namespace mu;
using namespace mu::converter;
using namespace mu::notation;

static const std::string PNG_WRITER_NAME = "png";
static const std::string SVG_WRITER_NAME = "svg";
static const std::string SEGMENTS_POSITIONS_WRITER_NAME = "sposXML";
static const std::string MEASURES_POSITIONS_WRITER_NAME = "mposXML";
static const std::string PDF_WRITER_NAME = "pdf";
static const std::string MIDI_WRITER_NAME = "midi";
static const std::string MUSICXML_WRITER_NAME = "mxml";
static const std::string META_DATA_WRITER_NAME = "metadata";

Ret BackendApi::exportScoreMedia(const io::path& in, const io::path& out, const io::path& highlightConfigPath)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    INotationPtr notation = openScoreRetVal.val ? openScoreRetVal.val->notation() : nullptr;
    if (!notation) {
        return make_ret(Ret::Code::InternalError);
    }

    notation->setViewMode(ViewMode::PAGE);

    bool result = true;

    BackendJsonWriter jsonWriter(out);

    result &= exportScorePngs(notation, jsonWriter);
    result &= exportScoreSvgs(notation, jsonWriter, highlightConfigPath);
    result &= exportScoreElementsPositions(SEGMENTS_POSITIONS_WRITER_NAME, notation, jsonWriter);
    result &= exportScoreElementsPositions(MEASURES_POSITIONS_WRITER_NAME, notation, jsonWriter);
    result &= exportScorePdf(notation, jsonWriter);
    result &= exportScoreMidi(notation, jsonWriter);
    result &= exportScoreMusicXML(notation, jsonWriter);
    result &= exportScoreMetaData(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

RetVal<notation::IMasterNotationPtr> BackendApi::openScore(const io::path& path)
{
    TRACEFUNC

    auto notation = notationCreator()->newMasterNotation();
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = notation->load(path);
    if (!ret) {
        LOGE() << "failed load: " << path << ", ret: " << ret.toString();
        return make_ret(Ret::Code::InternalError);
    }

    return RetVal<notation::IMasterNotationPtr>::make_ok(notation);
}

PageList BackendApi::pages(const INotationPtr notation)
{
    auto elements = notation->elements();
    if (!elements) {
        return {};
    }

    return elements->pages();
}

QVariantMap BackendApi::readNotesColors(const io::path& filePath)
{
    TRACEFUNC

    if (filePath.empty()) {
        return QVariantMap();
    }

    RetVal<QByteArray> fileData = fileSystem()->readFile(filePath);
    if (!fileData.ret) {
        LOGW() << fileData.ret.toString();
        return QVariantMap();
    }

    QString content(fileData.val);

    QJsonDocument document = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = document.object();
    QJsonArray colors = obj.value("highlight").toArray();

    QVariantMap result;

    for (const QJsonValue colorObj: colors) {
        QJsonObject cobj = colorObj.toObject();
        QJsonArray notesIndexes = cobj.value("notes").toArray();
        QColor notesColor = QColor(cobj.value("color").toString());

        for (const QJsonValue index: notesIndexes) {
            result[index.toString()] = notesColor;
        }
    }

    return result;
}

Ret BackendApi::exportScorePngs(const INotationPtr notation, BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    auto pngWriter = writers()->writer(PNG_WRITER_NAME);
    if (!pngWriter) {
        LOGW() << "Not found writer " << PNG_WRITER_NAME;
        return make_ret(Ret::Code::InternalError);
    }

    jsonWriter.addKey("pngs");
    jsonWriter.openArray();

    PageList notationPages = pages(notation);

    bool result = true;
    for (size_t i = 0; i < notationPages.size(); ++i) {
        QByteArray pngData;
        QBuffer pngDevice(&pngData);
        pngDevice.open(QIODevice::ReadWrite);

        INotationWriter::Options options {
            { INotationWriter::OptionKey::PAGE_NUMBER, Val(static_cast<int>(i)) },
            { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(false) }
        };

        Ret writeRet = pngWriter->write(notation, pngDevice, options);
        if (!writeRet) {
            LOGW() << writeRet.toString();
            result = false;
        }

        bool lastArrayValue = ((notationPages.size() - 1) == i);
        jsonWriter.addValue(pngData.toBase64(), lastArrayValue);
    }

    jsonWriter.closeArray();

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreSvgs(const INotationPtr notation, BackendJsonWriter& jsonWriter, const io::path& highlightConfigPath)
{
    TRACEFUNC

    auto svgWriter = writers()->writer(SVG_WRITER_NAME);
    if (!svgWriter) {
        LOGW() << "Not found writer " << SVG_WRITER_NAME;
        return make_ret(Ret::Code::InternalError);
    }

    jsonWriter.addKey("svgs");
    jsonWriter.openArray();

    PageList notationPages = pages(notation);
    QVariantMap notesColors = readNotesColors(highlightConfigPath);

    bool result = true;
    for (size_t i = 0; i < notationPages.size(); ++i) {
        QByteArray svgData;
        QBuffer svgDevice(&svgData);
        svgDevice.open(QIODevice::ReadWrite);

        INotationWriter::Options options {
            { INotationWriter::OptionKey::PAGE_NUMBER, Val(static_cast<int>(i)) },
            { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(false) },
            { INotationWriter::OptionKey::NOTES_COLORS, Val(notesColors) }
        };

        Ret writeRet = svgWriter->write(notation, svgDevice, options);
        if (!writeRet) {
            LOGW() << writeRet.toString();
            result = false;
        }

        bool lastArrayValue = ((notationPages.size() - 1) == i);
        jsonWriter.addValue(svgData.toBase64(), lastArrayValue);
    }

    jsonWriter.closeArray();

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const INotationPtr notation,
                                             BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    return processWriter(elementsPositionsWriterName, notation, jsonWriter);
}

Ret BackendApi::exportScorePdf(const INotationPtr notation, BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    return processWriter(PDF_WRITER_NAME, notation, jsonWriter);
}

Ret BackendApi::exportScoreMidi(const INotationPtr notation, BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    return processWriter(MIDI_WRITER_NAME, notation, jsonWriter);
}

Ret BackendApi::exportScoreMusicXML(const INotationPtr notation, BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    return processWriter(MUSICXML_WRITER_NAME, notation, jsonWriter);
}

Ret BackendApi::exportScoreMetaData(const INotationPtr notation, BackendJsonWriter& jsonWriter)
{
    TRACEFUNC

    return processWriter(META_DATA_WRITER_NAME, notation, jsonWriter);
}

Ret BackendApi::processWriter(const std::string& writerName, const INotationPtr notation,
                              BackendJsonWriter& jsonWriter)
{
    auto writer = writers()->writer(writerName);
    if (!writer) {
        LOGW() << "Not found writer " << writerName;
        return make_ret(Ret::Code::InternalError);
    }

    QByteArray data;
    QBuffer device(&data);
    device.open(QIODevice::ReadWrite);

    Ret writeRet = writer->write(notation, device);
    if (!writeRet) {
        LOGW() << writeRet.toString();
        return writeRet;
    }

    jsonWriter.addKey(writerName.c_str());
    jsonWriter.addValue(data.toBase64());

    return make_ret(Ret::Code::Ok);
}
