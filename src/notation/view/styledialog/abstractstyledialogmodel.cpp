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

#include "engraving/style/style.h"

using namespace mu::notation;
using namespace mu::engraving;

AbstractStyleDialogModel::AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids)
    : QObject(parent)
{
    for (StyleId id : ids) {
        m_items.insert_or_assign(id, buildStyleItem(id));
    }

    currentNotationStyle()->styleChanged().onNotify(this, [this]() {
        for (auto [id, item] : m_items) {
            item->setValue(toUiValue(id, currentNotationStyle()->styleValue(id)));
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

StyleItem* AbstractStyleDialogModel::buildStyleItem(StyleId id)
{
    QVariant value = toUiValue(id, currentNotationStyle()->styleValue(id));
    QVariant defaultValue = toUiValue(id, currentNotationStyle()->defaultStyleValue(id));

    StyleItem* item = new StyleItem(this, value, defaultValue);

    connect(item, &StyleItem::valueModified, this, [this, id](const QVariant& newValue) {
        currentNotationStyle()->setStyleValue(id, fromUiValue(id, newValue));
    });

    return item;
}

QVariant AbstractStyleDialogModel::toUiValue(StyleId id, const PropertyValue& logicalValue) const
{
    if (Ms::MStyle::valueType(id) == P_TYPE::SPATIUM) {
        return logicalValue.value<Ms::Spatium>().val();
    }

    return logicalValue.toQVariant();
}

PropertyValue AbstractStyleDialogModel::fromUiValue(StyleId id, const QVariant& uiValue) const
{
    if (Ms::MStyle::valueType(id) == P_TYPE::SPATIUM) {
        return Ms::Spatium(uiValue.toDouble());
    }

    return PropertyValue::fromQVariant(uiValue, Ms::MStyle::valueType(id));
}
