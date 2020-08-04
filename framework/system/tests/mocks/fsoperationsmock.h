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
#ifndef MU_FRAMEWORK_FSOPERATIONSMOCK_H
#define MU_FRAMEWORK_FSOPERATIONSMOCK_H

#include <gmock/gmock.h>

#include "framework/system/ifsoperations.h"

namespace mu {
namespace framework {
class FsOperationsMock : public IFsOperations
{
public:
    MOCK_METHOD(Ret, exists, (const QString&), (const, override));
    MOCK_METHOD(Ret, remove, (const QString&), (const, override));

    MOCK_METHOD(RetVal<QString>, fileName, (const QString&), (const, override));
    MOCK_METHOD(RetVal<QString>, baseName, (const QString&), (const, override));

    MOCK_METHOD(RetVal<QByteArray>, readFile, (const QString&), (const, override));

    MOCK_METHOD(Ret, makePath, (const QString&), (const, override));

    MOCK_METHOD(RetVal<QStringList>, directoryFileList, (const QString&, const QStringList&, QDir::Filters), (const, override));

    MOCK_METHOD(QStringList, scanForFiles, (const QString&, const QStringList&, ScanMode), (const, override));
};
}
}

#endif // MU_FRAMEWORK_FSOPERATIONSMOCK_H

