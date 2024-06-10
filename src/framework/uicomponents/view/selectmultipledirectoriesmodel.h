/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MUSE_UICOMPONENTS_SELECTMULTIPLEDIRECTORIESMODEL_H
#define MUSE_UICOMPONENTS_SELECTMULTIPLEDIRECTORIESMODEL_H

#include <QAbstractListModel>
#include <QItemSelection>

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace muse::uicomponents {
class ItemMultiSelectionModel;
class SelectMultipleDirectoriesModel : public QAbstractListModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY selectionChanged)

    muse::Inject<IInteractive> interactive = { this };

public:
    explicit SelectMultipleDirectoriesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QItemSelectionModel* selection() const;

    Q_INVOKABLE void load(const QString& startDir, const QString& directoriesStr);

    Q_INVOKABLE void selectRow(int row);
    Q_INVOKABLE void removeSelectedDirectories();
    Q_INVOKABLE void addDirectory();

    Q_INVOKABLE QString directories() const;

    bool isRemovingAvailable() const;

signals:
    void selectionChanged();
    void directoryAdded(int index);

private:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        SelectedRole
    };

    bool isIndexValid(int index) const;
    int indexOf(const io::path_t& path) const;

    void doRemoveDirectory(int index);

    io::paths_t m_directories;
    io::path_t m_dir;

    uicomponents::ItemMultiSelectionModel* m_selectionModel = nullptr;
};
}

#endif // MUSE_UICOMPONENTS_SELECTMULTIPLEDIRECTORIESMODEL_H
