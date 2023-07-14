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
static const QString FIRST_GROUP_ID("FIRST_GROUP_ID");
static const QString INSTRUMENT_TEMPLATE_KEY("instrumentTemplate");

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QAbstractListModel(parent), m_selection(new ItemMultiSelectionModel(this))
{
    connect(m_selection, &ItemMultiSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
        QModelIndexList changedIndexes;
        changedIndexes << selected.indexes();
        changedIndexes << deselected.indexes();

        for (const QModelIndex& index : changedIndexes) {
            emit dataChanged(index, index, { RoleIsSelected });
        }

        emit selectionChanged();
    });
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
    case RoleDescription:
        return instrument.templates[instrument.currentTemplateIndex]->description.toQString();
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
    case RoleDescription:
    case RoleTraits:
    case RoleIsSelected:
        break;
    case RoleCurrentTraitIndex:
        CombinedInstrument& instrument = m_instruments[row];
        instrument.currentTemplateIndex = value.toInt();
        selectInstrument(index.row());
        emit dataChanged(index, index, { RoleCurrentTraitIndex });
        emit selectionChanged();
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
        { RoleDescription, "description" },
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

    loadGenres();
    loadGroups();

    if (currentInstrumentId.isEmpty()) {
        init(COMMON_GENRE_ID, FIRST_GROUP_ID);
    } else {
        init(COMMON_GENRE_ID, resolveInstrumentGroupId(currentInstrumentId));
    }
}

QStringList InstrumentListModel::genres() const
{
    QStringList result;

    for (const InstrumentGenre* genre: m_genres) {
        result << genre->name;
    }

    return result;
}

QStringList InstrumentListModel::groups() const
{
    QStringList result;

    for (const InstrumentGroup* group: m_groups) {
        result << group->name;
    }

    return result;
}

void InstrumentListModel::init(const QString& genreId, const QString& groupId)
{
    TRACEFUNC;

    setCurrentGenre(genreId);

    QString newGroupId = groupId;
    if (newGroupId == FIRST_GROUP_ID) {
        newGroupId = !m_groups.empty() ? m_groups.first()->id.toQString() : NONE_GROUP_ID;
    }

    setCurrentGroup(newGroupId);

    m_instrumentsLoadingAllowed = true;
    loadInstruments();
}

QString InstrumentListModel::resolveInstrumentGroupId(const String& instrumentId) const
{
    for (const InstrumentTemplate* templ : repository()->instrumentTemplates()) {
        if (templ->id == instrumentId) {
            return templ->groupId;
        }
    }

    return NONE_GROUP_ID;
}

void InstrumentListModel::loadGenres()
{
    TRACEFUNC;

    static InstrumentGenre allInstrumentsGenre;
    allInstrumentsGenre.id = ALL_INSTRUMENTS_GENRE_ID;
    allInstrumentsGenre.name = qtrc("instruments", "All instruments");

    m_genres << &allInstrumentsGenre;

    for (const InstrumentGenre* genre: repository()->genres()) {
        if (genre->id == COMMON_GENRE_ID) {
            m_genres.prepend(genre);
        } else {
            m_genres << genre;
        }
    }

    emit genresChanged();
}

void InstrumentListModel::loadGroups()
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

    InstrumentGroupList acceptedGroups;

    for (const InstrumentGroup* group : repository()->groups()) {
        if (isGroupAccepted(group)) {
            acceptedGroups << group;
        }
    }

    if (m_groups == acceptedGroups) {
        return;
    }

    m_groups = acceptedGroups;
    emit groupsChanged();

    bool currentGroupInNewGenre = false;
    for (const InstrumentGroup* group : m_groups) {
        if (group->id == m_currentGroupId) {
            currentGroupInNewGenre = true;
            break;
        }
    }
    if (!currentGroupInNewGenre && !isSearching()) {
        setCurrentGroupIndex(0);
    }

    emit currentGroupIndexChanged();
}

void InstrumentListModel::loadInstruments()
{
    if (!m_instrumentsLoadingAllowed) {
        return;
    }

    TRACEFUNC;

    using InstrumentName = QString;
    QHash<InstrumentName, InstrumentTemplateList> templatesByInstrumentName;

    for (const InstrumentTemplate* templ: repository()->instrumentTemplates()) {
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

    QList<CombinedInstrument> instruments;

    for (const InstrumentName& instrumentName : templatesByInstrumentName.keys()) {
        const InstrumentTemplateList& templates = templatesByInstrumentName[instrumentName];
        CombinedInstrument instrument;

        if (templates.length() == 1) {
            // Only one trait option so let's display it in the instrument name.
            const InstrumentTemplate* templ = templates.at(0);
            instrument.name = formatInstrumentTitle(instrumentName, templ->trait);
        } else {
            // Multiple traits to choose from so don't add any to instrument name yet.
            instrument.name = instrumentName;
        }

        instrument.templates = templates;
        instrument.currentTemplateIndex = 0;

        instruments << instrument;
    }

    if (m_instruments == instruments) {
        return;
    }

    m_selection->clear();

    beginResetModel();

    m_instruments = std::move(instruments);
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
            int ti1 = instrument1.currentTemplateIndex;
            int ti2 = instrument2.currentTemplateIndex;
            if (ti1 >= 0 && ti1 < instrument1.templates.size() && ti2 >= 0 && ti2 < instrument2.templates.size()) {
                int instrumentIndex1 = instrument1.templates[ti1]->sequenceOrder;
                int instrumentIndex2 = instrument2.templates[ti2]->sequenceOrder;
                return instrumentIndex1 < instrumentIndex2;
            } else {
                return instrumentName1 < instrumentName2;
            }
        }

        return searchTextPosition1 < searchTextPosition2;
    });
}

