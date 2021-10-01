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

#include "engraving/libmscore/note.h"
#include "engraving/libmscore/scorefont.h"

using namespace mu::inspector;
using namespace Ms;

NoteheadGroupsModel::NoteheadGroupsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> NoteheadGroupsModel::roleNames() const
{
    return {
        { HeadGroupRole, "headGroup" },
        { HintRole, "headHint" },
        { IconCodeRole, "iconCode" }
    };
}

int NoteheadGroupsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(NoteHead::Group::HEAD_DO_WALKER);
}

QVariant NoteheadGroupsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    int row = index.row();
    auto group = static_cast<NoteHead::Group>(row);

    switch (role) {
    case HeadGroupRole:
        return row;
    case HintRole:
        return NoteHead::group2userName(group);
    case IconCodeRole: {
        auto type = (group == NoteHead::Group::HEAD_BREVIS_ALT) ? NoteHead::Type::HEAD_BREVIS : NoteHead::Type::HEAD_QUARTER;
        return ScoreFont::fallbackFont()->symCode(Note::noteHead(0, group, type));
    }
    default: break;
    }

    return QVariant();
}
