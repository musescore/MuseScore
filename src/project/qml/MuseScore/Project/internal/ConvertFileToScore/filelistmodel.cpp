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

using namespace mu::project;
using namespace muse::ui;

static const QStringList IMAGE_EXTENSIONS { "jpg", "jpeg", "png" };

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
        { FileNameRole, "fileNameRole" }
    };

    return roles;
}

int FileListModel::fileIconCode() const
{
    if (m_paths.isEmpty()) {
        return int(IconCode::Code::PAGE);
    }

    const QString suffix = QFileInfo(m_paths.first()).suffix().toLower();

    if (IMAGE_EXTENSIONS.contains(suffix)) {
        return int(IconCode::Code::IMAGE_MOUNTAINS);
    }

    if (isAudioFileSuffix(suffix.toStdString())) {
        return int(IconCode::Code::AUDIO);
    }

    return int(IconCode::Code::PAGE);
}

void FileListModel::setPaths(const QStringList& paths)
{
    beginResetModel();
    m_paths = paths;
    endResetModel();

    emit countChanged();
}

void FileListModel::removeAt(int index)
{
    if (index < 0 || index >= m_paths.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_paths.removeAt(index);
    endRemoveRows();

    emit countChanged();
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
}

QStringList FileListModel::paths() const
{
    return m_paths;
}

QString FileListModel::baseName(int index) const
{
    if (index < 0 || index >= m_paths.size()) {
        return QString();
    }

    return QFileInfo(m_paths.at(index)).completeBaseName();
}

QString FileListModel::fileName(int index) const
{
    if (index < 0 || index >= m_paths.size()) {
        return QString();
    }

    return QFileInfo(m_paths.at(index)).fileName();
}
