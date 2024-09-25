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
#include "filecorruptorbase.h"

using namespace muse::io;

FileCorruptorBase::FileCorruptorBase(const path_t& filePath)
    : File(filePath)
{
}

size_t FileCorruptorBase::writeData(const uint8_t* data, size_t len)
{
    // Ignore the actual data and write something else so as to corrupt the file
    Q_UNUSED(data);
    uint8_t* corruptData = new uint8_t[len];
    fillCorruptData(corruptData, len);
    return File::writeData(corruptData, len);
}

