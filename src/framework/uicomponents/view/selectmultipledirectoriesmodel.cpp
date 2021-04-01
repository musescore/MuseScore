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
    : QAbstractListModel(parent)
{
    m_selectionModel = new ItemMultiSelectionModel(this);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
        QModelIndexList indexes;
        indexes << selected.indexes() << deselected.indexes();
        QSet<QModelIndex> indexesSet(indexes.begin(), indexes.end());
        for (const QModelIndex& index: indexesSet) {
            emit dataChanged(index, index, { SelectedRole });
        }
    });
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

void SelectMultipleDirectoriesModel::load(const QString& directoriesStr)
{
    beginResetModel();
    m_directories = io::pathsFromString(directoriesStr.toStdString());
    endResetModel();
}

void SelectMultipleDirectoriesModel::selectRow(int row)
{
    m_selectionModel->select(index(row));
}

void SelectMultipleDirectoriesModel::deleteSelectedDirectories()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    beginResetModel();

    QList<io::path> directoriesToRemove;
    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        directoriesToRemove << m_directories[index.row()];
    }

    for (int i = m_directories.size() - 1; i >= 0; i--) {
        if (directoriesToRemove.contains(m_directories[i])) {
            m_directories.erase(m_directories.begin() + i);
        }
    }

    endResetModel();
}

void SelectMultipleDirectoriesModel::addDirectory()
{
    io::path path = interactive()->selectDirectory(qtrc("uicomponents", "Choose a directory"), m_lastSelectedDirectory);
    if (path.empty()) {
        return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_directories.push_back(path);
    endInsertRows();

    m_lastSelectedDirectory = path.toQString();
}

QStringList SelectMultipleDirectoriesModel::directories() const
{
    QStringList directories;
    for (const io::path& directory: m_directories) {
        directories << directory.toQString();
    }

    return directories;
}
