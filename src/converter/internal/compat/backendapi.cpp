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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QRandomGenerator>

#include "io/buffer.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/libmscore/excerpt.h"
#include "engraving/rw/mscsaver.h"

#include "backendjsonwriter.h"
#include "notationmeta.h"

#include "muversion.h"
#include "log.h"

using namespace mu;
using namespace mu::converter;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;
using namespace mu::io;

static const std::string PNG_WRITER_NAME = "png";
static const std::string SVG_WRITER_NAME = "svg";
static const std::string SEGMENTS_POSITIONS_WRITER_NAME = "spos";
static const std::string SEGMENTS_POSITIONS_TAG_NAME = "sposXML";
static const std::string MEASURES_POSITIONS_WRITER_NAME = "mpos";
static const std::string MEASURES_POSITIONS_TAG_NAME = "mposXML";
static const std::string PDF_WRITER_NAME = "pdf";
static const std::string MIDI_WRITER_NAME = "midi";
static const std::string MUSICXML_WRITER_NAME = "mxl";
static const std::string MUSICXML_JSON_NAME = "mxml";
static const std::string META_DATA_NAME = "metadata";
static const std::string DEV_INFO_NAME = "devinfo";

static constexpr bool ADD_SEPARATOR = true;
static constexpr auto NO_STYLE = "";

