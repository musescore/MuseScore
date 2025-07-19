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

#include "enums.h"

#include <QVariant>

#include "log.h"

using namespace mu::engraving::apiv1;

//---------------------------------------------------------
//   Enum::Enum
//---------------------------------------------------------

Enum::Enum(const QMetaEnum& m_enum, QObject* parent)
    : QQmlPropertyMap(this, parent)
{
    add(m_enum);
}

void Enum::add(const QMetaEnum& m_enum)
{
    IF_ASSERT_FAILED(m_enum.isValid()) {
        return;
    }

    QVariantHash enumValues;
    const int nkeys = m_enum.keyCount();
    for (int i = 0; i < nkeys; ++i) {
        enumValues.insert(m_enum.key(i), m_enum.value(i));
    }

    insert(enumValues);
}
