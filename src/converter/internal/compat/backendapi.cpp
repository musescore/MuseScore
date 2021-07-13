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

#include <stdio.h>

#include <QString>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QRandomGenerator>

#include "libmscore/excerpt.h"

#include "log.h"
#include "backendjsonwriter.h"
#include "notationmeta.h"

using namespace mu;
using namespace mu::converter;
using namespace mu::notation;
using namespace mu::io;

static const std::string PNG_WRITER_NAME = "png";
static const std::string SVG_WRITER_NAME = "svg";
static const std::string SEGMENTS_POSITIONS_WRITER_NAME = "sposXML";
static const std::string MEASURES_POSITIONS_WRITER_NAME = "mposXML";
static const std::string PDF_WRITER_NAME = "pdf";
static const std::string MIDI_WRITER_NAME = "midi";
static const std::string MUSICXML_WRITER_NAME = "mxml";
static const std::string META_DATA_NAME = "metadata";

static constexpr bool ADD_SEPARATOR = true;
static constexpr auto NO_STYLE = "";

Ret BackendApi::exportScoreMedia(const io::path& in, const io::path& out, const io::path& highlightConfigPath, const io::path& stylePath,
                                 bool forceMode)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in, stylePath, forceMode);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    INotationPtr notation = openScoreRetVal.val->notation();

    bool result = true;

    QFile outputFile;
    openOutputFile(outputFile, out);

    BackendJsonWriter jsonWriter(&outputFile);

    result &= exportScorePngs(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreSvgs(notation, highlightConfigPath, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreElementsPositions(SEGMENTS_POSITIONS_WRITER_NAME, notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreElementsPositions(MEASURES_POSITIONS_WRITER_NAME, notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScorePdf(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMidi(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMusicXML(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMetaData(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreMeta(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in, stylePath, forceMode);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    INotationPtr notation = openScoreRetVal.val->notation();

    QFile outputFile;
    openOutputFile(outputFile, out);

    BackendJsonWriter jsonWriter(&outputFile);

    bool result = exportScoreMetaData(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreParts(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in, stylePath, forceMode);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    INotationPtr notation = openScoreRetVal.val->notation();

    QFile outputFile;
    openOutputFile(outputFile, out);

    Ret ret = doExportScoreParts(notation, outputFile);

    outputFile.close();

    return ret;
}

Ret BackendApi::exportScorePartsPdfs(const io::path& in, const io::path& out, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in, stylePath, forceMode);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    QFile outputFile;
    openOutputFile(outputFile, out);

    std::string scoreFileName = io::dirpath(in).toStdString() + "/" + io::completebasename(in).toStdString() + ".pdf";

    Ret ret = doExportScorePartsPdfs(openScoreRetVal.val, outputFile, scoreFileName);

    outputFile.close();

    return ret;
}

Ret BackendApi::exportScoreTranspose(const io::path& in, const io::path& out, const std::string& optionsJson, const io::path& stylePath,
                                     bool forceMode)
{
    TRACEFUNC

    RetVal<IMasterNotationPtr> openScoreRetVal = openScore(in, stylePath, forceMode);
    if (!openScoreRetVal.ret) {
        return openScoreRetVal.ret;
    }

    INotationPtr notation = openScoreRetVal.val->notation();
    Ret ret = applyTranspose(notation, optionsJson);
    if (!ret) {
        return ret;
    }

    QFile outputFile;
    openOutputFile(outputFile, out);

    BackendJsonWriter jsonWriter(&outputFile);

    bool result = doExportScoreTranspose(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::openOutputFile(QFile& file, const io::path& out)
{
    bool ok = false;
    if (!out.empty()) {
        file.setFileName(out.toQString());
        ok = file.open(QFile::WriteOnly);
    } else {
        ok = file.open(stdout, QFile::WriteOnly);
    }

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

RetVal<notation::INotationProjectPtr> BackendApi::openProject(const io::path& path,
                                                              const io::path& stylePath,
                                                              bool forceMode)
{
    TRACEFUNC

    auto notationProject = notationCreator()->newNotationProject();
    IF_ASSERT_FAILED(notationProject) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = notationProject->load(path, stylePath, forceMode);
    if (!ret) {
        LOGE() << "failed load: " << path << ", ret: " << ret.toString();
        return make_ret(Ret::Code::InternalError);
    }

    INotationPtr notation = notationProject->masterNotation()->notation();
    if (!notation) {
        return make_ret(Ret::Code::InternalError);
    }

    notation->setViewMode(ViewMode::PAGE);

    return RetVal<notation::INotationProjectPtr>::make_ok(notationProject);
}

RetVal<notation::IMasterNotationPtr> BackendApi::openScore(const io::path& path, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC
    RetVal<notation::INotationProjectPtr> prj = openProject(path, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    return RetVal<notation::IMasterNotationPtr>::make_ok(prj.val->masterNotation());
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

Ret BackendApi::exportScorePngs(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
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

    jsonWriter.closeArray(addSeparator);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreSvgs(const INotationPtr notation, const io::path& highlightConfigPath, BackendJsonWriter& jsonWriter,
                                bool addSeparator)
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
        jsonWriter.addValue(svgData.toBase64(), !lastArrayValue);
    }

    jsonWriter.closeArray(addSeparator);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const INotationPtr notation,
                                             BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(elementsPositionsWriterName, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    jsonWriter.addKey(elementsPositionsWriterName.c_str());
    jsonWriter.addValue(writerRetVal.val, addSeparator, false);

    return make_ret(Ret::Code::Ok);
}

Ret BackendApi::exportScorePdf(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(PDF_WRITER_NAME, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    jsonWriter.addKey(PDF_WRITER_NAME.c_str());
    jsonWriter.addValue(writerRetVal.val, addSeparator);

    return make_ret(Ret::Code::Ok);
}

Ret BackendApi::exportScorePdf(const INotationPtr notation, Device& destinationDevice)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(PDF_WRITER_NAME, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    bool ok = destinationDevice.write(QJsonDocument::fromJson(writerRetVal.val).toJson(QJsonDocument::Compact)) != -1;
    destinationDevice.close();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreMidi(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(MIDI_WRITER_NAME, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    jsonWriter.addKey(MIDI_WRITER_NAME.c_str());
    jsonWriter.addValue(writerRetVal.val, addSeparator);

    return make_ret(Ret::Code::Ok);
}

Ret BackendApi::exportScoreMusicXML(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(MUSICXML_WRITER_NAME, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    jsonWriter.addKey(MUSICXML_WRITER_NAME.c_str());
    jsonWriter.addValue(writerRetVal.val, addSeparator);

    return make_ret(Ret::Code::Ok);
}

Ret BackendApi::exportScoreMetaData(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<std::string> meta = NotationMeta::metaJson(notation);
    if (!meta.ret) {
        LOGW() << meta.ret.toString();
        return meta.ret;
    }

    jsonWriter.addKey(META_DATA_NAME.c_str());
    jsonWriter.addValue(QString::fromStdString(meta.val).toUtf8(), addSeparator, true);

    return make_ret(Ret::Code::Ok);
}

mu::RetVal<QByteArray> BackendApi::processWriter(const std::string& writerName, const INotationPtr notation)
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

    RetVal<QByteArray> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = data.toBase64();

    device.close();

    return result;
}

mu::RetVal<QByteArray> BackendApi::processWriter(const std::string& writerName, const INotationPtrList notations,
                                                 const INotationWriter::Options& options)
{
    auto writer = writers()->writer(writerName);
    if (!writer) {
        LOGW() << "Not found writer " << writerName;
        return make_ret(Ret::Code::InternalError);
    }

    QByteArray data;
    QBuffer device(&data);
    device.open(QIODevice::ReadWrite);

    Ret writeRet = writer->writeList(notations, device, options);
    if (!writeRet) {
        LOGW() << writeRet.toString();
        return writeRet;
    }

    RetVal<QByteArray> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = data.toBase64();

    device.close();

    return result;
}

Ret BackendApi::doExportScoreParts(const notation::INotationPtr notation, Device& destinationDevice)
{
    Ms::Score* score = notation->elements()->msScore();

    QJsonArray partsObjList;
    QJsonArray partsMetaList;
    QJsonArray partsTitles;

    for (const Ms::Excerpt* excerpt : score->excerpts()) {
        Ms::Score* part = excerpt->partScore();
        QMap<QString, QString> partMetaTags = part->metaTags();

        QJsonValue partTitle(part->title());
        partsTitles << partTitle;

        QVariantMap meta;
        for (const QString& key: partMetaTags.keys()) {
            meta[key] = partMetaTags[key];
        }

        QJsonValue partMetaObj = QJsonObject::fromVariantMap(meta);
        partsMetaList << partMetaObj;

        std::string fileName = io::escapeFileName(part->title().toStdString()).toStdString() + ".mscz";
        QJsonValue partObj(QString::fromLatin1(scorePartJson(part, fileName).val));
        partsObjList << partObj;
    }

    QJsonObject json;
    json["parts"] = partsTitles;
    json["partsMeta"] = partsMetaList;
    json["partsBin"] = partsObjList;

    bool ok = destinationDevice.write(QJsonDocument(json).toJson(QJsonDocument::Compact)) != -1;
    destinationDevice.close();

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::doExportScorePartsPdfs(const IMasterNotationPtr masterNotation, Device& destinationDevice,
                                       const std::string& scoreFileName)
{
    QJsonObject jsonForPdfs;
    jsonForPdfs["score"] = QString::fromStdString(scoreFileName);
    QByteArray scoreBin = processWriter(PDF_WRITER_NAME, masterNotation->notation()).val;
    jsonForPdfs["scoreBin"] = QString::fromLatin1(scoreBin);

    INotationPtrList notations;

    QJsonArray partsArray;
    QJsonArray partsNamesArray;
    for (IExcerptNotationPtr e : masterNotation->excerpts().val) {
        QJsonValue partNameVal(e->metaInfo().title);
        partsNamesArray.append(partNameVal);

        QByteArray partBin = processWriter(PDF_WRITER_NAME, e->notation()).val;
        QJsonValue partVal(QString::fromLatin1(partBin));
        partsArray.append(partVal);

        notations.push_back(e->notation());
    }

    jsonForPdfs["parts"] = partsNamesArray;
    jsonForPdfs["partsBin"] = partsArray;

    jsonForPdfs["scoreFullPostfix"] = QString("-Score_and_parts") + ".pdf";

    INotationWriter::Options options {
        { INotationWriter::OptionKey::UNIT_TYPE, Val(static_cast<int>(INotationWriter::UnitType::MULTI_PART)) }
    };

    QByteArray fullScoreData = processWriter(PDF_WRITER_NAME, notations, options).val;
    jsonForPdfs["scoreFullBin"] = QString::fromLatin1(fullScoreData.toBase64());

    QJsonDocument jsonDoc(jsonForPdfs);
    bool ok = destinationDevice.write(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact)) != -1;

    return ok;
}

Ret BackendApi::doExportScoreTranspose(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    Ms::Score* score = notation->elements()->msScore();

    jsonWriter.addKey("mscz");

    std::string fileNumber = std::to_string(QRandomGenerator::global()->generate() % 1000000);
    std::string fileName = score->title().toStdString() + "_transposed." + fileNumber + ".mscx";

    RetVal<QByteArray> scoreJson = scorePartJson(score, fileName);
    if (!scoreJson.ret) {
        LOGW() << "Transpose: adding mscz failed";
    }

    jsonWriter.addValue(scoreJson.val, ADD_SEPARATOR);

    Ret ret = exportScorePdf(notation, jsonWriter, addSeparator);
    return ret;
}

RetVal<QByteArray> BackendApi::scorePartJson(Ms::Score* score, const std::string& fileName)
{
    QByteArray scoreData;
    QBuffer buf(&scoreData);

    mu::engraving::MsczWriter msczWriter(&buf);
    msczWriter.setFilePath(QString::fromStdString(fileName));
    msczWriter.open();

    bool ok = score->writeMscz(msczWriter);
    if (!ok) {
        LOGW() << "Error save mscz file";
    }
    msczWriter.close();

    RetVal<QByteArray> result;
    result.ret = ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
    result.val = scoreData.toBase64();

    return result;
}

RetVal<TransposeOptions> BackendApi::parseTransposeOptions(const std::string& optionsJson)
{
    TransposeOptions options;

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(optionsJson).toUtf8());
    if (!doc.isObject()) {
        LOGW() << "Transpose options JSON is not an object: " << optionsJson;
        return make_ret(Ret::Code::InternalError);
    }

    QJsonObject optionsObj = doc.object();

    const QString modeName = optionsObj["mode"].toString();
    if (modeName == "by_key" || modeName == "to_key") { // "by_key" for backwards compatibility
        options.mode = TransposeMode::TO_KEY;
    } else if (modeName == "by_interval") {
        options.mode = TransposeMode::BY_INTERVAL;
    } else if (modeName == "diatonically") {
        options.mode = TransposeMode::DIATONICALLY;
    } else {
        LOGW() << "Transpose: invalid \"mode\" option: " << modeName;
        return make_ret(Ret::Code::InternalError);
    }

    const QString directionName = optionsObj["direction"].toString();
    if (directionName == "up") {
        options.direction = TransposeDirection::UP;
    } else if (directionName == "down") {
        options.direction = TransposeDirection::DOWN;
    } else if (directionName == "closest") {
        options.direction = TransposeDirection::CLOSEST;
    } else {
        LOGW() << "Transpose: invalid \"direction\" option: " << directionName;
        return make_ret(Ret::Code::InternalError);
    }

    constexpr int defaultKey = int(Key::INVALID);
    const Key targetKey = Key(optionsObj["targetKey"].toInt(defaultKey));
    if (options.mode == TransposeMode::TO_KEY) {
        const bool targetKeyValid = int(Key::MIN) <= int(targetKey) && int(targetKey) <= int(Key::MAX);
        if (!targetKeyValid) {
            LOGW() << "Transpose: invalid targetKey: " << int(targetKey);
            return make_ret(Ret::Code::InternalError);
        }
    }

    const int transposeInterval = optionsObj["transposeInterval"].toInt(-1);
    constexpr int INTERVAL_LIST_SIZE = 26;

    if (options.mode != TransposeMode::TO_KEY) {
        const bool transposeIntervalValid = -1 < transposeInterval && transposeInterval < INTERVAL_LIST_SIZE;
        if (!transposeIntervalValid) {
            LOGW() << "Transpose: invalid transposeInterval: " << transposeInterval;
            return make_ret(Ret::Code::InternalError);
        }
    }

    options.needTransposeKeys = optionsObj["transposeKeySignatures"].toBool();
    options.needTransposeChordNames = optionsObj["transposeChordNames"].toBool();
    options.needTransposeDoubleSharpsFlats = optionsObj["useDoubleSharpsFlats"].toBool();

    RetVal<TransposeOptions> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = options;

    return result;
}

Ret BackendApi::applyTranspose(const INotationPtr notation, const std::string& optionsJson)
{
    RetVal<TransposeOptions> options = parseTransposeOptions(optionsJson);
    if (!options.ret) {
        return options.ret;
    }

    INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    if (!interaction) {
        return make_ret(Ret::Code::InternalError);
    }

    bool ok = interaction->transpose(options.val);
    if (!ok) {
        LOGW() << "Error transpose";
    }

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::updateSource(const io::path& in, const std::string& newSource, bool forceMode)
{
    RetVal<INotationProjectPtr> notation = openProject(in, NO_STYLE, forceMode);
    if (!notation.ret) {
        return notation.ret;
    }

    Meta meta = notation.val->masterNotation()->metaInfo();
    meta.source = QString::fromStdString(newSource);

    notation.val->masterNotation()->setMetaInfo(meta);

    return notation.val->save();
}
