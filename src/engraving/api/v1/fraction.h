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

#ifndef MU_ENGRAVING_APIV1_FRACTION_H
#define MU_ENGRAVING_APIV1_FRACTION_H

#include <QQmlEngine>

#include "engraving/types/fraction.h"

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
//   FractionWrapper
///   Fraction object available to QML plugins.
///   Use PluginAPI::PluginAPI::fraction to create a
///   fraction for usage within your plugin:
///   \code
///   var ts = newElement(EngravingItem.TIMESIG);
///   ts.timesig = fraction(3, 4);
///   \endcode
//---------------------------------------------------------

class FractionWrapper : public QObject
{
    Q_OBJECT
    /** Fraction numerator */
    Q_PROPERTY(int numerator READ numerator)
    /** Fraction denominator */
    Q_PROPERTY(int denominator READ denominator)
    /**
     * MIDI ticks number equal to the number of the whole
     * notes represented by this fraction.
     */
    Q_PROPERTY(int ticks READ ticks)   // FIXME: fraction transition
    /** String representation of this fraction */
    Q_PROPERTY(QString str READ toString)

    mu::engraving::Fraction f;

    /// \cond MS_INTERNAL
public slots:
    void setFraction(engraving::Fraction _f) { f = _f; }

public:
    FractionWrapper() = default;
    FractionWrapper(const mu::engraving::Fraction& _f)
        : f(_f) {}

    mu::engraving::Fraction fraction() const { return f; }
    int numerator() const { return f.numerator(); }
    int denominator() const { return f.denominator(); }
    int ticks() const { return f.ticks(); }
    QString toString() const { return f.toString(); }
    /// \endcond
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates FractionWrapper
//---------------------------------------------------------

inline FractionWrapper* wrap(mu::engraving::Fraction f)
{
    FractionWrapper* w = new FractionWrapper(f);
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}
}

#endif
