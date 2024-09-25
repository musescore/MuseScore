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
#ifndef MUSE_IO_ALLZEROSFILECORRUPTOR_H
#define MUSE_IO_ALLZEROSFILECORRUPTOR_H

#include "filecorruptorbase.h"

namespace muse::io {
class AllZerosFileCorruptor : public FileCorruptorBase
{
public:
    AllZerosFileCorruptor(const path_t& filePath);

protected:
    void fillCorruptData(uint8_t* data, size_t len) override;
};
}

#endif // MUSE_IO_ALLZEROSFILECORRUPTOR_H

