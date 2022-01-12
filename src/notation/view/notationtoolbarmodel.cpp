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

#include "notationtoolbarmodel.h"

#include "log.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::ui;

NotationToolBarModel::NotationToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int NotationToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QVariant NotationToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    MenuItem item = m_items[index.row()];

    switch (role) {
    case TitleRole: return item.title;
    case CodeRole: return QString::fromStdString(item.code);
    case IconRole: return static_cast<int>(item.iconCode);
    case EnabledRole: return item.state.enabled;
    case DescriptionRole: return item.description;
    case ShortcutRole: return item.shortcutsTitle();
    }

    return QVariant();
}

QHash<int, QByteArray> NotationToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TitleRole, "title" },
        { CodeRole, "code" },
        { IconRole, "icon" },
        { EnabledRole, "enabled" },
        { DescriptionRole, "description" },
        { ShortcutRole, "shortcut" }
    };

    return roles;
}

void NotationToolBarModel::load()
{
    beginResetModel();

    m_items.clear();

    m_items << makeItem("parts");
    m_items << makeItem("toggle-mixer");

    endResetModel();

    context()->currentMasterNotationChanged().onNotify(this, [this]() {
        load();
    });
}

void NotationToolBarModel::handleAction(const QString& actionCode)
{
    dispatcher()->dispatch(actions::codeFromQString(actionCode));
}

MenuItem NotationToolBarModel::makeItem(const actions::ActionCode& actionCode) const
{
    MenuItem item = actionsRegister()->action(actionCode);
    item.state.enabled = context()->currentNotation() != nullptr;

    return item;
}
