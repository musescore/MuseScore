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
#include "instrumentlistmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::uicomponents;

static const QString ALL_INSTRUMENTS_GENRE_ID("ALL_INSTRUMENTS");
static const QString NONE_GROUP_ID("");

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QAbstractListModel(parent), m_selection(new ItemMultiSelectionModel(this))
{
    connect(m_selection, &ItemMultiSelectionModel::selectionChanged, this, &InstrumentListModel::selectionChanged);
}

QVariant InstrumentListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const CombinedInstrument& instrument = m_instruments[index.row()];

    switch (role) {
    case RoleName:
        return instrument.name;
    case RoleTraits: {
        QStringList traits;
        for (const InstrumentTemplate* templ : instrument.templates) {
            traits << templ->trait.name;
        }

        return traits;
    }
    case RoleIsSelected:
        return m_selection->isSelected(index);
    }

    return QVariant();
}

int InstrumentListModel::rowCount(const QModelIndex&) const
{
    return m_instruments.size();
}

QHash<int, QByteArray> InstrumentListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleName, "name" },
        { RoleIsSelected, "isSelected" },
        { RoleTraits, "traits" }
    };

    return roles;
}

void InstrumentListModel::load(bool canSelectMultipleInstruments, const QString& currentInstrumentId)
{
    TRACEFUNC;

    m_selection->setSingleItemSelectionMode(!canSelectMultipleInstruments);

    RetValCh<InstrumentsMeta> instrumentsMeta = repository()->instrumentsMeta();
    if (!instrumentsMeta.ret) {
        LOGE() << instrumentsMeta.ret.toString();
    }

    m_instrumentsMeta = instrumentsMeta.val;

    for (const InstrumentTemplate* templ: m_instrumentsMeta.instrumentTemplates) {
        if (templ->id == currentInstrumentId) {
            setCurrentGroup(templ->groupId);
            break;
        }
    }

    setCurrentGenre(COMMON_GENRE_ID);
    emit genresChanged();

    m_isInited = true;

    loadInstruments();
}

QStringList InstrumentListModel::genres() const
{
    QStringList result;

    for (const InstrumentGenre* genre: availableGenres()) {
        result << genre->name;
    }

    return result;
}

QStringList InstrumentListModel::groups() const
{
    QStringList result;

    for (const InstrumentGroup* group: availableGroups()) {
        result << group->name;
    }

    return result;
}

void InstrumentListModel::loadInstruments()
{
    if (!m_isInited) {
        return;
    }

    TRACEFUNC;

    using InstrumentName = QString;
    QHash<InstrumentName, InstrumentTemplateList> templatesByInstrumentName;

    for (const InstrumentTemplate* templ: m_instrumentsMeta.instrumentTemplates) {
        if (!isInstrumentAccepted(*templ)) {
            continue;
        }

        const InstrumentName& instrumentName = templ->trackName;

        if (templ->trait.isDefault) {
            templatesByInstrumentName[instrumentName].prepend(templ);
        } else {
            templatesByInstrumentName[instrumentName] << templ;
        }
    }

    beginResetModel();
    m_instruments.clear();

    for (const InstrumentName& instrumentName : templatesByInstrumentName.keys()) {
        CombinedInstrument instrument;
        instrument.name = instrumentName;
        instrument.templates = templatesByInstrumentName[instrumentName];
        instrument.activeTemplateIndex = 0;

        m_instruments << instrument;
    }

    sortInstruments(m_instruments);

    endResetModel();
}

void InstrumentListModel::sortInstruments(QList<CombinedInstrument>& instruments) const
{
    TRACEFUNC;

    std::sort(instruments.begin(), instruments.end(), [this](const CombinedInstrument& instrument1, const CombinedInstrument& instrument2) {
        QString instrumentName1 = instrument1.name.toLower();
        QString instrumentName2 = instrument2.name.toLower();
        QString searchText = m_searchText.toLower();

        int searchTextPosition1 = instrumentName1.indexOf(searchText);
        int searchTextPosition2 = instrumentName2.indexOf(searchText);

        if (searchTextPosition1 == searchTextPosition2) {
            return instrumentName1 < instrumentName2;
        }

        return searchTextPosition1 < searchTextPosition2;
    });
}

void InstrumentListModel::setCurrentGenreIndex(int index)
{
    InstrumentGenreList genres = availableGenres();

    if (index >= 0 && index < genres.size()) {
        setCurrentGenre(genres[index]->id);
    }
}

void InstrumentListModel::setCurrentGroupIndex(int index)
{
    InstrumentGroupList groups = availableGroups();

    if (index >= 0 && index < groups.size()) {
        setCurrentGroup(groups[index]->id);
    }
}

