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

#include "ui/view/iconcodes.h"

namespace mu::project {
class FileListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int fileIconCode READ fileIconCode NOTIFY countChanged)

    QML_ELEMENT

public:
    explicit FileListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int fileIconCode() const;

    Q_INVOKABLE void setPaths(const QStringList& paths);
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void move(int from, int to);

    Q_INVOKABLE QStringList paths() const;
    Q_INVOKABLE QString baseName(int index) const;
    Q_INVOKABLE QString fileName(int index) const;

signals:
    void countChanged();

private:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        FileNameRole
    };

    QStringList m_paths;
};
}
