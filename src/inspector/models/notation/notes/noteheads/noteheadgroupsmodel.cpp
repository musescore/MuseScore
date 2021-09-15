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
#include "noteheadgroupsmodel.h"

using namespace mu::inspector;

NoteheadGroupsModel::NoteheadGroupsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void NoteheadGroupsModel::load()
{
    beginResetModel();

    if (!m_noteheadGroupDataList.isEmpty()) {
        return;
    }

    for (int i = 0; i < static_cast<int>(Ms::NoteHead::Group::HEAD_DO_WALKER); ++i) {
        HeadGroupData headGroupData;

        headGroupData.group = static_cast<Ms::NoteHead::Group>(i);
        headGroupData.hint = Ms::NoteHead::group2userName(headGroupData.group);

        m_noteheadGroupDataList << headGroupData;
    }

    endResetModel();
}

QHash<int, QByteArray> NoteheadGroupsModel::roleNames() const
{
    return {
        { HeadGroupRole, "headGroupRole" },
        { HintRole, "hintRole" }
    };
}

int NoteheadGroupsModel::rowCount(const QModelIndex&) const
{
    return m_noteheadGroupDataList.count();
}

QVariant NoteheadGroupsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_noteheadGroupDataList.isEmpty()) {
        return QVariant();
    }

    HeadGroupData headTypeData = m_noteheadGroupDataList.at(index.row());

    switch (role) {
    case HeadGroupRole: return static_cast<int>(headTypeData.group);
    case HintRole: return headTypeData.hint;
    default: return QVariant();
    }
}

void NoteheadGroupsModel::init(const Ms::NoteHead::Group noteHeadGroup)
{
    load();
    m_selectedHeadGroupIndex = indexOfHeadGroup(noteHeadGroup);
    emit selectedHeadGroupIndexChanged(m_selectedHeadGroupIndex);
}

int NoteheadGroupsModel::selectedHeadGroupIndex() const
{
    return m_selectedHeadGroupIndex;
}

void NoteheadGroupsModel::setSelectedHeadGroupIndex(int selectedHeadTypeIndex)
{
    if (m_selectedHeadGroupIndex == selectedHeadTypeIndex) {
        return;
    }

    m_selectedHeadGroupIndex = selectedHeadTypeIndex;
    emit selectedHeadGroupIndexChanged(m_selectedHeadGroupIndex);

    emit noteHeadGroupSelected(selectedHeadTypeIndex);
}

int NoteheadGroupsModel::indexOfHeadGroup(const Ms::NoteHead::Group group) const
{
    for (int i = 0; i < m_noteheadGroupDataList.count(); ++i) {
        if (m_noteheadGroupDataList.at(i).group == group) {
            return i;
        }
    }

    return -1;
}
