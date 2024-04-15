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
#include "exporttype.h"

using namespace mu::project;

ExportTypeList::ExportTypeList()
    : QList()
{
}

ExportTypeList::ExportTypeList(std::initializer_list<ExportType> args)
    : QList(args.begin(), args.end())
{
}

bool ExportTypeList::contains(const QString& id) const
{
    return std::find_if(cbegin(), cend(), [id](const ExportType& type) {
        return type.id == id;
    }) != cend();
}

const ExportType& ExportTypeList::getById(const QString& id) const
{
    auto it = std::find_if(cbegin(), cend(), [id](const ExportType& type) {
        return type.id == id;
    });

    if (it != cend()) {
        return *it;
    }

    static ExportType null;
    return null;
}

QVariantList ExportTypeList::toVariantList() const
{
    QVariantList result;
    for (const ExportType& type : *this) {
        result << type.toMap();
    }
    return result;
}

QVariantMap ExportType::toMap() const
{
    return {
        { "id", id },
        { "name", name },
        { "subtypes", subtypes.toVariantList() },
        { "settingsPagePath", settingsPagePath }
    };
}

std::vector<std::string> ExportType::filter() const
{
    QStringList filterSuffixes;
    for (const QString& suffix : suffixes) {
        filterSuffixes << QString("*.%1").arg(suffix);
    }

    return { QString("%1 (%2)").arg(filterName, filterSuffixes.join(" ")).toStdString() };
}

bool ExportType::hasSubtypes() const
{
    return !subtypes.isEmpty();
}

ExportType ExportType::makeWithSuffixes(const QStringList& suffixes, const QString& name, const QString& filterName,
                                        const QString& settingsPagePath)
{
    ExportType type;

    if (suffixes.isEmpty()) {
        return type;
    }

    type.id = suffixes.front();
    type.suffixes = suffixes;
    type.name = name;
    type.filterName = filterName;
    type.settingsPagePath = settingsPagePath;
    return type;
}

ExportType ExportType::makeWithSubtypes(const ExportTypeList& subtypes, const QString& name)
{
    ExportType type;

    if (subtypes.isEmpty()) {
        return type;
    }

    type.id = subtypes.front().id;
    type.subtypes = subtypes;
    type.name = name;
    return type;
}
