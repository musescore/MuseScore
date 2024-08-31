/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "converterapi.h"

#include <QCoreApplication>

#include "log.h"

using namespace muse;
using namespace mu::converter::api;

ConverterApi::ConverterApi(muse::api::IApiEngine* e)
    : muse::api::ApiObject(e) {}

QString ConverterApi::selectDir(const QString& title, const QString& dir) const
{
    io::path_t path = interactive()->selectDirectory(title, dir);
    return path.toQString();
}

QStringList ConverterApi::scanDir(const QString& dir, const QStringList& filters) const
{
    std::vector<std::string> fv;
    fv.reserve(filters.size());
    for (const QString& f : filters) {
        fv.push_back(f.toStdString());
    }
    RetVal<io::paths_t> rv = fileSystem()->scanFiles(dir, fv);
    if (!rv.ret) {
        LOGE() << rv.ret.toString();
        return QStringList();
    }

    QStringList files;
    files.reserve(rv.val.size());
    for (const io::path_t& p : rv.val) {
        files << p.toQString();
    }
    return files;
}

QString ConverterApi::basename(const QString& filePath) const
{
    return io::basename(filePath).toQString();
}

bool ConverterApi::batch(const QString& outDir, const QString& job, const QString& uriQuery, QJSValue progressCallback)
{
    io::path_t jobFile = outDir + "/job.json";
    QByteArray jobData = job.toUtf8();
    Ret ret = fileSystem()->writeFile(jobFile, ByteArray::fromQByteArray(jobData));
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    muse::ProgressPtr progress = std::make_shared<muse::Progress>();
    progress->progressChanged.onReceive(this, [progressCallback](int64_t current, int64_t total, const std::string& title) {
        QCoreApplication::processEvents();
        if (progressCallback.isCallable()) {
            QJSValueList args;
            args << int(current) << int(total) << QString::fromStdString(title);
            progressCallback.call(args);
        }
        QCoreApplication::processEvents();
    });

    QCoreApplication::processEvents();

    ret = converter()->batchConvert(jobFile, io::path_t(), false, String(), UriQuery(uriQuery.toStdString()), progress);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    return true;
}
