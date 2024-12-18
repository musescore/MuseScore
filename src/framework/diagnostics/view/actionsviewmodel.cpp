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
#include "actionsviewmodel.h"

using namespace muse::diagnostics;
using namespace muse::actions;
using namespace muse::ui;

ActionsViewModel::ActionsViewModel()
{
}

template<typename T>
static std::list<T> toList(const std::vector<T>& vec)
{
    std::list<T> l;
    for (const T& v : vec) {
        l.push_back(v);
    }
    return l;
}

void ActionsViewModel::load()
{
    ActionCodeList actions = actionsDispatcher()->actionList();
    std::list<UiAction> uiactions = toList(uiActionsRegister()->actionList());

    auto takeByCode = [](std::list<UiAction>& uia, const ActionCode& code) -> UiAction {
        auto it = std::find_if(uia.begin(), uia.end(), [code](const UiAction& a) {
            return a.code == code;
        });

        if (it == uia.end()) {
            return UiAction();
        }

        UiAction a = *it;
        uia.erase(it);
        return a;
    };

    beginResetModel();

    for (const ActionCode& a : actions) {
        Item item;
        item.isReg = true;
        item.actionCode = QString::fromStdString(a);

        UiAction uia = takeByCode(uiactions, a);
        item.isHasUi = uia.isValid();
        if (item.isHasUi) {
            item.actionTitle = uia.title.qTranslatedWithoutMnemonic();
        } else {
            item.actionTitle = "none";
        }

        m_allItems.append(item);
    }

    for (const UiAction& uia : uiactions) {
        Item item;
        item.isReg = false;
        item.actionCode = QString::fromStdString(uia.code);
        item.isHasUi = true;
        item.actionTitle = uia.title.qTranslatedWithoutMnemonic();
        m_allItems.append(item);
    }

    for (Item& item : m_allItems) {
        item.formatted = QString("action: %1, isreg: %2, title: %3, ishasui: %4")
                         .arg(item.actionCode).arg(item.isReg).arg(item.actionTitle).arg(item.isHasUi);
    }

    std::sort(m_allItems.begin(), m_allItems.end(), [](const Item& f, const Item& s) {
        return f.actionCode < s.actionCode;
    });

    m_items = m_allItems;

    endResetModel();
}

QVariant ActionsViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case rItemData: return QString::number(index.row()) + ". " + m_items.at(index.row()).formatted;
    }

    return QVariant();
}

int ActionsViewModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> ActionsViewModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}

void ActionsViewModel::find(const QString& str)
{
    beginResetModel();

    m_searchText = str;

    if (m_searchText.isEmpty()) {
        m_items = m_allItems;
    } else {
        m_items.clear();
        for (const Item& item : m_allItems) {
            if (item.formatted.contains(m_searchText, Qt::CaseInsensitive)) {
                m_items.append(item);
            }
        }
    }

    endResetModel();
}

void ActionsViewModel::print()
{
    for (const Item& item : m_items) {
        std::cout << item.formatted.toStdString() << std::endl;
    }
}