Ret BackendApi::exportScoreMedia(const io::path_t& in, const io::path_t& out, const io::path_t& highlightConfigPath,
                                 const io::path_t& stylePath,
                                 bool forceMode)
{
    TRACEFUNC

    RetVal<INotationProjectPtr> prj = openProject(in, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    INotationPtr notation = prj.val->masterNotation()->notation();

    bool result = true;

    QFile outputFile;
    openOutputFile(outputFile, out);

    BackendJsonWriter jsonWriter(&outputFile);

    result &= exportScorePngs(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreSvgs(notation, highlightConfigPath, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreElementsPositions(SEGMENTS_POSITIONS_WRITER_NAME, SEGMENTS_POSITIONS_TAG_NAME,
                                           notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreElementsPositions(MEASURES_POSITIONS_WRITER_NAME, MEASURES_POSITIONS_TAG_NAME,
                                           notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScorePdf(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMidi(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMusicXML(notation, jsonWriter, ADD_SEPARATOR);
    result &= exportScoreMetaData(notation, jsonWriter, ADD_SEPARATOR);
    result &= devInfo(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreMeta(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<INotationProjectPtr> prj = openProject(in, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    INotationPtr notation = prj.val->masterNotation()->notation();

    QFile outputFile;
    openOutputFile(outputFile, out);

    BackendJsonWriter jsonWriter(&outputFile);

    bool result = exportScoreMetaData(notation, jsonWriter);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreParts(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<INotationProjectPtr> prj = openProject(in, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    QFile outputFile;
    openOutputFile(outputFile, out);

    Ret ret = doExportScoreParts(prj.val->masterNotation(), outputFile);

    outputFile.close();

    return ret;
}

Ret BackendApi::exportScorePartsPdfs(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC

    RetVal<INotationProjectPtr> prj = openProject(in, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    QFile outputFile;
    openOutputFile(outputFile, out);

    std::string scoreFileName = io::dirpath(in).toStdString() + "/" + io::filename(in, false).toStdString() + ".pdf";

    Ret ret = doExportScorePartsPdfs(prj.val->masterNotation(), outputFile, scoreFileName);

    outputFile.close();

    return ret;
}

Ret BackendApi::exportScoreTranspose(const io::path_t& in, const io::path_t& out, const std::string& optionsJson,
                                     const io::path_t& stylePath,
                                     bool forceMode)
{
    TRACEFUNC

    RetVal<INotationProjectPtr> prj = openProject(in, stylePath, forceMode);
    if (!prj.ret) {
        return prj.ret;
    }

    INotationPtr notation = prj.val->masterNotation()->notation();

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

Ret BackendApi::openOutputFile(QFile& file, const io::path_t& out)
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

RetVal<project::INotationProjectPtr> BackendApi::openProject(const io::path_t& path,
                                                             const io::path_t& stylePath,
                                                             bool forceMode)
{
    TRACEFUNC

    auto notationProject = notationCreator()->newProject();
    IF_ASSERT_FAILED(notationProject) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = notationProject->load(path, stylePath, forceMode);
    if (!ret) {
        LOGE() << "failed load: " << path << ", ret: " << ret.toString();
        return make_ret(Ret::Code::InternalError);
    }

    IMasterNotationPtr masterNotation = notationProject->masterNotation();
    if (!masterNotation) {
        return make_ret(Ret::Code::InternalError);
    }

    INotationPtr notation = masterNotation->notation();
    if (!notation) {
        return make_ret(Ret::Code::InternalError);
    }

    switchToPageView(masterNotation);
    renderExcerptsContents(masterNotation);

    return RetVal<INotationProjectPtr>::make_ok(notationProject);
}

PageList BackendApi::pages(const INotationPtr notation)
{
    auto elements = notation->elements();
    if (!elements) {
        return {};
    }

    return elements->pages();
}

QVariantMap BackendApi::readBeatsColors(const io::path_t& filePath)
{
    TRACEFUNC

    if (filePath.empty()) {
        return QVariantMap();
    }

    RetVal<ByteArray> fileData = fileSystem()->readFile(filePath);
    if (!fileData.ret) {
        LOGW() << fileData.ret.toString();
        return QVariantMap();
    }

    QJsonDocument document = QJsonDocument::fromJson(fileData.val.toQByteArrayNoCopy());
    QJsonObject obj = document.object();
    QJsonArray colors = obj.value("highlight").toArray();

    QVariantMap result;

    for (const QJsonValue colorObj: colors) {
        QJsonObject cobj = colorObj.toObject();
        QJsonArray beatsIndexes = cobj.value("beats").toArray();
        QColor beatsColor = QColor(cobj.value("color").toString());

        for (const QJsonValue index: beatsIndexes) {
            result[index.toString()] = beatsColor;
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
        jsonWriter.addValue(pngData.toBase64(), !lastArrayValue);
    }

    jsonWriter.closeArray(addSeparator);

    return result ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret BackendApi::exportScoreSvgs(const INotationPtr notation, const io::path_t& highlightConfigPath, BackendJsonWriter& jsonWriter,
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
    QVariantMap beatsColors = readBeatsColors(highlightConfigPath);

    bool result = true;
    for (size_t i = 0; i < notationPages.size(); ++i) {
        QByteArray svgData;
        QBuffer svgDevice(&svgData);
        svgDevice.open(QIODevice::ReadWrite);

        INotationWriter::Options options {
            { INotationWriter::OptionKey::PAGE_NUMBER, Val(static_cast<int>(i)) },
            { INotationWriter::OptionKey::TRANSPARENT_BACKGROUND, Val(false) },
            { INotationWriter::OptionKey::BEATS_COLORS, Val::fromQVariant(beatsColors) }
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

Ret BackendApi::exportScoreElementsPositions(const std::string& elementsPositionsWriterName, const std::string& elementsPositionsTagName,
                                             const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    TRACEFUNC

    RetVal<QByteArray> writerRetVal = processWriter(elementsPositionsWriterName, notation);
    if (!writerRetVal.ret) {
        return writerRetVal.ret;
    }

    jsonWriter.addKey(elementsPositionsTagName.c_str());
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

Ret BackendApi::exportScorePdf(const INotationPtr notation, QIODevice& destinationDevice)
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

    jsonWriter.addKey(MUSICXML_JSON_NAME.c_str());
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

Ret BackendApi::devInfo(const notation::INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    UNUSED(notation);

    TRACEFUNC

    QJsonObject infoObj;
    infoObj["version"] = QString::fromStdString(String("%1(%2)").arg(framework::MUVersion::fullVersion(),
                                                                     framework::MUVersion::revision()).toStdString());

    jsonWriter.addKey(DEV_INFO_NAME.c_str());
    jsonWriter.addValue(QJsonDocument(infoObj).toJson(), addSeparator, true);

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

Ret BackendApi::doExportScoreParts(const IMasterNotationPtr masterNotation, QIODevice& destinationDevice)
{
    QJsonArray partsObjList;
    QJsonArray partsMetaList;
    QJsonArray partsTitles;

    ExcerptNotationList excerpts = allExcerpts(masterNotation);

    for (IExcerptNotationPtr excerpt : excerpts) {
        mu::engraving::Score* part = excerpt->notation()->elements()->msScore();
        std::map<String, String> partMetaTags = part->metaTags();

        QJsonValue partTitle(part->name());
        partsTitles << partTitle;

        QVariantMap meta;
        for (const String& key: mu::keys(partMetaTags)) {
            meta[key] = partMetaTags[key].toQString();
        }

        QJsonValue partMetaObj = QJsonObject::fromVariantMap(meta);
        partsMetaList << partMetaObj;

        std::string fileName = io::escapeFileName(part->name().toStdString()).toStdString() + ".mscz";
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

Ret BackendApi::doExportScorePartsPdfs(const IMasterNotationPtr masterNotation, QIODevice& destinationDevice,
                                       const std::string& scoreFileName)
{
    QJsonObject jsonForPdfs;
    jsonForPdfs["score"] = QString::fromStdString(scoreFileName);
    QByteArray scoreBin = processWriter(PDF_WRITER_NAME, masterNotation->notation()).val;
    jsonForPdfs["scoreBin"] = QString::fromLatin1(scoreBin);

    INotationPtrList notations;
    notations.push_back(masterNotation->notation());

    QJsonArray partsArray;
    QJsonArray partsNamesArray;

    ExcerptNotationList excerpts = allExcerpts(masterNotation);

    for (IExcerptNotationPtr e : excerpts) {
        QJsonValue partNameVal(e->name());
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
        { INotationWriter::OptionKey::UNIT_TYPE, Val(INotationWriter::UnitType::MULTI_PART) }
    };

    QByteArray fullScoreData = processWriter(PDF_WRITER_NAME, notations, options).val;
    jsonForPdfs["scoreFullBin"] = QString::fromLatin1(fullScoreData.toBase64());

    QJsonDocument jsonDoc(jsonForPdfs);
    bool ok = destinationDevice.write(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact)) != -1;

    return ok;
}

Ret BackendApi::doExportScoreTranspose(const INotationPtr notation, BackendJsonWriter& jsonWriter, bool addSeparator)
{
    mu::engraving::Score* score = notation->elements()->msScore();

    jsonWriter.addKey("mscz");

    std::string fileNumber = std::to_string(QRandomGenerator::global()->generate() % 1000000);
    std::string fileName = score->name().toStdString() + "_transposed." + fileNumber + ".mscx";

    RetVal<QByteArray> scoreJson = scorePartJson(score, fileName);
    if (!scoreJson.ret) {
        LOGW() << "Transpose: adding mscz failed";
    }

    jsonWriter.addValue(scoreJson.val, ADD_SEPARATOR);

    Ret ret = exportScorePdf(notation, jsonWriter, addSeparator);
    return ret;
}

RetVal<QByteArray> BackendApi::scorePartJson(mu::engraving::Score* score, const std::string& fileName)
{
    ByteArray scoreData;
    Buffer buf(&scoreData);

    MscWriter::Params params;
    params.device = &buf;
    params.filePath = QString::fromStdString(fileName);
    params.mode = MscIoMode::Zip;

    MscWriter mscWriter(params);
    mscWriter.open();

    bool ok = MscSaver().exportPart(score, mscWriter);
    if (!ok) {
        LOGW() << "Error save mscz file";
    }

    mscWriter.close();
    if (mscWriter.hasError()) {
        ok = false;
        LOGW() << "Error write mscz file";
    }

    QByteArray ba = QByteArray::fromRawData(reinterpret_cast<const char*>(scoreData.constData()), static_cast<int>(scoreData.size()));

    RetVal<QByteArray> result;
    result.ret = ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
    result.val = ba.toBase64();

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

void BackendApi::switchToPageView(IMasterNotationPtr masterNotation)
{
    //! NOTE: All operations must be done in page view mode
    masterNotation->notation()->setViewMode(ViewMode::PAGE);
    for (IExcerptNotationPtr excerpt : masterNotation->excerpts().val) {
        excerpt->notation()->setViewMode(ViewMode::PAGE);
    }
}

void BackendApi::renderExcerptsContents(IMasterNotationPtr masterNotation)
{
    //! NOTE: Due to optimization, only the master score is layouted
    //!       Let's layout all the scores of the excerpts
    for (IExcerptNotationPtr excerpt : masterNotation->excerpts().val) {
        Score* score = excerpt->notation()->elements()->msScore();
        if (!score->autoLayoutEnabled()) {
            score->doLayout();
        }
    }
}

ExcerptNotationList BackendApi::allExcerpts(notation::IMasterNotationPtr masterNotation)
{
    initPotentialExcerpts(masterNotation);

    ExcerptNotationList excerpts = masterNotation->excerpts().val;
    ExcerptNotationList potentialExcerpts = masterNotation->potentialExcerpts();
    excerpts.insert(excerpts.end(), potentialExcerpts.begin(), potentialExcerpts.end());

    masterNotation->sortExcerpts(excerpts);

    return excerpts;
}

void BackendApi::initPotentialExcerpts(notation::IMasterNotationPtr masterNotation)
{
    ExcerptNotationList potentialExcerpts = masterNotation->potentialExcerpts();

    masterNotation->initExcerpts(potentialExcerpts);
    renderExcerptsContents(masterNotation);
}

Ret BackendApi::updateSource(const io::path_t& in, const std::string& newSource, bool forceMode)
{
    RetVal<INotationProjectPtr> project = openProject(in, NO_STYLE, forceMode);
    if (!project.ret) {
        return project.ret;
    }

    ProjectMeta meta = project.val->metaInfo();
    meta.source = QString::fromStdString(newSource);

    project.val->setMetaInfo(meta);

    return project.val->save();
}
