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

#include "instrumentsonscoremodel.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::uicomponents;
using namespace mu::async;
using namespace mu::notation;

static const QString ID_KEY("id");
static const QString IS_SOLOIST_KEY("isSoloist");
static const QString IS_EXISTING_PART_KEY("isExistingPart");
static const QString INSTRUMENT_TEMPLATE_KEY("instrumentTemplate");
static const QString INSTRUMENTS_KEY("instruments");
static const QString SCORE_ORDER_KEY("scoreOrder");

InstrumentsOnScoreModel::InstrumentsOnScoreModel(QObject* parent)
    : QAbstractListModel(parent), m_selection(new ItemMultiSelectionModel(this))
{
    connect(m_selection, &ItemMultiSelectionModel::selectionChanged, this, &InstrumentsOnScoreModel::selectionChanged);

    //! TODO: need to implement the multi selection
    m_selection->setSingleItemSelectionMode(true);
}

QVariant InstrumentsOnScoreModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const InstrumentOnScore& instrument = m_instruments[index.row()];

    switch (role) {
    case RoleName: return instrument.name;
    case RoleIsSoloist: return instrument.isSoloist;
    case RoleIsSelected: return m_selection->isSelected(index);
    }

    return QVariant();
}

int InstrumentsOnScoreModel::rowCount(const QModelIndex&) const
{
    return m_instruments.size();
}

QHash<int, QByteArray> InstrumentsOnScoreModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleName, "name" },
        { RoleIsSelected, "isSelected" },
        { RoleIsSoloist, "isSoloist" }
    };

    return roles;
}

QStringList InstrumentsOnScoreModel::orders() const
{
    QStringList result;

    for (const ScoreOrder* order: m_scoreOrders) {
        result << order->name;
    }

    return result;
}

int InstrumentsOnScoreModel::currentOrderIndex() const
{
    return m_currentOrderIndex;
}

bool InstrumentsOnScoreModel::isMovingUpAvailable() const
{
    QList<int> rows = m_selection->selectedRows();
    return !rows.isEmpty() && rows.first() > 0;
}

bool InstrumentsOnScoreModel::isMovingDownAvailable() const
{
    QList<int> rows = m_selection->selectedRows();
    return !rows.isEmpty() && rows.last() < m_instruments.size() - 1;
}

bool InstrumentsOnScoreModel::isRemovingAvailable() const
{
    return m_selection->hasSelection();
}

void InstrumentsOnScoreModel::load()
{
    TRACEFUNC;

    INotationPtr notation = context()->currentNotation();
    auto parts = notation ? notation->parts()->partList() : NotifyList<const Part*>();

    beginResetModel();

    for (const Part* part : parts) {
        InstrumentOnScore instrument;
        instrument.id = part->id();
        instrument.isExistingPart = true;
        instrument.name = part->partName();
        instrument.isSoloist = part->soloist();
        instrument.familyId = part->familyId();

        m_instruments << instrument;
    }

    endResetModel();
    emit countChanged();

    loadInstrumentsMeta();
}

void InstrumentsOnScoreModel::loadInstrumentsMeta()
{
    TRACEFUNC;

    RetValCh<InstrumentsMeta> meta = repository()->instrumentsMeta();
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
    }

    m_scoreOrders = meta.val.scoreOrders;
    m_instrumentTemplates = meta.val.instrumentTemplates;

    INotationPtr notation = context()->currentNotation();
    QString currentOrderId = notation ? notation->scoreOrder().id : QString();

    for (int i = 0; i < m_scoreOrders.size(); ++i) {
        if (m_scoreOrders[i]->id == currentOrderId) {
            m_currentOrderIndex = i;
            break;
        }
    }

    emit ordersChanged();
    emit currentOrderChanged();
}

void InstrumentsOnScoreModel::addInstruments(const QVariantList& instruments)
{
    TRACEFUNC;

    for (const QVariant& obj : instruments) {
        if (obj.isNull()) {
            continue;
        }

        InstrumentTemplate templ = obj.value<InstrumentTemplate>();

        InstrumentOnScore instrument;
        instrument.id = templ.id;
        instrument.name = formatInstrumentTitle(templ.trackName, templ.trait);
        instrument.familyId = templ.familyId();
        instrument.instrumentTemplate = templ;

        m_instruments << instrument;
    }

    sortInstruments();

    emit countChanged();
}

