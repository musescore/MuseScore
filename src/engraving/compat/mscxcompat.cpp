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
#include "mscxcompat.h"

#include "io/file.h"
#include "io/buffer.h"

#include "engraving/engravingerrors.h"
#include "../rw/mscloader.h"

#include "infrastructure/localfileinfoprovider.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;

Ret mu::engraving::compat::mscxToMscz(const io::path_t& mscxFilePath, ByteArray* msczData)
{
    File mscxFile(mscxFilePath);
    if (!mscxFile.open(IODevice::ReadOnly)) {
        return make_ret(Err::FileOpenError, mscxFilePath);
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

    return muse::make_ok();
}

Ret mu::engraving::compat::loadMsczOrMscx(MasterScore* score, const io::path_t& path, bool ignoreVersionError)
{
    std::string suffix = io::suffix(path);

    ByteArray msczData;
    if (suffix == MSCX) {
        //! NOTE Convert mscx -> mscz
        Ret ret = mscxToMscz(path, &msczData);
        if (!ret) {
            return ret;
        }
    } else if (suffix == MSCZ) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            return make_ret(Err::FileOpenError, path);
        }

        msczData = msczFile.readAll();
    } else {
        return make_ret(Err::FileUnknownType, path);
    }

    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    Buffer msczBuf(&msczData);
    MscReader::Params params;
    params.device = &msczBuf;
    params.filePath = path;
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    reader.open();

    MscLoader scoreReader;
    SettingsCompat audioSettings;
    return scoreReader.loadMscz(score, reader, audioSettings, ignoreVersionError);
}

Ret mu::engraving::compat::loadMsczOrMscx(EngravingProjectPtr project, const io::path_t& path, bool ignoreVersionError)
{
    std::string suffix = io::suffix(path);

    ByteArray msczData;
    if (suffix == MSCX) {
        //! NOTE Convert mscx -> mscz
        Ret ret = mscxToMscz(path, &msczData);
        if (!ret) {
            return ret;
        }
    } else if (suffix == MSCZ) {
        File msczFile(path);
        if (!msczFile.open(IODevice::ReadOnly)) {
            return make_ret(Err::FileOpenError, path);
        }

        msczData = msczFile.readAll();
    } else {
        return make_ret(Err::FileUnknownType, path);
    }

    project->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    Buffer msczBuf(&msczData);
    MscReader::Params params;
    params.device = &msczBuf;
    params.filePath = path;
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    reader.open();

    SettingsCompat settingsCompat;
    return project->loadMscz(reader, settingsCompat, ignoreVersionError);
}
