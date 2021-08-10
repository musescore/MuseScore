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

#include "engraving/libmscore/select.h"

#include "translation.h"

using namespace mu::notation;

SelectionFilterModel::SelectionFilterModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void SelectionFilterModel::load()
{
    beginResetModel();

    m_types.clear();
    m_types << SelectionFilterType::ALL;

    for (int i = 0; i < Ms::NUMBER_OF_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<SelectionFilterType>(1 << i);
    }

    updateIsAllSelected();
    endResetModel();

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        emit enabledChanged();

        if (currentNotation()) {
            updateIsAllSelected();
            emit dataChanged(index(1), index(rowCount() - 1), { IsSelectedRole });
        }
    });
}

void SelectionFilterModel::updateIsAllSelected()
{
    if (!currentNotationInteraction()) {
        return;
    }

    m_isNoneSelected = true;
    m_isAllSelected = true;

    // Skip "All" type
    for (auto type : m_types.mid(1)) {
        const bool selected = currentNotationInteraction()->isSelectionTypeFiltered(type);
        if (selected) {
            m_isNoneSelected = false;
        } else {
            m_isAllSelected = false;
        }

        if (!m_isAllSelected && !m_isNoneSelected) {
            break;
        }
    }

    emit dataChanged(index(0), index(0), { IsSelectedRole, IsIndeterminateRole });
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
        if (type == SelectionFilterType::ALL) {
            return m_isAllSelected;
        }

        return currentNotationInteraction() ? currentNotationInteraction()->isSelectionTypeFiltered(type) : false;

    case IsIndeterminateRole:
        if (type == SelectionFilterType::ALL) {
            return !m_isAllSelected && !m_isNoneSelected;
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

    if (type == SelectionFilterType::ALL) {
        currentNotationInteraction()->setAllSelectionTypesFiltered(filtered);
        m_isAllSelected = filtered;
        m_isNoneSelected = !filtered;
        emit dataChanged(this->index(0), this->index(rowCount() - 1), { IsSelectedRole, IsIndeterminateRole });
    } else {
        currentNotationInteraction()->setSelectionTypeFiltered(type, filtered);
        emit dataChanged(index, index, { IsSelectedRole, IsIndeterminateRole });
        updateIsAllSelected();
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
    return globalContext()->currentNotation() != nullptr;
}

INotationPtr SelectionFilterModel::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr SelectionFilterModel::currentNotationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

QString SelectionFilterModel::titleForType(SelectionFilterType type) const
{
    switch (type) {
    case SelectionFilterType::ALL:
        return qtrc("notation", "All");
    case SelectionFilterType::FIRST_VOICE:
        return qtrc("notation", "Voice 1");
    case SelectionFilterType::SECOND_VOICE:
        return qtrc("notation", "Voice 2");
    case SelectionFilterType::THIRD_VOICE:
        return qtrc("notation", "Voice 3");
    case SelectionFilterType::FOURTH_VOICE:
        return qtrc("notation", "Voice 4");
    case SelectionFilterType::DYNAMIC:
        return qtrc("notation", "Dynamics");
    case SelectionFilterType::HAIRPIN:
        return qtrc("notation", "Hairpins");
    case SelectionFilterType::FINGERING:
        return qtrc("notation", "Fingerings");
    case SelectionFilterType::LYRICS:
        return qtrc("notation", "Lyrics");
    case SelectionFilterType::CHORD_SYMBOL:
        return qtrc("notation", "Chord Symbols");
    case SelectionFilterType::OTHER_TEXT:
        return qtrc("notation", "Other Text");
    case SelectionFilterType::ARTICULATION:
        return qtrc("notation", "Articulations");
    case SelectionFilterType::ORNAMENT:
        return qtrc("notation", "Ornaments");
    case SelectionFilterType::SLUR:
        return qtrc("notation", "Slurs");
    case SelectionFilterType::FIGURED_BASS:
        return qtrc("notation", "Figured Bass");
    case SelectionFilterType::OTTAVA:
        return qtrc("notation", "Ottavas");
    case SelectionFilterType::PEDAL_LINE:
        return qtrc("notation", "Pedal Lines");
    case SelectionFilterType::OTHER_LINE:
        return qtrc("notation", "Other Lines");
    case SelectionFilterType::ARPEGGIO:
        return qtrc("notation", "Arpeggios");
    case SelectionFilterType::GLISSANDO:
        return qtrc("notation", "Glissandi");
    case SelectionFilterType::FRET_DIAGRAM:
        return qtrc("notation", "Fretboard Diagrams");
    case SelectionFilterType::BREATH:
        return qtrc("notation", "Breath Marks");
    case SelectionFilterType::TREMOLO:
        return qtrc("notation", "Tremolos");
    case SelectionFilterType::GRACE_NOTE:
        return qtrc("notation", "Grace Notes");
    case SelectionFilterType::NONE:
        break;
    }

    return {};
}
