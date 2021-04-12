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

#ifndef MU_USERSCORES_TYPES_H
#define MU_USERSCORES_TYPES_H

#include <QList>

#include "notation/notationtypes.h"
#include "notation/inotationwriter.h"

namespace mu::userscores {
struct Template : public notation::Meta {
    QString categoryTitle;

    Template() = default;
    Template(const notation::Meta& meta)
        : Meta(meta) {}
};

using Templates = QList<Template>;

using ExportUnitType = notation::INotationWriter::UnitType;

struct ExportType;
using ExportTypes = QList<ExportType>;
struct ExportType {
    QString id;
    QString name;
    QStringList allowedSuffixes;
    QString filter;
    ExportTypes subtypes;

    bool isValid() const
    {
        if (id.isEmpty()
            || allowedSuffixes.empty()
            || filter.isEmpty()) {
            return false;
        }

        return true;
    }

    QVariantMap toMap() const
    {
        QVariantList subtypesList;
        for (const ExportType& subtype : subtypes) {
            subtypesList << subtype.toMap();
        }

        //! NOTE Only properties relevant in QML
        return {
            { "id", id },
            { "name", name },
            { "subtypes", subtypesList }
        };
    }
};

//{
//public:
//    ExportTypes()
//        : QList<ExportType>() {}
//    ExportType(const QList<ExportType>& exportTypes)
//        : QList<ExportType>(exportTypes) {}
//
//    const ExportType& getById(const QString& id) {
//        auto it = std::find_if(cbegin(), cend(), [id](const ExportType& exportType) {
//            return exportType.id == id;
//        });
//
//        return *it;
//    }
//
//    QList<QVariantMap> toVariantList() const
//    {
//        QList<QVariantMap> variantList;
//        for (const ExportType& type : *this) {
//            variantList << type.toMap();
//        }
//        return variantList;
//    }
//};
}

#endif // MU_USERSCORES_TYPES_H
