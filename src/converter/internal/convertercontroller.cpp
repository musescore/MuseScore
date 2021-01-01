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
#include "convertercontroller.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "log.h"
#include "convertercodes.h"
#include "stringutils.h"

using namespace mu::converter;

mu::Ret ConverterController::batchConvert(const io::path& batchJobFile)
{
    RetVal<BatchJob> batchJob = parseBatchJob(batchJobFile);
    if (!batchJob.ret) {
        LOGE() << "failed parse batch job file, err: " << batchJob.ret.toString();
        return batchJob.ret;
    }

    Ret ret = make_ret(Ret::Code::Ok);
    for (const Job& job : batchJob.val) {
        ret = convert(job.in, job.out);
        if (!ret) {
            LOGE() << "failed convert, err: " << ret.toString() << ", in: " << job.in << ", out: " << job.out;
            break;
        }
    }

    return ret;
}

mu::Ret ConverterController::convert(const io::path& in, const io::path& out)
{
    TRACEFUNC;
    LOGI() << "in: " << in << ", out: " << out;
    auto notation = notationCreator()->newMasterNotation();
    IF_ASSERT_FAILED(notation) {
        return make_ret(Err::UnknownError);
    }

    std::string suffix = io::syffix(out);
    auto writer = writers()->writer(suffix);
    if (!writer) {
        return make_ret(Err::ConvertTypeUnknown);
    }

    Ret ret = notation->load(in);
    if (!ret) {
        LOGE() << "failed load notation, err: " << ret.toString() << ", path: " << in;
        return make_ret(Err::InFileFailedLoad);
    }

    QFile file(out.toQString());
    if (!file.open(QFile::WriteOnly)) {
        return make_ret(Err::OutFileFailedOpen);
    }

    ret = writer->write(notation, file);
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
