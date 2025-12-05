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

#include "engraving/style/style.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
//   MStyle
///   Provides an access to score style settings.
///   Style settings for a score can be obtained by
///   querying the \ref Score.style property.
///
///   Usage example:
///   \code
///   var style = curScore.style;
///   var genClef = style.value("genClef"); // retrieves style setting on clefs generation, true by default
///   style.setValue("genClef", false); // disables generating clefs for this score
///   \endcode
///   \since MuseScore 3.5
///   \see \ref Sid
//---------------------------------------------------------

/** APIDOC
 * Provides an access to score style settings.
 * Style settings for a score can be obtained by
 * querying the `Score.style` property.
 * @class Style
 * @memberof Engraving
 * @hideconstructor
 * @example
 * let style = curScore.style;
 * let genClef = style.value("genClef"); // retrieves style setting on clefs generation, true by default
 * style.setValue("genClef", false); // disables generating clefs for this score
*/
class MStyle : public QObject
{
    Q_OBJECT

public:
    MStyle(mu::engraving::MStyle* style, mu::engraving::Score* score)
        : QObject(), m_style(style), m_score(score) {}

    /** APIDOC
    * Get the value of style by key.
    * Key should be one of `Sid` values.
    * @method
    * @param {Engraving.Sid} key Key
    * @return {Variant} Value. Type of the returned value depends on type of the corresponding style setting.
    */
    Q_INVOKABLE QVariant value(const QString& key) const;

    /** APIDOC
    * Sets the value of style setting named `key` to `value`.
    * Key should be one of `Sid` values.
    * @method
    * @param {Engraving.Sid} key Key
    * @param {Variant} value Value
    */
    Q_INVOKABLE void setValue(const QString& key, QVariant value);

    /** APIDOC
    * Resets the value of style setting named `key` to its default.
    * Key should be one of `Sid` values.
    * @method
    * @param {Engraving.Sid} key Key
    * @since 4.7
    */
    Q_INVOKABLE void resetValue(const QString& key);

private:

    static engraving::Sid keyToSid(const QString& key);

    mu::engraving::MStyle* m_style = nullptr;
    mu::engraving::Score* m_score = nullptr;
};

extern MStyle* styleWrap(mu::engraving::MStyle*, mu::engraving::Score*);
}
