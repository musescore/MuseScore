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
#ifndef MU_PROJECT_EXPORTTYPE_H
#define MU_PROJECT_EXPORTTYPE_H

#include <unordered_set>
#include <QList>
#include <QVariant>

#include "containers.h"

namespace mu::project {
struct ExportType;
class ExportTypeList : public QList<ExportType>
{
public:
    ExportTypeList();
    ExportTypeList(std::initializer_list<ExportType> args);

    bool contains(const QString& id) const;
    const ExportType& getById(const QString& id) const;

    QVariantList toVariantList() const;
};

struct ExportType
{
    QString id;
    QStringList suffixes;
    ExportTypeList subtypes;
    QString name;
    QString filterName;
    QString settingsPagePath;

    QVariantMap toMap() const;
    std::vector<std::string> filter() const;
    bool hasSubtypes() const;

    static ExportType makeWithSuffixes(const QStringList& suffixes, const QString& name, const QString& filterName,
                                       const QString& settingsPagePath = QString());
    static ExportType makeWithSubtypes(const ExportTypeList& subtypes, const QString& name);

    inline bool operator==(const ExportType& other) const { return id == other.id; }
    inline bool operator!=(const ExportType& other) const { return id != other.id; }
};

inline bool isAudioExport(const std::string& suffix)
{
    static const std::unordered_set<std::string> audioSuffixes {
        "mp3", "wav", "ogg", "flac",
    };

    return muse::contains(audioSuffixes, suffix);
}
}

#endif // MU_PROJECT_EXPORTTYPE_H