void InstrumentListModel::setCurrentGenreIndex(int index)
{
    if (index >= 0 && index < m_genres.size()) {
        setCurrentGenre(m_genres[index]->id);
    }
}

void InstrumentListModel::setCurrentGroupIndex(int index)
{
    if (index >= 0 && index < m_groups.size()) {
        setCurrentGroup(m_groups[index]->id);
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
}

QStringList InstrumentListModel::selectedInstrumentIdList() const
{
    QSet<QString> result;

    for (int row : m_selection->selectedRows()) {
        const CombinedInstrument& instrument = m_instruments[row];
        int templateIndex = instrument.currentTemplateIndex;

        if (templateIndex < 0 || templateIndex >= instrument.templates.size()) {
            continue;
        }

        const InstrumentTemplate* templ = instrument.templates[templateIndex];
        if (templ->id.empty()) {
            continue;
        }

        result << templ->id.toQString();
    }

    return QStringList(result.begin(), result.end());
}

void InstrumentListModel::saveCurrentGroup()
{
    m_saveCurrentGroup = true;
}

void InstrumentListModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    updateStateBySearch();
}

int InstrumentListModel::currentGenreIndex() const
{
    for (int i = 0; i < m_genres.size(); ++i) {
        if (m_genres[i]->id == m_currentGenreId) {
            return i;
        }
    }

    return -1;
}

int InstrumentListModel::currentGroupIndex() const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i]->id == m_currentGroupId) {
            return i;
        }
    }

    return -1;
}

bool InstrumentListModel::hasSelection() const
{
    return m_selection->hasSelection();
}

QVariant InstrumentListModel::selectedInstrument() const
{
    QList<int> selectedRows = m_selection->selectedRows();
    if (selectedRows.length() != 1) {
        return QVariant();
    }

    const CombinedInstrument& instrument = m_instruments.at(selectedRows.at(0));
    const InstrumentTemplate* templ = instrument.templates[instrument.currentTemplateIndex];

    QVariantMap obj;
    obj["instrumentId"] = templ->id.toQString();
    obj["description"] = templ->description.toQString();

    return obj;
}

bool InstrumentListModel::isSearching() const
{
    return !m_searchText.isEmpty();
}

void InstrumentListModel::updateStateBySearch()
{
    TRACEFUNC;

    bool searching = isSearching();

    //! The current group may not be present in the saved genre,
    //! so let's keep ALL_INSTRUMENTS_GENRE_ID as the current genre
    if (m_saveCurrentGroup && !searching) {
        m_savedGenreId.clear();
    }

    bool genreSaved = !m_savedGenreId.isEmpty();
    bool needSaveGenre = !genreSaved && m_currentGenreId != ALL_INSTRUMENTS_GENRE_ID;

    if (searching && needSaveGenre) {
        m_savedGenreId = m_currentGenreId;
        setCurrentGenre(ALL_INSTRUMENTS_GENRE_ID);
    } else if (!searching && genreSaved) {
        setCurrentGenre(m_savedGenreId);
        m_savedGenreId.clear();
    } else {
        loadGroups();
        loadInstruments();
    }

    if (searching && m_savedGroupId.isEmpty()) {
        m_savedGroupId = m_currentGroupId;
        setCurrentGroup(NONE_GROUP_ID);
    } else if (!searching) {
        setCurrentGroup(m_savedGroupId);
        m_savedGroupId = "";
    }

    m_saveCurrentGroup = false;
}

bool InstrumentListModel::isInstrumentAccepted(const InstrumentTemplate& instrument, bool compareWithCurrentGroup) const
{
    if (isSearching()) {
        return instrument.trackName.toQString().contains(m_searchText, Qt::CaseInsensitive);
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
    emit currentGenreIndexChanged();

    loadGroups();
    loadInstruments();
}

void InstrumentListModel::setCurrentGroup(const QString& groupId)
{
    doSetCurrentGroup(groupId);
    loadInstruments();
}

void InstrumentListModel::doSetCurrentGroup(const QString& groupId)
{
    if (m_currentGroupId == groupId) {
        return;
    }

    m_currentGroupId = groupId;
    emit currentGroupIndexChanged();
}
