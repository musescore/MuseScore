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
#include "convertercontroller.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "io/dir.h"
#include "stringutils.h"

#include "convertercodes.h"
#include "compat/backendapi.h"

#include "log.h"

using namespace mu::converter;
using namespace mu::project;
using namespace mu::notation;

static const std::string PDF_SUFFIX = "pdf";
static const std::string PNG_SUFFIX = "png";

mu::Ret ConverterController::batchConvert(const io::path_t& batchJobFile, const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC;

    RetVal<BatchJob> batchJob = parseBatchJob(batchJobFile);
    if (!batchJob.ret) {
        LOGE() << "failed parse batch job file, err: " << batchJob.ret.toString();
        return batchJob.ret;
    }

    Ret ret = make_ret(Ret::Code::Ok);
    for (const Job& job : batchJob.val) {
        ret = fileConvert(job.in, job.out, stylePath, forceMode);
        if (!ret) {
            LOGE() << "failed convert, err: " << ret.toString() << ", in: " << job.in << ", out: " << job.out;
            break;
        }
    }

    return ret;
}

mu::Ret ConverterController::fileConvert(const io::path_t& in, const io::path_t& out, const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC;

    LOGI() << "in: " << in << ", out: " << out;
    auto notationProject = notationCreator()->newProject();
    IF_ASSERT_FAILED(notationProject) {
        return make_ret(Err::UnknownError);
    }

    std::string suffix = io::suffix(out);
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return make_ret(Err::ConvertTypeUnknown);
    }

    Ret ret = notationProject->load(in, stylePath, forceMode);
    if (!ret) {
        LOGE() << "failed load notation, err: " << ret.toString() << ", path: " << in;
        return make_ret(Err::InFileFailedLoad);
    }

    globalContext()->setCurrentProject(notationProject);

    if (isConvertPageByPage(suffix)) {
        ret = convertPageByPage(writer, notationProject->masterNotation()->notation(), out);
    } else {
        ret = convertFullNotation(writer, notationProject->masterNotation()->notation(), out);
    }

    globalContext()->setCurrentProject(nullptr);

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::convertScoreParts(const mu::io::path_t& in, const mu::io::path_t& out, const mu::io::path_t& stylePath,
                                               bool forceMode)
{
    TRACEFUNC;

    auto notationProject = notationCreator()->newProject();
    IF_ASSERT_FAILED(notationProject) {
        return make_ret(Err::UnknownError);
    }

    std::string suffix = io::suffix(out);
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return make_ret(Err::ConvertTypeUnknown);
    }

    Ret ret = notationProject->load(in, stylePath, forceMode);
    if (!ret) {
        LOGE() << "failed load notation, err: " << ret.toString() << ", path: " << in;
        return make_ret(Err::InFileFailedLoad);
    }

    if (suffix == PDF_SUFFIX) {
        ret = convertScorePartsToPdf(writer, notationProject->masterNotation(), out);
    } else if (suffix == PNG_SUFFIX) {
        ret = convertScorePartsToPngs(writer, notationProject->masterNotation(), out);
    } else {
        ret = make_ret(Ret::Code::NotSupported);
    }

    return make_ret(Ret::Code::Ok);
}

mu::RetVal<ConverterController::BatchJob> ConverterController::parseBatchJob(const io::path_t& batchJobFile) const
{
    TRACEFUNC;

    RetVal<BatchJob> rv;
    QFile file(batchJobFile.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        rv.ret = make_ret(Err::BatchJobFileFailedOpen);
        return rv;
    }

    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        rv.ret = make_ret(Err::BatchJobFileFailedParse, err.errorString().toStdString());
        return rv;
    }

    QJsonArray arr = doc.array();

    auto correctUserInputPath = [](const QString& path) -> QString {
        return io::Dir::fromNativeSeparators(path).toQString();
    };

    for (const QJsonValue v : arr) {
        QJsonObject obj = v.toObject();

        Job job;
        job.in = correctUserInputPath(obj["in"].toString());
        job.out = correctUserInputPath(obj["out"].toString());

        if (!job.in.empty() && !job.out.empty()) {
            rv.val.push_back(std::move(job));
        }
    }

    rv.ret = make_ret(Ret::Code::Ok);
    return rv;
}

bool ConverterController::isConvertPageByPage(const std::string& suffix) const
{
    QList<std::string> types {
        PNG_SUFFIX
    };

    return types.contains(suffix);
}

