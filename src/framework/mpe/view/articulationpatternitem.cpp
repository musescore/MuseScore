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

#include "articulationpatternitem.h"

#include <iterator>

#include "internal/articulationstringutils.h"

using namespace muse::mpe;

static const QString SINGLE_NOTE_MENU_ID = "0";
static const QString MULTI_NOTE_MENU_ID = "1";

static const QString SINGLE_NOTE_MENU_TITLE = "Single note";
static const QString MULTI_NOTE_MENU_TITLE = "Multi note";

ArticulationPatternItem::ArticulationPatternItem(QObject* parent, const ArticulationType type, const bool isSingleNoteType)
    : QAbstractListModel(parent), m_isSingleNoteType(isSingleNoteType)
{
    updateType(type);
}

const QString& ArticulationPatternItem::title() const
{
    return m_title;
}

const ArticulationType& ArticulationPatternItem::type() const
{
    return m_type;
}

ArticulationPattern ArticulationPatternItem::patternData() const
{
    ArticulationPattern result;

    for (const auto& item : m_items) {
        result.emplace(item->positionFrom(), item->patternSegmentData());
    }

    return result;
}

void ArticulationPatternItem::updateType(const ArticulationType type)
{
    m_type = type;

    if (type != ArticulationType::Undefined) {
        m_title = articulationTypeToString(type);
    } else {
        m_title = QString();
    }

    emit titleChanged();
}

ArticulationPatternSegment ArticulationPatternItem::buildBlankPatternSegment() const
{
    return ArticulationPatternSegment(ArrangementPattern(HUNDRED_PERCENT /*durationFactor*/, 0 /*timestampOffset*/),
                                      PitchPattern(EXPECTED_SIZE, TEN_PERCENT, 0),
                                      ExpressionPattern(EXPECTED_SIZE, TEN_PERCENT, 0));
}

ArticulationPatternSegmentItem* ArticulationPatternItem::currentPatternSegment() const
{
    return m_currentPattern;
}

void ArticulationPatternItem::setCurrentPatternSegment(ArticulationPatternSegmentItem* newCurrentPattern)
{
    if (m_currentPattern == newCurrentPattern) {
        return;
    }
    m_currentPattern = newCurrentPattern;
    emit currentPatternSegmentChanged();
}

void ArticulationPatternItem::load(const ArticulationPattern& pattern)
{
    beginResetModel();

    m_items.clear();
    qDeleteAll(m_items);

    for (auto it = pattern.begin(); it != pattern.end(); ++it) {
        auto next = std::next(it, 1);

        float scopePositionFrom = it->first;
        float scopePositionTo = HUNDRED_PERCENT;

        if (next != pattern.cend()) {
            scopePositionTo = next->first;
        }

        m_items << new ArticulationPatternSegmentItem(this, it->second, scopePositionFrom, scopePositionTo);
    }

    if (pattern.empty()) {
        m_items << new ArticulationPatternSegmentItem(this, buildBlankPatternSegment(), 0, HUNDRED_PERCENT);
    }

    endResetModel();

    setCurrentPatternSegment(m_items.first());
}

void ArticulationPatternItem::appendNewSegment()
{
    if (!m_currentPattern) {
        return;
    }

    float halvedDuration = (m_currentPattern->positionTo() - m_currentPattern->positionFrom()) / 2;

    m_currentPattern->setPositionTo(m_currentPattern->positionFrom() + halvedDuration);

    ArticulationPatternSegmentItem* newSegment = new ArticulationPatternSegmentItem(this,
                                                                                    buildBlankPatternSegment(),
                                                                                    m_currentPattern->positionTo(),
                                                                                    m_currentPattern->positionTo() + halvedDuration);

    int currentSegmentIndex = m_items.indexOf(m_currentPattern);

    beginInsertRows(QModelIndex(), currentSegmentIndex + 1, currentSegmentIndex + 1);
    m_items.insert(currentSegmentIndex + 1, newSegment);
    endInsertRows();

    setCurrentPatternSegment(newSegment);
}

void ArticulationPatternItem::removeCurrentSegment()
{
    if (!m_currentPattern) {
        return;
    }

    int currentSegmentIndex = m_items.indexOf(m_currentPattern);
    int currentSegmentDuration = m_currentPattern->positionFrom() - m_currentPattern->positionTo();

    beginRemoveRows(QModelIndex(), currentSegmentIndex, currentSegmentIndex);
    m_items.removeAt(currentSegmentIndex);
    delete m_currentPattern;
    endRemoveRows();

    int newCurrentSegmentIndex = currentSegmentIndex - 1;
    if (m_items.at(newCurrentSegmentIndex)) {
        setCurrentPatternSegment(m_items.at(newCurrentSegmentIndex));
        m_currentPattern->setPositionTo(m_currentPattern->positionFrom() + currentSegmentDuration);
    }
}

bool ArticulationPatternItem::isAbleToRemoveCurrentSegment()
{
    return rowCount() > 1;
}

int ArticulationPatternItem::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant ArticulationPatternItem::data(const QModelIndex& index, int /*role*/) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    return QVariant::fromValue(m_items.at(index.row()));
}

QHash<int, QByteArray> ArticulationPatternItem::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { PatternSegmentItem, "patternSegmentItem" }
    };

    return roles;
}

bool ArticulationPatternItem::isActive() const
{
    return m_isActive;
}

void ArticulationPatternItem::setIsActive(bool newIsChecked)
{
    if (m_isActive == newIsChecked) {
        return;
    }
    m_isActive = newIsChecked;
    emit isActiveChanged();
}

bool ArticulationPatternItem::isSingleNoteType() const
{
    return m_isSingleNoteType;
}

bool ArticulationPatternItem::isSelected() const
{
    return m_isSelected;
}

void ArticulationPatternItem::setIsSelected(bool newIsSelected)
{
    if (m_isSelected == newIsSelected) {
        return;
    }
    m_isSelected = newIsSelected;
    emit isSelectedChanged();
}