void InstrumentsOnScoreModel::selectInstrument(int instrumentIndex)
{
    if (!isIndexValid(instrumentIndex)) {
        return;
    }

    QModelIndex modelIndex = index(instrumentIndex);
    m_selection->select(modelIndex);

    emit dataChanged(index(0), index(rowCount() - 1), { RoleIsSelected });
}

void InstrumentsOnScoreModel::toggleSoloist(int instrumentIndex)
{
    if (!isIndexValid(instrumentIndex)) {
        return;
    }

    InstrumentOnScore& instrument = m_instruments[instrumentIndex];
    instrument.isSoloist = !instrument.isSoloist;

    sortInstruments();
}

void InstrumentsOnScoreModel::removeSelection()
{
    TRACEFUNC;

    QList<int> selectedRows = m_selection->selectedRows();
    if (selectedRows.isEmpty()) {
        return;
    }

    QList<InstrumentOnScore> instrumentsToRemove;
    for (int row : selectedRows) {
        instrumentsToRemove << m_instruments[row];
    }

    beginResetModel();

    for (const InstrumentOnScore& instrument : instrumentsToRemove) {
        m_instruments.removeOne(instrument);
    }

    endResetModel();

    emit countChanged();
}

void InstrumentsOnScoreModel::moveSelectionUp()
{

}

void InstrumentsOnScoreModel::moveSelectionDown()
{

}

QVariantMap InstrumentsOnScoreModel::scoreContent() const
{
    TRACEFUNC;

    QVariantList instruments;

    for (const InstrumentOnScore& instrument : m_instruments) {
        QVariantMap obj;
        obj[ID_KEY] = instrument.id;
        obj[IS_SOLOIST_KEY] = instrument.isSoloist;
        obj[IS_EXISTING_PART_KEY] = instrument.isExistingPart;
        obj[INSTRUMENT_TEMPLATE_KEY] = QVariant::fromValue(instrument.instrumentTemplate);

        instruments << obj;
    }

    QVariantMap result;
    result[SCORE_ORDER_KEY] = QVariant::fromValue(*m_scoreOrders[m_currentOrderIndex]);
    result[INSTRUMENTS_KEY] = instruments;

    return result;
}

void InstrumentsOnScoreModel::setCurrentOrderIndex(int index)
{
    if (m_currentOrderIndex == index) {
        return;
    }

    m_currentOrderIndex = index;
    sortInstruments();

    emit currentOrderChanged();
}

void InstrumentsOnScoreModel::sortInstruments()
{
    TRACEFUNC;

    beginResetModel();

    m_selection->clear();

    std::sort(m_instruments.begin(), m_instruments.end(),
              [this](const InstrumentOnScore& instrument1, const InstrumentOnScore& instrument2) {

        int index1 = sortInstrumentsIndex(instrument1);
        int index2 = sortInstrumentsIndex(instrument2);

        return index1 < index2;
    });

    endResetModel();
}

int InstrumentsOnScoreModel::sortInstrumentsIndex(const InstrumentOnScore& instrument) const
{
    static const QString SoloistsGroup("<soloists>");
    static const QString UnsortedGroup("<unsorted>");

    enum class Priority {
        Undefined,
        Unsorted,
        UnsortedGroup,
        Family,
        Soloist
    };

    auto calculateIndex = [this, instrument](int index) {
        return index * m_instrumentTemplates.size() + instrument.instrumentTemplate.sequenceOrder;
    };

    if (m_currentOrderIndex < 0 || m_currentOrderIndex >= m_scoreOrders.size()) {
        return -1;
    }

    const ScoreOrder& order = *m_scoreOrders[m_currentOrderIndex];
    QString family = order.instrumentMap.contains(instrument.id) ? order.instrumentMap[instrument.id].id : instrument.familyId;
    int index = order.groups.size();
    Priority priority = Priority::Undefined;

    for (int i = 0; i < order.groups.size(); ++i) {
        const ScoreOrderGroup& sg = order.groups[i];

        if ((sg.family == SoloistsGroup) && instrument.isSoloist) {
            return calculateIndex(i);
        } else if ((priority < Priority::Family) && (sg.family == family)) {
            index = i;
            priority = Priority::Family;
        } else if ((priority < Priority::UnsortedGroup) && (sg.family == UnsortedGroup) && (sg.unsorted == instrument.instrumentTemplate.groupId)) {
            index = i;
            priority = Priority::UnsortedGroup;
        } else if ((priority < Priority::Unsorted) && (sg.family == UnsortedGroup)) {
            index = i;
            priority = Priority::Unsorted;
        }
    }

    return calculateIndex(index);
}

bool InstrumentsOnScoreModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_instruments.size();
}
