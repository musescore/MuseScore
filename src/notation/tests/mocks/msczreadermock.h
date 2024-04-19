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
#ifndef MU_NOTATION_NOTATIONMSCZREADERMOCK_H
#define MU_NOTATION_NOTATIONMSCZREADERMOCK_H

#include <gmock/gmock.h>

#include "project/imscmetareader.h"

namespace mu::notation {
class MsczReaderMock : public project::IMscMetaReader
{
public:
    MOCK_METHOD(muse::RetVal<project::ProjectMeta>, readMeta, (const muse::io::path_t& filePath), (const, override));
    MOCK_METHOD(muse::RetVal<project::CloudProjectInfo>, readCloudProjectInfo, (const muse::io::path_t& filePath), (const, override));
};
}

#endif // MU_NOTATION_NOTATIONMSCZREADERMOCK_H
