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
#ifndef MU_LANGUAGES_LANGUAGESTYPES_H
#define MU_LANGUAGES_LANGUAGESTYPES_H

#include <QString>
#include <QVersionNumber>
#include <QJsonObject>
#include <QObject>
#include <QMetaObject>
#include <QJsonArray>

namespace mu::languages {
const QString SYSTEM_LANGUAGE_CODE = "system";
const QString PLACEHOLDER_LANGUAGE_CODE = "en@placeholder";

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

struct LanguageFile
{
    QString name;
    QString hash;

    LanguageFile() = default;
    LanguageFile(const QString& name, const QString& hash)
        : name(name), hash(hash) {}

    QJsonObject toJson() const
    {
        return { { "name", name },
            { "hash", hash } };
    }
};
using LanguageFiles = QList<LanguageFile>;

struct Language
{
    QString code;
    QString name;
    QString archiveFileName;

    LanguageFiles files;

    Language() = default;

    QJsonObject toJson() const
    {
        QJsonObject obj = {
            { "name", name },
            { "fileName", archiveFileName }
        };

        QJsonArray filesArray;
        for (const LanguageFile& file: files) {
            filesArray << file.toJson();
        }
        obj["files"] = filesArray;

        return obj;
    }
};

using LanguagesHash = QHash<QString /*code*/, Language>;
}

#endif // MU_LANGUAGES_LANGUAGESTYPES_H