void InstrumentListModel::selectInstrument(int instrumentIndex)
{
    if (!isInstrumentIndexValid(instrumentIndex)) {
        return;
    }

    QModelIndex modelIndex = index(instrumentIndex);
    m_selection->select(modelIndex);

    emit dataChanged(index(0), index(rowCount() - 1), { RoleIsSelected });
}

void InstrumentListModel::setActiveTrait(int instrumentIndex, int traitIndex)
{
    if (!isInstrumentIndexValid(instrumentIndex)) {
        return;
    }

    CombinedInstrument& instrument = m_instruments[instrumentIndex];
    instrument.activeTemplateIndex = traitIndex;

    selectInstrument(instrumentIndex);
}

QVariantList InstrumentListModel::selectedInstruments() const
{
    QVariantList result;

    for (int row : m_selection->selectedRows()) {
        const CombinedInstrument& instrument = m_instruments[row];
        int templateIndex = instrument.activeTemplateIndex;

        if (templateIndex < 0 || templateIndex >= instrument.templates.size()) {
            continue;
        }

        result << QVariant::fromValue(*instrument.templates[templateIndex]);
    }

    return result;
}

InstrumentGenreList InstrumentListModel::availableGenres() const
{
    TRACEFUNC;

    static InstrumentGenre allInstrumentsGenre;
    allInstrumentsGenre.id = ALL_INSTRUMENTS_GENRE_ID;
    allInstrumentsGenre.name = qtrc("instruments", "All instruments");

    InstrumentGenreList result;
    result << &allInstrumentsGenre;

    for (const InstrumentGenre* genre: m_instrumentsMeta.genres) {
        if (genre->id == COMMON_GENRE_ID) {
            result.prepend(genre);
        } else {
            result << genre;
        }
    }

    return result;
}

InstrumentGroupList InstrumentListModel::availableGroups() const
{
    TRACEFUNC;

    auto isGroupAccepted = [this](const InstrumentGroup* group) {
        for (const InstrumentTemplate* templ : group->instrumentTemplates) {
            constexpr bool compareWithSelectedGroup = false;

            if (isInstrumentAccepted(*templ, compareWithSelectedGroup)) {
                return true;
            }
        }

        return false;
    };

    InstrumentGroupList result;

    for (const InstrumentGroup* group : m_instrumentsMeta.groups) {
        if (isGroupAccepted(group)) {
            result << group;
        }
    }

    return result;
}

void InstrumentListModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    updateGenreStateBySearch();
}

int InstrumentListModel::currentGenreIndex() const
{
    InstrumentGenreList genres = availableGenres();

    for (int i = 0; i < genres.size(); ++i) {
        if (genres[i]->id == m_currentGenreId) {
            return i;
        }
    }

    return -1;
}

int InstrumentListModel::currentGroupIndex() const
{
    InstrumentGroupList groups = availableGroups();

    for (int i = 0; i < groups.size(); ++i) {
        if (groups[i]->id == m_currentGroupId) {
            return i;
        }
    }

    return -1;
}

bool InstrumentListModel::hasSelection() const
{
    return m_selection->hasSelection();
}

bool InstrumentListModel::isSearching() const
{
    return !m_searchText.isEmpty();
}

void InstrumentListModel::updateGenreStateBySearch()
{
    TRACEFUNC;

    bool genreSaved = !m_savedGenreId.isEmpty();

    if (isSearching() && !genreSaved) {
        m_savedGenreId = m_currentGenreId;
        setCurrentGenre(ALL_INSTRUMENTS_GENRE_ID);
        setCurrentGroup(NONE_GROUP_ID);
    } else if (!isSearching() && genreSaved) {
        setCurrentGenre(m_savedGenreId);
        m_savedGenreId.clear();
    } else {
        loadInstruments();
    }
}

bool InstrumentListModel::isInstrumentAccepted(const InstrumentTemplate& instrument, bool compareWithSelectedGroup) const
{
    if (isSearching()) {
        return instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
    }

    if (instrument.groupId != m_currentGroupId && compareWithSelectedGroup) {
        return false;
    }

    if (m_currentGenreId == ALL_INSTRUMENTS_GENRE_ID) {
        return true;
    }

    if (instrument.containsGenre(m_currentGenreId)) {
        return true;
    }

    return false;
}

bool InstrumentListModel::isInstrumentIndexValid(int index) const
{
    return index >= 0 && index < m_instruments.size();
}

void InstrumentListModel::setCurrentGenre(const QString& genreId)
{
    if (m_currentGenreId == genreId) {
        return;
    }

    m_currentGenreId = genreId;
    loadInstruments();

    emit currentGenreChanged();
    emit groupsChanged();
    emit currentGroupChanged();
}

void InstrumentListModel::setCurrentGroup(const QString& groupId)
{
    if (m_currentGroupId == groupId) {
        return;
    }

    m_currentGroupId = groupId;
    loadInstruments();

    emit currentGroupChanged();
}
