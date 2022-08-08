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

#include "infrastructure/localfileinfoprovider.h"

#include "log.h"

using namespace mu::io;
using namespace mu::engraving;

Err mu::engraving::compat::mscxToMscz(const String& mscxFilePath, ByteArray* msczData)
{
    File mscxFile(mscxFilePath);
    if (!mscxFile.open(IODevice::ReadOnly)) {
        LOGE() << "failed open file: " << mscxFilePath;
        return Err::FileOpenError;
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

    return Err::NoError;
}

Err mu::engraving::compat::loadMsczOrMscx(MasterScore* score, const String& path, bool ignoreVersionError)
{
    ByteArray msczData;
    if (path.endsWith(u".mscx", mu::CaseInsensitive)) {
        //! NOTE Convert mscx -> mscz

        Err err = mscxToMscz(path, &msczData);
        if (err != Err::NoError) {
            return err;
        }
    } else if (path.endsWith(u".mscz", mu::CaseInsensitive)) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            LOGE() << "failed open file: " << path;
            return Err::FileOpenError;
        }

        msczData = msczFile.readAll();
    } else {
        LOGE() << "unknown type, path: " << path;
        return Err::FileUnknownType;
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
    Err err = scoreReader.loadMscz(score, reader, ignoreVersionError);
    return err;
}

Err mu::engraving::compat::loadMsczOrMscx(EngravingProjectPtr project, const String& path, bool ignoreVersionError)
{
    ByteArray msczData;
    String filePath = path;
    if (path.endsWith(u".mscx", mu::CaseInsensitive)) {
        //! NOTE Convert mscx -> mscz

        Err err = mscxToMscz(path, &msczData);
        if (err != Err::NoError) {
            return err;
        }
    } else if (path.endsWith(u".mscz", mu::CaseInsensitive)) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            LOGE() << "failed open file: " << path;
            return Err::FileOpenError;
        }

        msczData = msczFile.readAll();
    } else {
        LOGE() << "unknown type, path: " << path;
        return Err::FileUnknownType;
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
