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
#ifndef MU_EXTENSIONS_EXTENSIONSTYPES_H
#define MU_EXTENSIONS_EXTENSIONSTYPES_H

#include <QString>
#include <QVersionNumber>
#include <QJsonObject>
#include <QObject>

namespace mu::extensions {
class ExtensionStatus
{
    Q_GADGET
public:
    enum Status {
        Undefined = 0,
        Installed,
        NoInstalled,
        NeedUpdate
    };
    Q_ENUM(Status)
};

struct ExtensionProgress
{
    QString status;
    bool indeterminate = true;

    qint64 current = 0;
    quint64 total = 0;

    ExtensionProgress() = default;
    ExtensionProgress(const QString& status, bool indeterminate)
        : status(status), indeterminate(indeterminate) {}
    ExtensionProgress(const QString& status, qint64 current, qint64 total)
        : status(status), indeterminate(false), current(current), total(total) {}
};

struct Extension
{
    QString code;
    QString name;
    QString description;
    QString fileName;
    double fileSize = 0.0;
    QVersionNumber version;
    ExtensionStatus::Status status = ExtensionStatus::Undefined;

    Extension() = default;

    QJsonObject toJson() const
    {
        return { { "name", name },
            { "description", description },
            { "fileName", fileName },
            { "fileSize", fileSize },
            { "version", version.toString() },
            { "status", QString::number(static_cast<int>(status)) } };
    }
};

using ExtensionsHash = QHash<QString /*code*/, Extension>;
}

#endif // MU_EXTENSIONS_EXTENSIONSTYPES_H
