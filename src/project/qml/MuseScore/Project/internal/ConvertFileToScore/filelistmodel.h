/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

#include "project/types/filecategory.h"
#include "project/types/converttypes.h"

namespace mu::project {
class FileListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY pathsChanged)
    Q_PROPERTY(FileCategoryQml fileCategory READ fileCategoryQml NOTIFY pathsChanged)
    Q_PROPERTY(int fileIconCode READ fileIconCode NOTIFY pathsChanged)
    Q_PROPERTY(QString combinedFilesNote READ combinedFilesNote NOTIFY pathsChanged)
    Q_PROPERTY(QVariantMap convertLimits READ convertLimits NOTIFY convertLimitsChanged)
    Q_PROPERTY(QString usedSizeLabel READ usedSizeLabel NOTIFY usedSizeLabelChanged)
    Q_PROPERTY(bool exceedsLimits READ exceedsLimits NOTIFY exceedsLimitsChanged)

    QML_ELEMENT

public:
    //! NOTE: must be in sync with mu::project::FileCategory
    enum class FileCategoryQml {
        Unknown,
        Audio,
        Pdf,
        Image
    };
    Q_ENUM(FileCategoryQml)

    explicit FileListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    void load(const QStringList& paths, const ConvertConfig& config);

    Q_INVOKABLE QStringList paths() const;
    void setPaths(const QStringList& paths, FileCategory category);
    void clear();

    FileCategory fileCategory() const;
    FileCategoryQml fileCategoryQml() const;
    int fileIconCode() const;
    QString combinedFilesNote() const;
    QVariantMap convertLimits() const;
    QString usedSizeLabel() const;
    bool exceedsLimits() const;

    Q_INVOKABLE QVariantMap get(int index);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void move(int from, int to);

signals:
    void pathsChanged();
    void convertLimitsChanged();
    void exceedsLimitsChanged();
    void usedSizeLabelChanged();

private:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        FileNameRole,
        FileSizeRole
    };

    void updateTotalSizeBytes();
    void updateConvertLimits();
    void updateExceedsLimits();
    void updateUsedSizeLabel();

    QStringList m_paths;
    FileCategory m_fileCategory = FileCategory::Unknown;
    ConvertConfig m_config;
    int m_maxFileCount = 0;
    qint64 m_maxCombinedSizeBytes = 0;
    bool m_exceedsLimits = false;
    qint64 m_totalSizeBytes = 0;
    QString m_usedSizeLabel;
};
}
