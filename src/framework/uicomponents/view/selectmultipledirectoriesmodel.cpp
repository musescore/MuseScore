//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "selectmultipledirectoriesmodel.h"

#include "translation.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::uicomponents;

SelectMultipleDirectoriesModel::SelectMultipleDirectoriesModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this))
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &SelectMultipleDirectoriesModel::selectionChanged);
}

QVariant SelectMultipleDirectoriesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    io::path path = m_directories[index.row()];
    switch (role) {
    case TitleRole: return path.toQString();
    case SelectedRole: return m_selectionModel->isSelected(index);
    }

    return QVariant();
}

bool SelectMultipleDirectoriesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case TitleRole:
        m_directories[index.row()] = value.toString().toStdString();
        emit dataChanged(index, index, { TitleRole });
        return true;
    }

    return false;
}

int SelectMultipleDirectoriesModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_directories.size());
}

QHash<int, QByteArray> SelectMultipleDirectoriesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, "titleRole" },
        { SelectedRole, "selectedRole" }
    };

    return roles;
}

QItemSelectionModel* SelectMultipleDirectoriesModel::selection() const
{
    return m_selectionModel;
}

void SelectMultipleDirectoriesModel::load(const QString& dir, const QString& directoriesStr)
{
    beginResetModel();
    m_directories = io::pathsFromString(directoriesStr.toStdString());
    m_originDirectories = m_directories;
    m_dir = dir;
    endResetModel();
}

void SelectMultipleDirectoriesModel::selectRow(int row)
{
    if (!isIndexValid(row)) {
        return;
    }

    QModelIndex rowIndex = index(row);
    m_selectionModel->select(rowIndex);
    emit dataChanged(index(0), index(rowCount() - 1), { SelectedRole });
}

void SelectMultipleDirectoriesModel::removeSelectedDirectories()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    QList<io::path> directoriesToRemove;
    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        directoriesToRemove << m_directories[index.row()];
    }

    for (int i = m_directories.size() - 1; i >= 0; i--) {
        if (directoriesToRemove.contains(m_directories[i])) {
            doRemoveDirectory(i);
        }
    }

    m_selectionModel->clear();
}

void SelectMultipleDirectoriesModel::addDirectory()
{
    io::path path = interactive()->selectDirectory(qtrc("uicomponents", "Choose a directory"), m_dir.toStdString());
    if (path.empty()) {
        return;
    }

    if (std::find(m_directories.cbegin(), m_directories.cend(), path) != m_directories.end()) {
        return;
    }

    int row = rowCount();

    beginInsertRows(QModelIndex(), row, row);
    m_directories.push_back(path);
    endInsertRows();

    m_dir = path.toQString();
    emit directoryAdded(row);
}

QStringList SelectMultipleDirectoriesModel::originDirectories() const
{
    QStringList directories;
    for (const io::path& directory : m_originDirectories) {
        directories << directory.toQString();
    }

    return directories;
}

QStringList SelectMultipleDirectoriesModel::directories() const
{
    QStringList directories;
    for (const io::path& directory : m_directories) {
        directories << directory.toQString();
    }

    return directories;
}

bool SelectMultipleDirectoriesModel::isRemovingAvailable() const
{
    return m_selectionModel->hasSelection();
}

bool SelectMultipleDirectoriesModel::isIndexValid(int index) const
{
    return 0 <= index && index < static_cast<int>(m_directories.size());
}

void SelectMultipleDirectoriesModel::doRemoveDirectory(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_directories.erase(m_directories.begin() + index);
    endRemoveRows();
}