mu::Ret ConverterController::convertPageByPage(INotationWriterPtr writer, INotationPtr notation, const mu::io::path_t& out) const
{
    TRACEFUNC;

    for (size_t i = 0; i < notation->elements()->pages().size(); i++) {
        const QString filePath
            = io::path_t(io::dirpath(out) + "/" + io::completeBasename(out) + "-%1." + io::suffix(out)).toQString().arg(i + 1);

        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) {
            return make_ret(Err::OutFileFailedOpen);
        }

        INotationWriter::Options options {
            { INotationWriter::OptionKey::PAGE_NUMBER, Val(static_cast<int>(i)) },
        };

        file.setProperty("path", out.toQString());

        Ret ret = writer->write(notation, file, options);
        if (!ret) {
            LOGE() << "failed write, err: " << ret.toString() << ", path: " << out;
            return make_ret(Err::OutFileFailedWrite);
        }

        file.close();
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::convertFullNotation(INotationWriterPtr writer, INotationPtr notation, const mu::io::path_t& out) const
{
    QFile file(out.toQString());
    if (!file.open(QFile::WriteOnly)) {
        return make_ret(Err::OutFileFailedOpen);
    }

    file.setProperty("path", out.toQString());
    Ret ret = writer->write(notation, file);
    if (!ret) {
        LOGE() << "failed write, err: " << ret.toString() << ", path: " << out;
        return make_ret(Err::OutFileFailedWrite);
    }

    file.close();

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::convertScorePartsToPdf(INotationWriterPtr writer, IMasterNotationPtr masterNotation,
                                                    const io::path_t& out) const
{
    TRACEFUNC;

    INotationPtrList notations;
    notations.push_back(masterNotation->notation());

    for (IExcerptNotationPtr e : masterNotation->excerpts().val) {
        notations.push_back(e->notation());
    }

    QFile file(out.toQString());
    if (!file.open(QFile::WriteOnly)) {
        return make_ret(Err::OutFileFailedOpen);
    }

    INotationWriter::Options options {
        { INotationWriter::OptionKey::UNIT_TYPE, Val(INotationWriter::UnitType::MULTI_PART) },
    };

    Ret ret = writer->writeList(notations, file, options);
    if (!ret) {
        LOGE() << "failed write, err: " << ret.toString() << ", path: " << out;
        return make_ret(Err::OutFileFailedWrite);
    }

    file.close();

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::convertScorePartsToPngs(INotationWriterPtr writer, mu::notation::IMasterNotationPtr masterNotation,
                                                     const io::path_t& out) const
{
    TRACEFUNC;

    Ret ret = convertPageByPage(writer, masterNotation->notation(), out);
    if (!ret) {
        return ret;
    }

    INotationPtrList excerpts;
    for (IExcerptNotationPtr e : masterNotation->excerpts().val) {
        excerpts.push_back(e->notation());
    }

    io::path_t pngFilePath = io::dirpath(out) + "/" + io::path_t(io::completeBasename(out) + "-excerpt.png");

    for (size_t i = 0; i < excerpts.size(); i++) {
        Ret ret = convertPageByPage(writer, excerpts[i], pngFilePath);
        if (!ret) {
            return ret;
        }
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::exportScoreMedia(const mu::io::path_t& in, const mu::io::path_t& out,
                                              const mu::io::path_t& highlightConfigPath,
                                              const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC;

    return BackendApi::exportScoreMedia(in, out, highlightConfigPath, stylePath, forceMode);
}

mu::Ret ConverterController::exportScoreMeta(const mu::io::path_t& in, const mu::io::path_t& out, const io::path_t& stylePath,
                                             bool forceMode)
{
    TRACEFUNC;

    return BackendApi::exportScoreMeta(in, out, stylePath, forceMode);
}

mu::Ret ConverterController::exportScoreParts(const mu::io::path_t& in, const mu::io::path_t& out, const io::path_t& stylePath,
                                              bool forceMode)
{
    TRACEFUNC;

    return BackendApi::exportScoreParts(in, out, stylePath, forceMode);
}

mu::Ret ConverterController::exportScorePartsPdfs(const mu::io::path_t& in, const mu::io::path_t& out, const io::path_t& stylePath,
                                                  bool forceMode)
{
    TRACEFUNC;

    return BackendApi::exportScorePartsPdfs(in, out, stylePath, forceMode);
}

mu::Ret ConverterController::exportScoreTranspose(const mu::io::path_t& in, const mu::io::path_t& out, const std::string& optionsJson,
                                                  const io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC;

    return BackendApi::exportScoreTranspose(in, out, optionsJson, stylePath, forceMode);
}

mu::Ret ConverterController::exportScoreVideo(const io::path_t& in, const io::path_t& out)
{
    TRACEFUNC;

    auto notationProject = notationCreator()->newProject();
    IF_ASSERT_FAILED(notationProject) {
        return make_ret(Err::UnknownError);
    }

    std::string suffix = io::suffix(out);
    auto writer = projectRW()->writer(suffix);
    if (!writer) {
        return make_ret(Err::ConvertTypeUnknown);
    }

    Ret ret = notationProject->load(in);
    if (!ret) {
        LOGE() << "failed load notation, err: " << ret.toString() << ", path: " << in;
        return make_ret(Err::InFileFailedLoad);
    }

    ret = writer->write(notationProject, out);
    if (!ret) {
        LOGE() << "failed write, err: " << ret.toString() << ", path: " << out;
        return make_ret(Err::OutFileFailedWrite);
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ConverterController::updateSource(const io::path_t& in, const std::string& newSource, bool forceMode)
{
    TRACEFUNC;

    return BackendApi::updateSource(in, newSource, forceMode);
}
