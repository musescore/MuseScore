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
#ifndef MU_FRAMEWORK_IFSOPERATIONS_H
#define MU_FRAMEWORK_IFSOPERATIONS_H

#include <QString>
#include <QDir>

#include "modularity/imoduleexport.h"
#include "retval.h"

namespace mu {
namespace framework {
class IFsOperations : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IFsOperations)

public:
    virtual ~IFsOperations() = default;

    virtual Ret exists(const QString& path) const = 0;
    virtual Ret remove(const QString& path) const = 0;

    virtual RetVal<QString> fileName(const QString& filePath) const = 0;
    virtual RetVal<QString> baseName(const QString& filePath) const = 0;

    virtual RetVal<QByteArray> readFile(const QString& filePath) const = 0;

    virtual Ret makePath(const QString& path) const = 0;

    virtual RetVal<QStringList> directoryFileList(const QString& path, const QStringList& nameFilters,
                                                  QDir::Filters filters = QDir::NoFilter) const = 0;
};
}
}

#endif // MU_FRAMEWORK_IFSOPERATIONS_H
