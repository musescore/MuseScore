/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "notation/inotation.h"
#include "notation/utilities/offsetutilities.h"

using namespace mu::notation;
using namespace mu::engraving;

AbstractStyleDialogModel::AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this)), m_ids(ids)
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
    P_TYPE type = mu::engraving::MStyle::valueType(id);

    if (type == P_TYPE::SPATIUM) {
        double value = logicalValue.value<mu::engraving::Spatium>().val();
        if (id == StyleId::measureRepeatNumberPos) {
            // Displayed: positive = up, negative = down
            // Internal:  negative = up, positive = down
            value = OffsetUtils::flipSign(value);
        }
        return value;
    }

    if (type == P_TYPE::POINT) {
        // Displayed: positive = up, negative = down
        // Internal:  negative = up, positive = down
        return OffsetUtils::flipYSign(logicalValue.value<PointF>()).toQPointF();
    }

    return logicalValue.toQVariant();
}

PropertyValue AbstractStyleDialogModel::fromUiValue(StyleId id, const QVariant& uiValue) const
{
    P_TYPE type = mu::engraving::MStyle::valueType(id);

    if (type == P_TYPE::SPATIUM) {
        double value = uiValue.toDouble();
        if (id == StyleId::measureRepeatNumberPos) {
            // Displayed: positive = up, negative = down
            // Internal:  negative = up, positive = down
            value = OffsetUtils::flipSign(value);
        }
        return mu::engraving::Spatium(value);
    }

    if (type == P_TYPE::POINT) {
        // Displayed: positive = up, negative = down
        // Internal:  negative = up, positive = down
        return OffsetUtils::flipYSign(PointF::fromQPointF(uiValue.value<QPointF>()));
    }

    return PropertyValue::fromQVariant(uiValue, type);
}
