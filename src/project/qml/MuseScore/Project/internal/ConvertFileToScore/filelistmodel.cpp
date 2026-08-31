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

#include "project/types/projecttypes.h"

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

    const std::string suffix = QFileInfo(paths.first()).suffix().toLower().toStdString();

    if (isImageFileSuffix(suffix)) {
        return FileCategory::Image;
    }

    if (isAudioFileSuffix(suffix)) {
        return FileCategory::Audio;
    }

    return FileCategory::Unknown;
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

void FileListModel::setPaths(const QStringList& paths)
{
    if (paths == m_paths) {
        return;
    }

    beginResetModel();
    m_paths = paths;
    endResetModel();

    m_fileCategory = resolveFileCategory(m_paths);

    emit pathsChanged();
    updateTotalSizeBytes();
    updateExceedsLimits();
    updateUsedSizeString();
}

int FileListModel::fileIconCode() const
{
    switch (m_fileCategory) {
    case FileCategory::Image:
        return int(IconCode::Code::IMAGE_MOUNTAINS);
    case FileCategory::Audio:
        return int(IconCode::Code::AUDIO);
    case FileCategory::Unknown:
    case FileCategory::Pdf:
        break;
    }

    return int(IconCode::Code::NEW_FILE);
}

QString FileListModel::combinedFilesNote() const
{
    if (m_fileCategory != FileCategory::Image) {
        return QString();
    }

    return muse::qtrc("project/convert", "Images will be combined into one score in the order you upload them");
}

QVariantMap FileListModel::convertLimits() const
{
    return m_convertLimits;
}

void FileListModel::setConvertLimits(const QVariantMap& limits)
{
    if (m_convertLimits == limits) {
        return;
    }

    m_convertLimits = limits;
    emit convertLimitsChanged();
    updateExceedsLimits();
    updateUsedSizeString();
}

int FileListModel::maxFileCount() const
{
    return m_convertLimits.value("maxFileCount", 0).toInt();
}

qint64 FileListModel::maxCombinedSizeBytes() const
{
    return m_convertLimits.value("maxCombinedSizeBytes", 0).toLongLong();
}

QString FileListModel::usedSizeString() const
{
    return m_usedSizeString;
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

    emit pathsChanged();
    updateTotalSizeBytes();
    updateExceedsLimits();
    updateUsedSizeString();
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

void FileListModel::updateExceedsLimits()
{
    bool exceeds = false;

    const int maxCount = maxFileCount();
    if (maxCount > 0 && m_paths.size() > maxCount) {
        exceeds = true;
    }

    if (!exceeds) {
        const qint64 maxBytes = maxCombinedSizeBytes();
        if (maxBytes > 0 && m_totalSizeBytes > maxBytes) {
            exceeds = true;
        }
    }

    if (m_exceedsLimits == exceeds) {
        return;
    }

    m_exceedsLimits = exceeds;
    emit exceedsLimitsChanged();
}

void FileListModel::updateUsedSizeString()
{
    const qint64 maxBytes = maxCombinedSizeBytes();

    QString sizeString;
    if (maxBytes > 0) {
        const QString totalSize = muse::DataFormatter::formatFileSize(size_t(m_totalSizeBytes));
        const QString maxSize = muse::DataFormatter::formatFileSize(size_t(maxBytes));
        sizeString = muse::qtrc("project/convert", "%1/%2 used").arg(totalSize, maxSize);
    }

    if (m_usedSizeString == sizeString) {
        return;
    }

    m_usedSizeString = sizeString;
    emit usedSizeStringChanged();
}
