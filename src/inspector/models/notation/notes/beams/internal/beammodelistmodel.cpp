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
#include "beammodelistmodel.h"

#include "translation.h"

using namespace mu::inspector;

BeamModeListModel::BeamModeListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(BeamModeRole, "beamModeRole");
    m_roleNames.insert(HintRole, "hintRole");

    m_beamTypesDataList = { { BeamTypes::Mode::MODE_AUTO, "auto" },
        { BeamTypes::Mode::MODE_BEGIN, qtrc("inspector", "Beam Start") },
        { BeamTypes::Mode::MODE_MID, qtrc("inspector", "Beam Middle") },
        { BeamTypes::Mode::MODE_NONE, qtrc("inspector", "No Beam") },
        { BeamTypes::Mode::MODE_BEGIN32, qtrc("inspector", "Beam 16th Sub") },
        { BeamTypes::Mode::MODE_BEGIN64, qtrc("inspector", "Beam 32nd Sub") } };
}

int BeamModeListModel::rowCount(const QModelIndex&) const
{
    return m_beamTypesDataList.count();
}

QVariant BeamModeListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_beamTypesDataList.isEmpty()) {
        return QVariant();
    }

    BeamTypesData beamTypeData = m_beamTypesDataList.at(index.row());

    switch (role) {
    case BeamModeRole: return static_cast<int>(beamTypeData.mode);
    case HintRole: return beamTypeData.hint;
    default: return QVariant();
    }
}

QHash<int, QByteArray> BeamModeListModel::roleNames() const
{
    return m_roleNames;
}

void BeamModeListModel::setSelectedBeamMode(const BeamTypes::Mode beamMode)
{
    m_selectedTypeIndex = indexOfBeamMode(beamMode);
    emit selectedTypeIndexChanged(m_selectedTypeIndex);
}

int BeamModeListModel::selectedTypeIndex() const
{
    return m_selectedTypeIndex;
}

void BeamModeListModel::setSelectedTypeIndex(int selectedTypeIndex)
{
    if (m_selectedTypeIndex == selectedTypeIndex) {
        return;
    }

    m_selectedTypeIndex = selectedTypeIndex;
    emit selectedTypeIndexChanged(m_selectedTypeIndex);

    emit beamModeSelected(m_beamTypesDataList.at(selectedTypeIndex).mode);
}

int BeamModeListModel::indexOfBeamMode(const BeamTypes::Mode beamMode) const
{
    for (int i = 0; i < m_beamTypesDataList.count(); ++i) {
        if (m_beamTypesDataList.at(i).mode == beamMode) {
            return i;
        }
    }

    return -1;
}
