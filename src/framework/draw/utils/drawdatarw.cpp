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
#include "drawdatarw.h"

#include "global/io/file.h"
#include "drawdatajson.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

RetVal<DrawDataPtr> DrawDataRW::readData(const io::path_t& filePath)
{
    ByteArray json;
    Ret ret = io::File::readFile(filePath, json);
    if (!ret) {
        return RetVal<DrawDataPtr>(ret);
    }

    RetVal<DrawDataPtr> rv = DrawDataJson::fromJson(json);
    return rv;
}

Ret DrawDataRW::writeData(const io::path_t& filePath, const DrawDataPtr& data, bool prettify)
{
    ByteArray json = DrawDataJson::toJson(data, prettify);
    return io::File::writeFile(filePath, json);
}

RetVal<Diff> DrawDataRW::readDiff(const io::path_t& filePath)
{
    ByteArray json;
    Ret ret = io::File::readFile(filePath, json);
    if (!ret) {
        return RetVal<Diff>(ret);
    }

    RetVal<Diff> rv = DrawDataJson::diffFromJson(json);
    return rv;
}

Ret DrawDataRW::writeDiff(const io::path_t& filePath, const Diff& diff)
{
    ByteArray json = DrawDataJson::diffToJson(diff);
    Ret ret = io::File::writeFile(filePath, json);
    if (ret) {
        LOGI() << "success write diff, file: " << filePath;
    } else {
        LOGE() << "failed write diff, err: " << ret.toString() << ", file: " << filePath;
    }
    return ret;
}
