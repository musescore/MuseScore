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
#ifndef MU_PROJECT_PROJECTTYPES_H
#define MU_PROJECT_PROJECTTYPES_H

#include <QString>

#include "io/path.h"

#include "notation/notationtypes.h"

namespace mu::project {
struct ProjectCreateOptions
{
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

    io::path templatePath;

    notation::ScoreCreateOptions scoreOptions;
};

struct MigrationOptions
{
    // common
    int appVersion = 0;
    bool isApplyMigration = false;
    bool isAskAgain = true;

    bool isApplyLeland = true;
    bool isApplyEdwin = true;
    bool isApplyAutoSpacing = true;

    bool isValid() const { return appVersion != 0; }
};

enum class SaveMode
{
    Save,
    SaveAs,
    SaveCopy,
    SaveSelection,
    AutoSave
};

enum class SaveLocationType
{
    Undefined,
    Local,
    Cloud
};

struct ProjectMeta
{
    io::path fileName;
    io::path filePath;
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;
    QString translator;
    QString arranger;
    size_t partsCount = 0;
    QPixmap thumbnail;
    QDate creationDate;

    QString source;
    QString platform;
    QString musescoreVersion;
    int musescoreRevision = 0;
    int mscVersion = 0;

    QVariantMap additionalTags;
};

using ProjectMetaList = QList<ProjectMeta>;

struct Template
{
    QString categoryTitle;
    ProjectMeta meta;
};

using Templates = QList<Template>;

class Migration
{
    Q_GADGET

public:
    enum class Type
    {
        Unknown,
        Pre300,
        Post300AndPre362,
        Ver362
    };
    Q_ENUM(Type)
};

using MigrationType = Migration::Type;

inline std::vector<MigrationType> allMigrationTypes()
{
    static const std::vector<MigrationType> types {
        MigrationType::Pre300,
        MigrationType::Post300AndPre362,
        MigrationType::Ver362
    };

    return types;
}
}

#endif // MU_PROJECT_PROJECTTYPES_H
