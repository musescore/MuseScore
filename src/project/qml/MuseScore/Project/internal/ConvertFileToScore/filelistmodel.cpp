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

#include "filelistmodel.h"

#include <QFileInfo>

#include "project/types/filecategory.h"

#include "ui/view/iconcodes.h"

#include "global/dataformatter.h"
#include "global/translation.h"

using namespace mu::project;
using namespace muse::ui;

static FileCategory resolveFileCategory(const QStringList& paths)
{
    if (paths.isEmpty()) {
        return FileCategory::Unknown;
    }

    return fileCategoryFromPath(muse::io::path_t(paths.first()));
}

FileListModel::FileListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant FileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_paths.size()) {
        return QVariant();
    }

    const QString& path = m_paths.at(index.row());

    switch (role) {
    case PathRole:
        return path;
    case FileNameRole:
        return QFileInfo(path).fileName();
    case FileSizeRole:
        return muse::DataFormatter::formatFileSize(size_t(QFileInfo(path).size())).toQString();
    }

    return QVariant();
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_paths.size();
}

QHash<int, QByteArray> FileListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { PathRole, "pathRole" },
        { FileNameRole, "fileNameRole" },
        { FileSizeRole, "fileSizeRole" }
    };

    return roles;
}

QStringList FileListModel::paths() const
{
    return m_paths;
}

void FileListModel::load(const QStringList& paths, const ConvertConfig& config)
{
    m_config = config;

    setPaths(paths, resolveFileCategory(paths));
}

void FileListModel::setPaths(const QStringList& paths, FileCategory category)
{
    if (paths == m_paths) {
        return;
    }

    beginResetModel();
    m_paths = paths;
    endResetModel();

    m_fileCategory = category;

    emit pathsChanged();
    updateTotalSizeBytes();
    updateConvertLimits();
    updateExceedsLimits();
    updateUsedSizeLabel();
}

void FileListModel::clear()
{
    setPaths({}, FileCategory::Unknown);
}

FileCategory FileListModel::fileCategory() const
{
    return m_fileCategory;
}

FileListModel::FileCategoryQml FileListModel::fileCategoryQml() const
{
    return FileCategoryQml(m_fileCategory);
}

int FileListModel::fileIconCode() const
{
    switch (m_fileCategory) {
    case FileCategory::Image:
        return int(IconCode::Code::IMAGE_MOUNTAINS);
    case FileCategory::Audio:
        return int(IconCode::Code::MUSIC_NOTES);
    case FileCategory::Unknown:
    case FileCategory::Pdf:
        break;
    }

    return int(IconCode::Code::NEW_FILE);
}

QString FileListModel::combinedFilesNote() const
{
    if (m_fileCategory != FileCategory::Image || m_paths.size() <= 1) {
        return QString();
    }

    return muse::qtrc("project/convert", "Images will be combined into one score in the order shown here");
}

QVariantMap FileListModel::convertLimits() const
{
    QVariantMap limits;
    limits["maxFileCount"] = m_maxFileCount;
    limits["maxCombinedSizeBytes"] = m_maxCombinedSizeBytes;
    return limits;
}

QString FileListModel::usedSizeLabel() const
{
    return m_usedSizeLabel;
}

bool FileListModel::exceedsLimits() const
{
    return m_exceedsLimits;
}

QVariantMap FileListModel::get(int index)
{
    QVariantMap result;

    const QModelIndex idx = this->index(index, 0);
    const QHash<int, QByteArray> roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
        result[it.value()] = idx.data(it.key());
    }

    return result;
}

void FileListModel::removeAt(int index)
{
    if (index < 0 || index >= m_paths.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_paths.removeAt(index);
    endRemoveRows();

    if (m_paths.isEmpty()) {
        m_fileCategory = FileCategory::Unknown;
    }

    emit pathsChanged();
    updateTotalSizeBytes();
    updateConvertLimits();
    updateExceedsLimits();
    updateUsedSizeLabel();
}

void FileListModel::move(int from, int to)
{
    if (from < 0 || from >= m_paths.size() || to < 0 || to >= m_paths.size() || from == to) {
        return;
    }

    const int destination = to > from ? to + 1 : to;
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination);
    m_paths.move(from, to);
    endMoveRows();

    emit pathsChanged();
}

void FileListModel::updateTotalSizeBytes()
{
    m_totalSizeBytes = 0;
    for (const QString& path : m_paths) {
        m_totalSizeBytes += QFileInfo(path).size();
    }
}

void FileListModel::updateConvertLimits()
{
    int maxFileCount = 0;
    qint64 maxCombinedSizeBytes = 0;

    switch (m_fileCategory) {
    case FileCategory::Audio:
        maxFileCount = m_config.audio2score.file.maxFiles;
        maxCombinedSizeBytes = m_config.audio2score.file.maxFileSizeBytes;
        break;
    case FileCategory::Pdf:
        maxFileCount = m_config.omr.pdf.maxFiles;
        maxCombinedSizeBytes = m_config.omr.pdf.maxFileSizeBytes;
        break;
    case FileCategory::Image:
    case FileCategory::Unknown:
        maxFileCount = m_config.omr.images.maxFiles;
        maxCombinedSizeBytes = m_config.omr.images.maxFileSizeBytes;
        break;
    }

    if (m_maxFileCount == maxFileCount && m_maxCombinedSizeBytes == maxCombinedSizeBytes) {
        return;
    }

    m_maxFileCount = maxFileCount;
    m_maxCombinedSizeBytes = maxCombinedSizeBytes;
    emit convertLimitsChanged();
}

void FileListModel::updateExceedsLimits()
{
    bool exceeds = false;

    if (m_maxFileCount > 0 && m_paths.size() > m_maxFileCount) {
        exceeds = true;
    }

    if (!exceeds && m_maxCombinedSizeBytes > 0 && m_totalSizeBytes > m_maxCombinedSizeBytes) {
        exceeds = true;
    }

    if (m_exceedsLimits == exceeds) {
        return;
    }

    m_exceedsLimits = exceeds;
    emit exceedsLimitsChanged();
}

void FileListModel::updateUsedSizeLabel()
{
    const qint64 maxBytes = m_maxCombinedSizeBytes;

    QString label;
    if (maxBytes > 0 && m_totalSizeBytes >= maxBytes * 0.75) { // only show once 75% of the limit is used
        const QString totalSize = muse::DataFormatter::formatFileSize(size_t(m_totalSizeBytes));
        const QString maxSize = muse::DataFormatter::formatFileSize(size_t(maxBytes));
        //: %1 and %2 are pre-formatted file sizes including units, e.g. "15 MB/20 MB used"
        label = muse::qtrc("project/convert", "%1/%2 used").arg(totalSize, maxSize);
    }

    if (m_usedSizeLabel == label) {
        return;
    }

    m_usedSizeLabel = label;
    emit usedSizeLabelChanged();
}
