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

#pragma once

#include <QAbstractListModel>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "imusesoundsrepository.h"

namespace mu::musesounds {
class MuseSoundsListModel : public QAbstractListModel, public muse::async::Asyncable, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

    Inject<muse::IInteractive> interactive = { this };
    Inject<IMuseSoundsRepository> repository = { this };

public:
    explicit MuseSoundsListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

    bool isEmpty() const;

signals:
    void isEmptyChanged();

private:
    enum Roles {
        rCatalogueTitle = Qt::UserRole + 1,
        rCatalogueSoundLibraries
    };

    void setSoundsCatalogs(const SoundCatalogueInfoList& soundsCatalogs);

    SoundCatalogueInfoList m_soundsCatalogs;
};
}
