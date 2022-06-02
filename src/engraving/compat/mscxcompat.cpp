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
#include "mscxcompat.h"

#include "io/file.h"
#include "io/buffer.h"

#include "../rw/scorereader.h"

#include "infrastructure/io/localfileinfoprovider.h"

#include "log.h"

using namespace mu::io;

mu::engraving::Score::FileError mu::engraving::compat::mscxToMscz(const QString& mscxFilePath, ByteArray* msczData)
{
    File mscxFile(mscxFilePath);
    if (!mscxFile.open(IODevice::ReadOnly)) {
        LOGE() << "failed open file: " << mscxFilePath;
        return mu::engraving::Score::FileError::FILE_OPEN_ERROR;
    }

    ByteArray mscxData = mscxFile.readAll();

    Buffer buf(msczData);
    MscWriter::Params params;
    params.device = &buf;
    params.filePath = mscxFilePath;
    params.mode = MscIoMode::Zip;
    MscWriter writer(params);
    writer.open();
    writer.writeScoreFile(mscxData);

    return mu::engraving::Score::FileError::FILE_NO_ERROR;
}

mu::engraving::Score::FileError mu::engraving::compat::loadMsczOrMscx(mu::engraving::MasterScore* score, const QString& path,
                                                                      bool ignoreVersionError)
{
    ByteArray msczData;
    if (path.endsWith(".mscx", Qt::CaseInsensitive)) {
        //! NOTE Convert mscx -> mscz

        mu::engraving::Score::FileError err = mscxToMscz(path, &msczData);
        if (err != mu::engraving::Score::FileError::FILE_NO_ERROR) {
            return err;
        }
    } else if (path.endsWith(".mscz", Qt::CaseInsensitive)) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            LOGE() << "failed open file: " << path;
            return mu::engraving::Score::FileError::FILE_OPEN_ERROR;
        }

        msczData = msczFile.readAll();
    } else {
        LOGE() << "unknown type, path: " << path;
        return mu::engraving::Score::FileError::FILE_UNKNOWN_TYPE;
    }

    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    Buffer msczBuf(&msczData);
    MscReader::Params params;
    params.device = &msczBuf;
    params.filePath = path;
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    reader.open();

    ScoreReader scoreReader;
    engraving::Err err = scoreReader.loadMscz(score, reader, ignoreVersionError);
    return err == Err::NoError ? mu::engraving::Score::FileError::FILE_NO_ERROR : mu::engraving::Score::FileError::FILE_ERROR;
}

mu::engraving::Err mu::engraving::compat::loadMsczOrMscx(EngravingProjectPtr project, const QString& path, bool ignoreVersionError)
{
    ByteArray msczData;
    QString filePath = path;
    if (path.endsWith(".mscx", Qt::CaseInsensitive)) {
        //! NOTE Convert mscx -> mscz

        mu::engraving::Score::FileError err = mscxToMscz(path, &msczData);
        if (err != mu::engraving::Score::FileError::FILE_NO_ERROR) {
            return scoreFileErrorToErr(err);
        }
    } else if (path.endsWith(".mscz", Qt::CaseInsensitive)) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            LOGE() << "failed open file: " << path;
            return scoreFileErrorToErr(mu::engraving::Score::FileError::FILE_OPEN_ERROR);
        }

        msczData = msczFile.readAll();
    } else {
        LOGE() << "unknown type, path: " << path;
        return scoreFileErrorToErr(mu::engraving::Score::FileError::FILE_UNKNOWN_TYPE);
    }

    project->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    Buffer msczBuf(&msczData);
    MscReader::Params params;
    params.device = &msczBuf;
    params.filePath = filePath;
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    reader.open();

    Err err = project->loadMscz(reader, ignoreVersionError);
    return err;
}
