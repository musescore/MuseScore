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
#ifndef MU_LANGUAGES_LANGUAGESTYPES_H
#define MU_LANGUAGES_LANGUAGESTYPES_H

#include <QString>
#include <QVersionNumber>
#include <QJsonObject>
#include <QObject>
#include <QMetaObject>

namespace mu {
namespace languages {
class LanguageStatus
{
    Q_GADGET
public:
    enum class Status {
        Undefined = 0,
        Installed,
        NoInstalled,
        NeedUpdate
    };
    Q_ENUM(Status)
};

struct LanguageProgress
{
    QString status;
    bool indeterminate = true;

    qint64 current = 0;
    quint64 total = 0;

    LanguageProgress() = default;
    LanguageProgress(const QString& status, bool indeterminate)
        : status(status), indeterminate(indeterminate) {}
    LanguageProgress(const QString& status, qint64 current, qint64 total)
        : status(status), indeterminate(false), current(current), total(total) {}
};

struct Language
{
    QString code;
    QString name;
    QString fileName;
    double fileSize = 0.0;
    bool isCurrent = false;
    LanguageStatus::Status status = LanguageStatus::Status::Undefined;

    Language() = default;

    QJsonObject toJson() const
    {
        return { { "name", name },
            { "fileName", fileName },
            { "fileSize", fileSize },
            { "status", QString::number(static_cast<int>(status)) } };
    }
};

using LanguagesHash = QHash<QString /*code*/, Language>;
}
}

#endif // MU_LANGUAGES_LANGUAGESTYPES_H
