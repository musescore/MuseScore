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

#include <variant>

#include <QString>
#include <QUrl>

#include "io/path.h"
#include "log.h"

#include "cloud/cloudtypes.h"
#include "notation/inotation.h"
#include "notation/notationtypes.h"
#include "inotationwriter.h"

namespace mu::project {
struct ProjectCreateOptions
{
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

    io::path_t templatePath;

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

struct CloudProjectInfo {
    QUrl sourceUrl;
    int revisionId = 0;
    QString name;

    cloud::Visibility visibility = cloud::Visibility::Private;

    bool isValid() const
    {
        return !name.isEmpty();
    }
};

struct ExportInfo {
    QString id;
    io::path_t projectPath;
    io::path_t exportPath;
    INotationWriter::UnitType unitType;
    std::vector<notation::INotationPtr> notations;

    bool operator==(const ExportInfo& other) const
    {
        return id == other.id
               && projectPath == other.projectPath
               && exportPath == other.exportPath
               && unitType == other.unitType
               && notations == other.notations;
    }

    bool operator!=(const ExportInfo other) const
    {
        return !(*this == other);
    }
};

struct SaveLocation
{
    SaveLocationType type = SaveLocationType::Undefined;

    std::variant<io::path_t, CloudProjectInfo> data;

    bool isLocal() const
    {
        return type == SaveLocationType::Local
               && std::holds_alternative<io::path_t>(data);
    }

    bool isCloud() const
    {
        return type == SaveLocationType::Cloud
               && std::holds_alternative<CloudProjectInfo>(data);
    }

    bool isValid() const
    {
        return isLocal() || isCloud();
    }

    const io::path_t& localPath() const
    {
        IF_ASSERT_FAILED(isLocal()) {
            static io::path_t null;
            return null;
        }

        return std::get<io::path_t>(data);
    }

    const CloudProjectInfo& cloudInfo() const
    {
        IF_ASSERT_FAILED(isCloud()) {
            static CloudProjectInfo null;
            return null;
        }

        return std::get<CloudProjectInfo>(data);
    }

    SaveLocation() = default;

    SaveLocation(SaveLocationType type, const std::variant<io::path_t, CloudProjectInfo>& data = {})
        : type(type), data(data) {}

    SaveLocation(const io::path_t& localPath)
        : type(SaveLocationType::Local), data(localPath) {}

    SaveLocation(const CloudProjectInfo& cloudInfo)
        : type(SaveLocationType::Cloud), data(cloudInfo) {}
};

struct ProjectMeta
{
    io::path_t filePath;

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

    io::path_t fileName(bool includingExtension = true) const
    {
        return io::filename(filePath, includingExtension);
    }

    bool operator==(const ProjectMeta& other) const
    {
        bool equal = filePath == other.filePath;
        equal &= title == other.title;
        equal &= subtitle == other.subtitle;
        equal &= composer == other.composer;
        equal &= lyricist == other.lyricist;
        equal &= copyright == other.copyright;
        equal &= translator == other.translator;
        equal &= arranger == other.arranger;
        equal &= partsCount == other.partsCount;
        equal &= creationDate == other.creationDate;
        equal &= source == other.source;
        equal &= platform == other.platform;
        equal &= musescoreVersion == other.musescoreVersion;
        equal &= musescoreRevision == other.musescoreRevision;
        equal &= mscVersion == other.mscVersion;
        equal &= additionalTags == other.additionalTags;
        equal &= thumbnail.toImage() == other.thumbnail.toImage();
        return equal;
    }

    bool operator!=(const ProjectMeta& other) const
    {
        return !(*this == other);
    }
};

using ProjectMetaList = QList<ProjectMeta>;

struct Template
{
    QString categoryTitle;
    ProjectMeta meta;
};

using Templates = QList<Template>;

class GenerateAudioTimePeriod
{
    Q_GADGET

public:
    enum class Type {
        Never = 0,
        Always,
        AfterCertainNumberOfSaves
    };
    Q_ENUM(Type)
};

using GenerateAudioTimePeriodType = GenerateAudioTimePeriod::Type;

class Migration
{
    Q_GADGET

public:
    enum class Type
    {
        Unknown,
        Pre_3_6,
        Ver_3_6
    };
    Q_ENUM(Type)
};

using MigrationType = Migration::Type;

inline std::vector<MigrationType> allMigrationTypes()
{
    static const std::vector<MigrationType> types {
        MigrationType::Pre_3_6,
        MigrationType::Ver_3_6
    };

    return types;
}
}

#endif // MU_PROJECT_PROJECTTYPES_H
