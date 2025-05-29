/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "webapi.h"

#include "global/io/file.h"

#include "log.h"

using namespace muse;
using namespace mu::webbridge;

void WebApi::onclickTest1(int num)
{
    LOGI() << "num: " << num;
    interactive()->info("onclickTest1", "Hey!");
}

void WebApi::load(const void* source, unsigned int len)
{
    LOGI() << source << ", len: " << len;
    ByteArray data = ByteArray::fromRawData(reinterpret_cast<const char*>(source), len);
    io::path_t tempFilePath = "/mu/temp/current.mscz";
    io::File::writeFile(tempFilePath, data);

    dispatcher()->dispatch("file-open", actions::ActionData::make_arg1(QUrl::fromLocalFile(tempFilePath.toQString())));

    io::File::remove(tempFilePath);
}
