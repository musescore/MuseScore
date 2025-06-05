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

#include "musesoundslistmodel.h"

#include "log.h"

using namespace mu::musesounds;

static const QVariantMap soundLibraryInfoToMap(const SoundLibraryInfo& soundLibraryInfo)
{
    QVariantMap map;
    map["code"] = soundLibraryInfo.code.toQString();
    map["title"] = soundLibraryInfo.title.toQString();
    map["subtitle"] = soundLibraryInfo.subtitle.toQString();
    map["thumbnail"] = QString::fromStdString(soundLibraryInfo.thumbnail.toString());
    map["uri"] = QString::fromStdString(soundLibraryInfo.uri.toString());

    return map;
}

static const QVariantList soundInfoListToVariantList(const SoundLibraryInfoList& soundLibraryInfoList)
{
    QVariantList list;

    for (const SoundLibraryInfo& soundLibraryInfo : soundLibraryInfoList) {
        list << soundLibraryInfoToMap(soundLibraryInfo);
    }

    return list;
}

MuseSoundsListModel::MuseSoundsListModel(QObject* parent)
    : QAbstractListModel(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void MuseSoundsListModel::load()
{
    setSoundsCatalogs(repository()->soundsCatalogueList());
    repository()->soundsCatalogueListChanged().onNotify(this, [this](){
        setSoundsCatalogs(repository()->soundsCatalogueList());
    });
}

QVariant MuseSoundsListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const SoundCatalogueInfo& soundCategoryInfo = m_soundsCatalogs.at(index.row());

    switch (role) {
    case rCatalogueTitle:
        return soundCategoryInfo.title.toQString();
    case rCatalogueSoundLibraries:
        return soundInfoListToVariantList(soundCategoryInfo.soundLibraries);
    }

    return QVariant();
}

int MuseSoundsListModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_soundsCatalogs.size());
}

QHash<int, QByteArray> MuseSoundsListModel::roleNames() const
{
    return {
        { rCatalogueTitle, "catalogueTitle" },
        { rCatalogueSoundLibraries, "catalogueSoundsLibraries" }
    };
}

bool MuseSoundsListModel::isEmpty() const
{
    return m_soundsCatalogs.empty();
}

void MuseSoundsListModel::setSoundsCatalogs(const SoundCatalogueInfoList& soundsCatalogs)
{
    beginResetModel();

    m_soundsCatalogs = soundsCatalogs;

    endResetModel();

    emit isEmptyChanged();
}
