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

#include "articulationpatternsscopeitem.h"

#include <iterator>

#include "internal/stringutils.h"

using namespace mu::mpe;

ArticulationPatternsScopeItem::ArticulationPatternsScopeItem(QObject* parent, const ArticulationType type,
                                                             const ArticulationPatternsScope& patterns)
    : QAbstractListModel(parent)
{
    setTitle(articulationTypeToString(type));

    for (auto it = patterns.begin(); it != patterns.end(); ++it) {
        auto next = std::next(it, 1);

        float scopePositionFrom = it->first;
        float scopePositionTo = 1.f;

        if (next == patterns.end()) {
            scopePositionTo = next->first;
        }

        m_items << new ArticulationPatternItem(this, it->second, scopePositionFrom, scopePositionTo);
    }
}

const QString& ArticulationPatternsScopeItem::title() const
{
    return m_title;
}

void ArticulationPatternsScopeItem::setTitle(const QString& newTitle)
{
    if (m_title == newTitle) {
        return;
    }
    m_title = newTitle;
    emit titleChanged();
}

ArticulationPatternItem* ArticulationPatternsScopeItem::currentPattern() const
{
    return m_currentPattern;
}

void ArticulationPatternsScopeItem::setCurrentPattern(ArticulationPatternItem* newCurrentPattern)
{
    if (m_currentPattern == newCurrentPattern) {
        return;
    }
    m_currentPattern = newCurrentPattern;
    emit currentPatternChanged();
}

int ArticulationPatternsScopeItem::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant ArticulationPatternsScopeItem::data(const QModelIndex& index, int /*role*/) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    return QVariant::fromValue(m_items.at(index.row()));
}

QHash<int, QByteArray> ArticulationPatternsScopeItem::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        { PatternItem, "patternItem" }
    };

    return roles;
}
