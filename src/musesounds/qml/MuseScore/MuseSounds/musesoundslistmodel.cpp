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

#include <unordered_set>

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
    list.reserve(soundLibraryInfoList.size());

    for (const SoundLibraryInfo& soundLibraryInfo : soundLibraryInfoList) {
        list << soundLibraryInfoToMap(soundLibraryInfo);
    }

    return list;
}

static SoundLibraryInfoList filteredLibraries(const SoundLibraryInfoList& libs, const QString& search,
                                              std::unordered_set<QString>& matchedCodes)
{
    SoundLibraryInfoList result;

    for (const SoundLibraryInfo& lib : libs) {
        if (lib.title.toQString().contains(search, Qt::CaseInsensitive)
            || lib.subtitle.toQString().contains(search, Qt::CaseInsensitive)) {
            // The same lib can be listed under several catalogs
            // Only show its first match
            if (matchedCodes.insert(lib.code.toQString()).second) {
                result.push_back(lib);
            }
        }
    }

    return result;
}

MuseSoundsListModel::MuseSoundsListModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void MuseSoundsListModel::load()
{
    if (!m_searchText.isEmpty()) {
        m_searchText.clear();
        emit searchTextChanged();
    }

    m_filteredCatalogs.clear();

    setSoundsCatalogs(repository()->soundsCatalogs());
    repository()->soundsCatalogsChanged().onNotify(this, [this](){
        setSoundsCatalogs(repository()->soundsCatalogs());
    });
}

QString MuseSoundsListModel::searchText() const
{
    return m_searchText;
}

void MuseSoundsListModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    bool narrowing = !m_searchText.isEmpty() && !text.isEmpty()
                     && text.startsWith(m_searchText, Qt::CaseInsensitive);
    m_searchText = text;
    emit searchTextChanged();
    applyFilter(narrowing);
}

QVariant MuseSoundsListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const SoundCatalogInfo& soundCategoryInfo = m_filteredCatalogs.at(index.row());

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
    return static_cast<int>(m_filteredCatalogs.size());
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

bool MuseSoundsListModel::noResultsFound() const
{
    return !m_searchText.isEmpty() && m_filteredCatalogs.empty();
}

void MuseSoundsListModel::setSoundsCatalogs(const SoundCatalogInfoList& soundsCatalogs)
{
    m_soundsCatalogs = soundsCatalogs;
    applyFilter();
}

void MuseSoundsListModel::applyFilter(bool narrowing)
{
    TRACEFUNC;

    SoundCatalogInfoList newFiltered;
    if (m_searchText.isEmpty()) {
        newFiltered = m_soundsCatalogs;
    } else {
        // Narrowing: scan only the already-filtered (smaller) set
        const SoundCatalogInfoList& source = narrowing ? m_filteredCatalogs : m_soundsCatalogs;
        std::unordered_set<QString> matchedCodes;
        for (const SoundCatalogInfo& catalogue : source) {
            SoundLibraryInfoList libs = filteredLibraries(catalogue.soundLibraries, m_searchText, matchedCodes);
            if (!libs.empty()) {
                newFiltered.push_back({ catalogue.title, std::move(libs) });
            }
        }
    }

    if (newFiltered != m_filteredCatalogs) {
        beginResetModel();
        m_filteredCatalogs = std::move(newFiltered);
        endResetModel();
    }

    emit isEmptyChanged();
    emit noResultsFoundChanged();
}
