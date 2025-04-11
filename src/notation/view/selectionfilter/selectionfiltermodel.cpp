/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/selectionfilter.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;

SelectionFilterModel::SelectionFilterModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
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

    if (!currentNotationSelectionFilter()) {
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

INotationSelectionFilterPtr SelectionFilterModel::currentNotationSelectionFilter() const
{
    const INotationInteractionPtr interaction = currentNotation() ? currentNotation()->interaction() : nullptr;
    return interaction ? interaction->selectionFilter() : nullptr;
}

bool SelectionFilterModel::isFiltered(SelectionFilterType type) const
{
    return currentNotationSelectionFilter() ? currentNotationSelectionFilter()->isSelectionTypeFiltered(type) : false;
}

void SelectionFilterModel::setFiltered(SelectionFilterType type, bool filtered)
{
    if (currentNotationSelectionFilter()) {
        currentNotationSelectionFilter()->setSelectionTypeFiltered(type, filtered);
    }
}

QString SelectionFilterModel::titleForType(SelectionFilterType type) const
{
    switch (type) {
    case SelectionFilterType::ALL:
        return muse::qtrc("notation", "All");
    case SelectionFilterType::FIRST_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(1);
    case SelectionFilterType::SECOND_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(2);
    case SelectionFilterType::THIRD_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(3);
    case SelectionFilterType::FOURTH_VOICE:
        return muse::qtrc("notation", "Voice %1").arg(4);
    case SelectionFilterType::DYNAMIC:
        return muse::qtrc("notation", "Dynamics");
    case SelectionFilterType::HAIRPIN:
        return muse::qtrc("notation", "Hairpins");
    case SelectionFilterType::FINGERING:
        return muse::qtrc("notation", "Fingerings");
    case SelectionFilterType::LYRICS:
        return muse::qtrc("notation", "Lyrics");
    case SelectionFilterType::CHORD_SYMBOL:
        return muse::qtrc("notation", "Chord symbols");
    case SelectionFilterType::OTHER_TEXT:
        return muse::qtrc("notation", "Other text");
    case SelectionFilterType::ARTICULATION:
        return muse::qtrc("notation", "Articulations");
    case SelectionFilterType::ORNAMENT:
        return muse::qtrc("notation", "Ornaments");
    case SelectionFilterType::SLUR:
        return muse::qtrc("notation", "Slurs");
    case SelectionFilterType::FIGURED_BASS:
        return muse::qtrc("notation", "Figured bass");
    case SelectionFilterType::OTTAVA:
        return muse::qtrc("notation", "Ottavas");
    case SelectionFilterType::PEDAL_LINE:
        return muse::qtrc("notation", "Pedal lines");
    case SelectionFilterType::OTHER_LINE:
        return muse::qtrc("notation", "Other lines");
    case SelectionFilterType::ARPEGGIO:
        return muse::qtrc("notation", "Arpeggios");
    case SelectionFilterType::GLISSANDO:
        return muse::qtrc("notation", "Glissandos");
    case SelectionFilterType::FRET_DIAGRAM:
        return muse::qtrc("notation", "Fretboard diagrams");
    case SelectionFilterType::BREATH:
        return muse::qtrc("notation", "Breath marks");
    case SelectionFilterType::TREMOLO:
        return muse::qtrc("notation", "Tremolos");
    case SelectionFilterType::GRACE_NOTE:
        return muse::qtrc("notation", "Grace notes");
    case SelectionFilterType::NONE:
        break;
    }

    return {};
}
