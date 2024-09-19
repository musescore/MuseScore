/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extensionstoolbarmodel.h"

#include "log.h"

using namespace muse::extensions;
using namespace muse::ui;
using namespace muse::actions;

ExtensionsToolBarModel::ExtensionsToolBarModel()
{
}

void ExtensionsToolBarModel::init()
{
    extensionsProvider()->toolBarConfigChanged().onNotify(this, [this]() {
        load();
    });

    actionsRegister()->actionStateChanged().onReceive(this, [this](const ActionCodeList& codes) {
        if (codes.empty()) {
            return;
        }

        for (const ActionCode& code : codes) {
            int idx = indexByAction(code);
            if (idx == -1) {
                continue;
            }

            Item& item = m_items[idx];
            item.state = actionsRegister()->actionState(code);

            dataChanged(index(idx), index(idx), { EnabledRole });
        }
    });

    load();
}

void ExtensionsToolBarModel::load()
{
    ToolBarConfig toolBarConfig = extensionsProvider()->toolBarConfig();

    beginResetModel();

    m_items.clear();

    for (const UiControl& c : toolBarConfig.controls) {
        Item item;
        item.control = c;
        const UiAction& action = actionsRegister()->action(c.actionCode);
        if (!action.isValid()) {
            LOGE() << "not found action: " << c.actionCode;
            continue;
        }
        item.action = action;
        item.state = actionsRegister()->actionState(action.code);

        m_items.append(std::move(item));
    }

    endResetModel();

    emit isEmptyChanged();
}

int ExtensionsToolBarModel::indexByAction(const ActionCode& code) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).action.code == code) {
            return i;
        }
    }
    return -1;
}

void ExtensionsToolBarModel::onClicked(int idx)
{
    IF_ASSERT_FAILED(idx >= 0 && idx < m_items.size()) {
        return;
    }

    const Item& item = m_items.at(idx);

    dispatcher()->dispatch(item.action.code);
}

QVariant ExtensionsToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case IconRole: return QVariant::fromValue(item.control.icon);
    case EnabledRole: return item.state.enabled;
    }
    return QVariant();
}

int ExtensionsToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> ExtensionsToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { IconRole, "iconRole" },
        { EnabledRole, "enabledRole" }
    };
    return roles;
}

bool ExtensionsToolBarModel::isEmpty() const
{
    return m_items.isEmpty();
}
