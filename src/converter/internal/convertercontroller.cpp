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

#include "log.h"
#include "convertercodes.h"
#include "stringutils.h"
#include "compat/backendapi.h"

using namespace mu::converter;

mu::Ret ConverterController::batchConvert(const io::path& batchJobFile, const io::path& stylePath)
{
    RetVal<BatchJob> batchJob = parseBatchJob(batchJobFile);
    if (!batchJob.ret) {
        LOGE() << "failed parse batch job file, err: " << batchJob.ret.toString();
        return batchJob.ret;
    }

    Ret ret = make_ret(Ret::Code::Ok);
    for (const Job& job : batchJob.val) {
        ret = fileConvert(job.in, job.out, stylePath);
        if (!ret) {
            LOGE() << "failed convert, err: " << ret.toString() << ", in: " << job.in << ", out: " << job.out;
            break;
        }
    }

    return ret;
}

mu::Ret ConverterController::fileConvert(const io::path& in, const io::path& out, const io::path& stylePath)
{
    TRACEFUNC;
    LOGI() << "in: " << in << ", out: " << out;
    auto masterNotation = notationCreator()->newMasterNotation();
    IF_ASSERT_FAILED(masterNotation) {
        return make_ret(Err::UnknownError);
    }

    std::string suffix = io::syffix(out);
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return make_ret(Err::ConvertTypeUnknown);
    }

    Ret ret = masterNotation->load(in, stylePath);
    if (!ret) {
        LOGE() << "failed load notation, err: " << ret.toString() << ", path: " << in;
        return make_ret(Err::InFileFailedLoad);
    }

    QFile file(out.toQString());
    if (!file.open(QFile::WriteOnly)) {
        return make_ret(Err::OutFileFailedOpen);
    }

    ret = writer->write(masterNotation->notation(), file);
    if (!ret) {
        LOGE() << "failed write, err: " << ret.toString() << ", path: " << out;
        return make_ret(Err::OutFileFailedWrite);
    }

    file.close();

    return make_ret(Ret::Code::Ok);
}

mu::RetVal<ConverterController::BatchJob> ConverterController::parseBatchJob(const io::path& batchJobFile) const
{
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

    for (const QJsonValue v : arr) {
        QJsonObject obj = v.toObject();

        Job job;
        job.in = obj["in"].toString();
        job.out = obj["out"].toString();

        if (!job.in.empty() && !job.out.empty()) {
            rv.val.push_back(std::move(job));
        }
    }

    rv.ret = make_ret(Ret::Code::Ok);
    return rv;
}

mu::Ret ConverterController::exportScoreMedia(const mu::io::path& in, const mu::io::path& out, const io::path& stylePath,
                                              const mu::io::path& highlightConfigPath)
{
    TRACEFUNC;

    return BackendApi::exportScoreMedia(in, out, stylePath, highlightConfigPath);
}

mu::Ret ConverterController::exportScoreMeta(const mu::io::path& in, const mu::io::path& out, const io::path& stylePath)
{
    TRACEFUNC;

    return BackendApi::exportScoreMeta(in, out, stylePath);
}

mu::Ret ConverterController::exportScoreParts(const mu::io::path& in, const mu::io::path& out, const io::path& stylePath)
{
    TRACEFUNC;

    return BackendApi::exportScoreParts(in, out, stylePath);
}

mu::Ret ConverterController::exportScorePartsPdfs(const mu::io::path& in, const mu::io::path& out, const io::path& stylePath)
{
    TRACEFUNC;

    return BackendApi::exportScorePartsPdfs(in, out, stylePath);
}

mu::Ret ConverterController::updateSource(const io::path& in, const QString& newSource)
{
    TRACEFUNC;

    return BackendApi::updateSource(in, newSource);
}
