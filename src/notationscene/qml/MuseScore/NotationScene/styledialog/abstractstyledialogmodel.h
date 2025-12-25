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

#pragma once

#include <unordered_map>

#include "async/asyncable.h"

#include "notation/notationtypes.h"
#include "notation/inotationstyle.h"

#include "styleitem.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::notation {
class AbstractStyleDialogModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable as it is an abstract base class")

    muse::Inject<context::IGlobalContext> context = { this };

protected:
    explicit AbstractStyleDialogModel(QObject* parent, std::set<StyleId> ids);
    StyleItem* styleItem(StyleId id) const;

    INotationStylePtr currentNotationStyle() const;

    void addStyleId(StyleId id) { m_ids.insert(id); }

    virtual StyleItem* buildStyleItem(StyleId id) const;

private:
    QVariant toUiValue(StyleId id, const PropertyValue& logicalValue) const;
    PropertyValue fromUiValue(StyleId id, const QVariant& uiValue) const;

    mutable bool m_inited = false;
    std::set<StyleId> m_ids;
    mutable std::unordered_map<StyleId, StyleItem*> m_items;
};
}
