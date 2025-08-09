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

Ret mu::engraving::compat::loadMsczOrMscx(MasterScore* score, const io::path_t& path, bool ignoreVersionError)
{
    std::string suffix = io::suffix(path);

    MscReader::Params params;
    params.filePath = path;
    params.mode = mscIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Err::FileUnknownType, path);
    }

    MscReader reader(params);
    Ret ret = reader.open();
    if (!ret) {
        return ret;
    }

    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    MscLoader scoreReader;
    SettingsCompat audioSettings;
    return scoreReader.loadMscz(score, reader, audioSettings, ignoreVersionError);
}

Ret mu::engraving::compat::loadMsczOrMscx(EngravingProjectPtr project, const io::path_t& path, bool ignoreVersionError)
{
    std::string suffix = io::suffix(path);

    MscReader::Params params;
    params.filePath = path;
    params.mode = mscIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Err::FileUnknownType, path);
    }

    MscReader reader(params);
    Ret ret = reader.open();
    if (!ret) {
        return ret;
    }

    project->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    SettingsCompat settingsCompat;
    return project->loadMscz(reader, settingsCompat, ignoreVersionError);
}
