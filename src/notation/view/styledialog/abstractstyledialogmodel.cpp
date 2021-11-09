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
#include "abstractstyledialogmodel.h"

using namespace mu::notation;

AbstractStyleDialogModel::AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids)
    : QObject(parent)
{
    for (StyleId id : ids) {
        QVariant value = currentNotationStyle()->styleValue(id);
        QVariant defaultValue = currentNotationStyle()->defaultStyleValue(id);

        StyleItem* item = new StyleItem(this, value, defaultValue);

        connect(item, &StyleItem::valueModified, this, [this, id](const QVariant& newValue) {
            currentNotationStyle()->setStyleValue(id, newValue);
        });

        m_items.insert_or_assign(id, item);
    }

    currentNotationStyle()->styleChanged().onNotify(this, [this]() {
        for (auto [id, item] : m_items) {
            item->setValue(currentNotationStyle()->styleValue(id));
        }
    });
}

StyleItem* AbstractStyleDialogModel::styleItem(StyleId id) const
{
    return m_items.at(id);
}

INotationStylePtr AbstractStyleDialogModel::currentNotationStyle() const
{
    return context()->currentNotation()->style();
}
