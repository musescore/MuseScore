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
static const QString INSTRUMENT_TEMPLATE_KEY("instrumentTemplate");

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QAbstractListModel(parent), m_selection(new ItemMultiSelectionModel(this))
{
    connect(m_selection, &ItemMultiSelectionModel::selectionChanged, this, &InstrumentListModel::selectionChanged);
}

QVariant InstrumentListModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();

    if (!isInstrumentIndexValid(row)) {
        return false;
    }

    const CombinedInstrument& instrument = m_instruments[row];

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
    case RoleCurrentTraitIndex:
        return instrument.currentTemplateIndex;
    case RoleIsSelected:
        return m_selection->isSelected(index);
    }

    return QVariant();
}

bool InstrumentListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();

    if (!isInstrumentIndexValid(row)) {
        return false;
    }

    switch (role) {
    case RoleName:
    case RoleTraits:
    case RoleIsSelected:
        break;
    case RoleCurrentTraitIndex:
        CombinedInstrument& instrument = m_instruments[row];
        instrument.currentTemplateIndex = value.toInt();
        selectInstrument(index.row());
        return true;
    }

    return false;
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
        { RoleTraits, "traits" },
        { RoleCurrentTraitIndex, "currentTraitIndex" }
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

    setInstrumentsMeta(instrumentsMeta.val);

    if (currentInstrumentId.isEmpty()) {
        init(COMMON_GENRE_ID, NONE_GROUP_ID);
    } else {
        init(ALL_INSTRUMENTS_GENRE_ID, resolveInstrumentGroupId(currentInstrumentId));
        focusOnInstrument(currentInstrumentId);
    }
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

void InstrumentListModel::setInstrumentsMeta(const notation::InstrumentsMeta& meta)
{
    m_instrumentsMeta = meta;

    emit genresChanged();
    emit groupsChanged();
}

void InstrumentListModel::init(const QString& genreId, const QString& groupId)
{
    setCurrentGenre(genreId);
    setCurrentGroup(groupId);

    m_instrumentsLoadingAllowed = true;
    loadInstruments();
}

QString InstrumentListModel::resolveInstrumentGroupId(const QString& instrumentId) const
{
    for (const InstrumentTemplate* templ : m_instrumentsMeta.instrumentTemplates) {
        if (templ->id == instrumentId) {
            return templ->groupId;
        }
    }

    return NONE_GROUP_ID;
}

void InstrumentListModel::focusOnInstrument(const QString& instrumentId)
{
    TRACEFUNC;

    for (int i = 0; i < m_instruments.size(); ++i) {
        for (const InstrumentTemplate* templ : m_instruments[i].templates) {
            if (templ->id == instrumentId) {
                selectInstrument(i);
                emit focusRequested(currentGroupIndex(), i);
                return;
            }
        }
    }
}

void InstrumentListModel::loadInstruments()
{
    if (!m_instrumentsLoadingAllowed) {
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
        instrument.currentTemplateIndex = 0;

        m_instruments << instrument;
    }

    sortInstruments(m_instruments);

    endResetModel();
}

void InstrumentListModel::sortInstruments(Instruments& instruments) const
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

    if (isSearching()) {
        const CombinedInstrument& instrument = m_instruments[instrumentIndex];

        if (m_selection->selection().size() == 1 && !instrument.templates.isEmpty()) {
            doSetCurrentGroup(instrument.templates.first()->groupId);
        } else {
            doSetCurrentGroup(NONE_GROUP_ID);
        }
    }

    emit dataChanged(index(0), index(rowCount() - 1), { RoleIsSelected });
}

QVariantList InstrumentListModel::selectedInstruments() const
{
    QVariantList result;

    for (int row : m_selection->selectedRows()) {
        const CombinedInstrument& instrument = m_instruments[row];
        int templateIndex = instrument.currentTemplateIndex;

        if (templateIndex < 0 || templateIndex >= instrument.templates.size()) {
            continue;
        }

        QVariantMap obj;
        obj[INSTRUMENT_TEMPLATE_KEY] = QVariant::fromValue(*instrument.templates[templateIndex]);

        result << obj;
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
            constexpr bool compareWithCurrentGroup = false;

            if (isInstrumentAccepted(*templ, compareWithCurrentGroup)) {
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

bool InstrumentListModel::isInstrumentAccepted(const InstrumentTemplate& instrument, bool compareWithCurrentGroup) const
{
    if (isSearching()) {
        return instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
    }

    if (instrument.groupId != m_currentGroupId && compareWithCurrentGroup) {
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

    doSetCurrentGroup(groupId);
    loadInstruments();
}

void InstrumentListModel::doSetCurrentGroup(const QString& groupId)
{
    m_currentGroupId = groupId;

    emit currentGroupChanged();
}
