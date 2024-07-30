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
#ifndef MUSE_AUTOBOT_ABFILESMODEL_H
#define MUSE_AUTOBOT_ABFILESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iautobot.h"
#include "async/asyncable.h"

namespace muse::autobot {
class AbFilesModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Inject<IAutobot> autobot = { this };

public:
    explicit AbFilesModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:

    void update();
    void updateFile(const File& f);
    QString fileStatus(const File& f) const;

    enum Roles {
        FileTitle = Qt::UserRole + 1,
        FileIndex,
        IsCurrentFile,
        FileStatus
    };

    ValNt<Files> m_files;
    ValCh<int> m_fileIndex;
};
}

#endif // MUSE_AUTOBOT_ABFILESMODEL_H
