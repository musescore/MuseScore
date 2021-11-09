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
#ifndef MU_SYSTEM_FILESYSTEMMOCK_H
#define MU_SYSTEM_FILESYSTEMMOCK_H

#include <gmock/gmock.h>

#include "framework/system/ifilesystem.h"

namespace mu::system {
class FileSystemMock : public IFileSystem
{
public:
    MOCK_METHOD(Ret, exists, (const io::path&), (const, override));
    MOCK_METHOD(Ret, remove, (const io::path&), (const, override));
    MOCK_METHOD(Ret, copy, (const io::path& src, const io::path& dst, bool replace), (const, override));
    MOCK_METHOD(Ret, move, (const io::path& src, const io::path& dst, bool replace), (const, override));

    MOCK_METHOD(RetVal<uint64_t>, fileSize, (const io::path& path), (const, override));

    MOCK_METHOD(RetVal<QByteArray>, readFile, (const io::path&), (const, override));
    MOCK_METHOD(Ret, writeToFile, (const io::path&, const QByteArray&), (const, override));

    MOCK_METHOD(Ret, makePath, (const io::path&), (const, override));

    MOCK_METHOD(RetVal<io::paths>, scanFiles, (const io::path&, const QStringList&, ScanMode), (const, override));

    MOCK_METHOD(void, setAttribute, (const io::path& path, Attribute attribute), (const, override));
};
}

#endif // MU_SYSTEM_FILESYSTEMMOCK_H
