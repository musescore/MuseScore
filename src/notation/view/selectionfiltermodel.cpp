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
#include "selectionfiltermodel.h"

#include "engraving/dom/select.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;

SelectionFilterModel::SelectionFilterModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void SelectionFilterModel::load()
{
    TRACEFUNC;

    beginResetModel();

    m_types.clear();
    m_types << SelectionFilterType::ALL;

    for (size_t i = 0; i < mu::engraving::NUMBER_OF_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<SelectionFilterType>(1 << i);
    }

    endResetModel();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        emit enabledChanged();

        if (currentNotation()) {
            emit dataChanged(index(0), index(rowCount() - 1), { IsSelectedRole, IsIndeterminateRole });
        }
    });
}

QVariant SelectionFilterModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return {};
    }

    auto type = m_types[row];

    switch (role) {
    case TitleRole:
        return titleForType(type);

    case IsSelectedRole:
        return isFiltered(type);

    case IsIndeterminateRole:
        if (type == SelectionFilterType::ALL) {
            return !isFiltered(SelectionFilterType::ALL) && !isFiltered(SelectionFilterType::NONE);
        }

        return false;
    }

    return {};
}

bool SelectionFilterModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
    if (role != IsSelectedRole) {
        return false;
    }

    if (!currentNotationInteraction()) {
        return false;
    }

    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return false;
    }

    auto type = m_types[row];
    const bool filtered = data.toBool();

    setFiltered(type, filtered);
    if (type == SelectionFilterType::ALL) {
        emit dataChanged(this->index(0), this->index(rowCount() - 1), { IsSelectedRole, IsIndeterminateRole });
    } else {
        emit dataChanged(this->index(0), this->index(0), { IsSelectedRole, IsIndeterminateRole });
        emit dataChanged(index, index, { IsSelectedRole });
    }

    return true;
}

int SelectionFilterModel::rowCount(const QModelIndex&) const
{
    return m_types.size();
}

QHash<int, QByteArray> SelectionFilterModel::roleNames() const
{
    return {
        { TitleRole, "title" },
        { IsSelectedRole, "isSelected" },
        { IsIndeterminateRole, "isIndeterminate" }
    };
}

bool SelectionFilterModel::enabled() const
{
    return currentNotation() != nullptr;
}

INotationPtr SelectionFilterModel::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr SelectionFilterModel::currentNotationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

bool SelectionFilterModel::isFiltered(SelectionFilterType type) const
{
    return currentNotationInteraction() ? currentNotationInteraction()->isSelectionTypeFiltered(type) : false;
}

void SelectionFilterModel::setFiltered(SelectionFilterType type, bool filtered)
{
    if (currentNotationInteraction()) {
        currentNotationInteraction()->setSelectionTypeFiltered(type, filtered);
    }
}

QString SelectionFilterModel::titleForType(SelectionFilterType type) const
{
    switch (type) {
    case SelectionFilterType::ALL:
        return qtrc("notation", "All");
    case SelectionFilterType::FIRST_VOICE:
        return qtrc("notation", "Voice %1").arg(1);
    case SelectionFilterType::SECOND_VOICE:
        return qtrc("notation", "Voice %1").arg(2);
    case SelectionFilterType::THIRD_VOICE:
        return qtrc("notation", "Voice %1").arg(3);
    case SelectionFilterType::FOURTH_VOICE:
        return qtrc("notation", "Voice %1").arg(4);
    case SelectionFilterType::DYNAMIC:
        return qtrc("notation", "Dynamics");
    case SelectionFilterType::HAIRPIN:
        return qtrc("notation", "Hairpins");
    case SelectionFilterType::FINGERING:
        return qtrc("notation", "Fingerings");
    case SelectionFilterType::LYRICS:
        return qtrc("notation", "Lyrics");
    case SelectionFilterType::CHORD_SYMBOL:
        return qtrc("notation", "Chord symbols");
    case SelectionFilterType::OTHER_TEXT:
        return qtrc("notation", "Other text");
    case SelectionFilterType::ARTICULATION:
        return qtrc("notation", "Articulations");
    case SelectionFilterType::ORNAMENT:
        return qtrc("notation", "Ornaments");
    case SelectionFilterType::SLUR:
        return qtrc("notation", "Slurs");
    case SelectionFilterType::FIGURED_BASS:
        return qtrc("notation", "Figured bass");
    case SelectionFilterType::OTTAVA:
        return qtrc("notation", "Ottavas");
    case SelectionFilterType::PEDAL_LINE:
        return qtrc("notation", "Pedal lines");
    case SelectionFilterType::OTHER_LINE:
        return qtrc("notation", "Other lines");
    case SelectionFilterType::ARPEGGIO:
        return qtrc("notation", "Arpeggios");
    case SelectionFilterType::GLISSANDO:
        return qtrc("notation", "Glissandi");
    case SelectionFilterType::FRET_DIAGRAM:
        return qtrc("notation", "Fretboard diagrams");
    case SelectionFilterType::BREATH:
        return qtrc("notation", "Breath marks");
    case SelectionFilterType::TREMOLO:
        return qtrc("notation", "Tremolos");
    case SelectionFilterType::GRACE_NOTE:
        return qtrc("notation", "Grace notes");
    case SelectionFilterType::NONE:
        break;
    }

    return {};
}
