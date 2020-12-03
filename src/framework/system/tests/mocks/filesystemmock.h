//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_FRAMEWORK_FILESYSTEMMOCK_H
#define MU_FRAMEWORK_FILESYSTEMMOCK_H

#include <gmock/gmock.h>

#include "framework/system/ifilesystem.h"

namespace mu {
namespace framework {
class FileSystemMock : public IFileSystem
{
public:
    MOCK_METHOD(Ret, exists, (const io::path&), (const, override));
    MOCK_METHOD(Ret, remove, (const io::path&), (const, override));

    MOCK_METHOD(RetVal<QByteArray>, readFile, (const io::path&), (const, override));

    MOCK_METHOD(Ret, makePath, (const io::path&), (const, override));

    MOCK_METHOD(RetVal<io::paths>, scanFiles, (const io::path&, const QStringList&, ScanMode), (const, override));
};
}
}

#endif // MU_FRAMEWORK_FILESYSTEMMOCK_H
