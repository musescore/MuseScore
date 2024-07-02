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

#include "style.h"

#include <QQmlEngine>
#include <QMetaEnum>

#include "engraving/dom/score.h"

#include "log.h"

using namespace mu::engraving::apiv1;

MStyle* mu::engraving::apiv1::styleWrap(mu::engraving::MStyle* style, mu::engraving::Score* score)
{
    MStyle* st = new MStyle(style, score);
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(st, QQmlEngine::JavaScriptOwnership);
    return st;
}

mu::engraving::Sid MStyle::keyToSid(const QString& key)
{
    static const QMetaEnum sidEnum = QMetaEnum::fromType<Sid>();

    QByteArray ba = key.toLatin1();
    bool ok;
    int val = sidEnum.keyToValue(ba.constData(), &ok);

    if (ok) {
        return static_cast<Sid>(val);
    } else {
        LOGW("Invalid style key: %s", qPrintable(key));
        return Sid::NOSTYLE;
    }
}

//---------------------------------------------------------
//   MStyle::value
///   Returns a value of style setting named \p key.
///   Key should be one of \ref Sid values. Type of the
///   returned value depends on type of the corresponding
///   style setting.
//---------------------------------------------------------

QVariant MStyle::value(const QString& key) const
{
    const Sid sid = keyToSid(key);

    if (sid == Sid::NOSTYLE) {
        return QVariant();
    }

    const PropertyValue val = _style->value(sid);
    return val.toQVariant();
}

//---------------------------------------------------------
//   MStyle::setValue
///   Sets the value of style setting named \p key to \p value.
///   Key should be one of \ref Sid values.
//---------------------------------------------------------

void MStyle::setValue(const QString& key, QVariant value)
{
    const Sid sid = keyToSid(key);

    if (sid == Sid::NOSTYLE) {
        return;
    }

    if (_score) {
        // Style belongs to actual score: change style value in undoable way
        _score->undoChangeStyleVal(sid, PropertyValue::fromQVariant(value, mu::engraving::MStyle::valueType(sid)));
    } else {
        // Style is not bound to a score: change the value directly
        _style->set(sid, PropertyValue::fromQVariant(value, mu::engraving::MStyle::valueType(sid)));
    }
}
