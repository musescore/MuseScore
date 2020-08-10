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
#ifndef MU_FRAMEWORK_FSOPERATIONS_H
#define MU_FRAMEWORK_FSOPERATIONS_H

#include "ifsoperations.h"

namespace mu {
namespace framework {
class FsOperations : public IFsOperations
{
public:
    Ret exists(const QString& path) const override;
    Ret remove(const QString& path) const override;

    QString fileName(const QString& filePath) const override;
    QString baseName(const QString& filePath) const override;

    RetVal<QByteArray> readFile(const QString& filePath) const override;
    Ret makePath(const QString& path) const override;

    RetVal<QStringList> scanFiles(const QString& rootDir, const QStringList& filters, ScanMode mode) const override;

private:
    Ret removeFile(const QString& path) const;
    Ret removeDir(const QString& path) const;
};
}
}

#endif // MU_FRAMEWORK_FSOPERATIONS_H
