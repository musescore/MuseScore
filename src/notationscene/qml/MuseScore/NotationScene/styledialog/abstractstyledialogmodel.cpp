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
#include "abstractstyledialogmodel.h"

#include "engraving/style/style.h"

using namespace mu::notation;
using namespace mu::engraving;

AbstractStyleDialogModel::AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this)), m_ids(ids)
{
}

StyleItem* AbstractStyleDialogModel::styleItem(StyleId id) const
{
    if (!m_inited) {
        for (StyleId mid : m_ids) {
            m_items.insert_or_assign(mid, buildStyleItem(mid));
        }

        currentNotationStyle()->styleChanged().onNotify(this, [this]() {
            for (auto [id, item] : m_items) {
                item->setValue(toUiValue(id, currentNotationStyle()->styleValue(id)));
            }
        });

        m_inited = true;
    }

    return m_items.at(id);
}

INotationStylePtr AbstractStyleDialogModel::currentNotationStyle() const
{
    return context()->currentNotation()->style();
}

StyleItem* AbstractStyleDialogModel::buildStyleItem(StyleId id) const
{
    QVariant value = toUiValue(id, currentNotationStyle()->styleValue(id));
    QVariant defaultValue = toUiValue(id, currentNotationStyle()->defaultStyleValue(id));

    StyleItem* item = new StyleItem(const_cast<AbstractStyleDialogModel*>(this), value, defaultValue);

    connect(item, &StyleItem::valueModified, this, [this, id](const QVariant& newValue) {
        currentNotationStyle()->setStyleValue(id, fromUiValue(id, newValue));
    });

    return item;
}

QVariant AbstractStyleDialogModel::toUiValue(StyleId id, const PropertyValue& logicalValue) const
{
    if (mu::engraving::MStyle::valueType(id) == P_TYPE::SPATIUM) {
        return logicalValue.value<mu::engraving::Spatium>().val();
    }

    return logicalValue.toQVariant();
}

PropertyValue AbstractStyleDialogModel::fromUiValue(StyleId id, const QVariant& uiValue) const
{
    if (mu::engraving::MStyle::valueType(id) == P_TYPE::SPATIUM) {
        return mu::engraving::Spatium(uiValue.toDouble());
    }

    return PropertyValue::fromQVariant(uiValue, mu::engraving::MStyle::valueType(id));
}
