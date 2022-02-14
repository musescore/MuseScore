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
#include "log.h"

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

enum class CloudProjectVisibility {
    Private,
    Public
};

struct SaveLocation
{
    struct LocalInfo {
        io::path path;
    };

    struct CloudInfo {
        // TODO(save-to-cloud)
    };

    SaveLocationType type = SaveLocationType::Undefined;
    std::variant<LocalInfo, CloudInfo> info;

    bool isLocal() const
    {
        return type == SaveLocationType::Local
               && std::holds_alternative<LocalInfo>(info);
    }

    bool isCloud() const
    {
        return type == SaveLocationType::Cloud
               && std::holds_alternative<CloudInfo>(info);
    }

    bool isValid() const
    {
        return isLocal() || isCloud();
    }

    const LocalInfo& localInfo() const
    {
        IF_ASSERT_FAILED(isLocal()) {
            static LocalInfo null;
            return null;
        }

        return std::get<LocalInfo>(info);
    }

    const CloudInfo& cloudInfo() const
    {
        IF_ASSERT_FAILED(isCloud()) {
            static CloudInfo null;
            return null;
        }

        return std::get<CloudInfo>(info);
    }

    SaveLocation() = default;

    SaveLocation(const LocalInfo& localInfo)
        : type(SaveLocationType::Local), info(localInfo) {}

    SaveLocation(const CloudInfo& cloudInfo)
        : type(SaveLocationType::Cloud), info(cloudInfo) {}
};

struct ProjectMeta
{
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

    io::path fileName(bool includingExtension = true) const
    {
        return io::filename(filePath, includingExtension);
    }
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
