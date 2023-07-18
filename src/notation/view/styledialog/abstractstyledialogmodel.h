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
#ifndef MU_NOTATION_ABSTRACTSTYLEDIALOGMODEL_H
#define MU_NOTATION_ABSTRACTSTYLEDIALOGMODEL_H

#include <unordered_map>

#include "async/asyncable.h"

#include "notationtypes.h"
#include "inotationstyle.h"

#include "styleitem.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class AbstractStyleDialogModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)

protected:
    explicit AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids);
    StyleItem* styleItem(StyleId id) const;

private:
    INotationStylePtr currentNotationStyle() const;

    StyleItem* buildStyleItem(StyleId id);

    QVariant toUiValue(StyleId id, const PropertyValue& logicalValue) const;
    PropertyValue fromUiValue(StyleId id, const QVariant& uiValue) const;

    std::unordered_map<StyleId, StyleItem*> m_items;
};
}

#endif // MU_NOTATION_ABSTRACTSTYLEDIALOGMODEL_H
