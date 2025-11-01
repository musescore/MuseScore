/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "modularity/ioc.h"
#include "iautobot.h"
#include "async/asyncable.h"

namespace muse::autobot {
class AbFilesModel : public QAbstractListModel, public LazyInjectable, public async::Asyncable
{
    Q_OBJECT

    LazyInject<IAutobot> autobot = { this };

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
