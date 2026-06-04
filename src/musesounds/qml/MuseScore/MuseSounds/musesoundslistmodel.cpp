/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void MuseSoundsListModel::load()
{
    setSoundsCatalogs(repository()->soundsCatalogs());
    repository()->soundsCatalogsChanged().onNotify(this, [this](){
        setSoundsCatalogs(repository()->soundsCatalogs());
    });
}

QVariant MuseSoundsListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const SoundCatalogInfo& soundCategoryInfo = m_soundsCatalogs.at(index.row());

    switch (role) {
    case rCatalogTitle:
        return soundCategoryInfo.title.toQString();
    case rCatalogSoundLibraries:
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
        { rCatalogTitle, "catalogTitle" },
        { rCatalogSoundLibraries, "catalogSoundsLibraries" }
    };
}

bool MuseSoundsListModel::isEmpty() const
{
    return m_soundsCatalogs.empty();
}

void MuseSoundsListModel::setSoundsCatalogs(const SoundCatalogInfoList& soundsCatalogs)
{
    beginResetModel();

    m_soundsCatalogs = soundsCatalogs;

    endResetModel();

    emit isEmptyChanged();
}
