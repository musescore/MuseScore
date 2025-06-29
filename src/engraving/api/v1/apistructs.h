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

#include <QQmlEngine>

#include "engraving/dom/interval.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

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
    void setFraction(engraving::Fraction m_f) { f = m_f; }

public:
    FractionWrapper() = default;
    FractionWrapper(const mu::engraving::Fraction& m_f)
        : f(m_f) {}

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

//---------------------------------------------------------
//   OrnamentIntervalWrapper
///   Ornament interval wrapper object available to QML plugins.
///   Use PluginAPI::PluginAPI::ornamentInterval to create an
///   ornament interval for usage within your plugin:
///   \code
///   var interval = ornamentInterval(IntervalStep.SECOND, IntervalType.MAJOR);
///   \endcode
//---------------------------------------------------------

class OrnamentIntervalWrapper : public QObject
{
    Q_OBJECT
    /// Interval step
    Q_PROPERTY(IntervalStep step READ step)
    /// Interval type
    Q_PROPERTY(IntervalType type READ type)
    /// Whether this interval is perfect
    Q_PROPERTY(bool isPerfect READ isPerfect)

    mu::engraving::OrnamentInterval o;

    /// \cond MS_INTERNAL
public slots:
    void setOrnamentInterval(engraving::OrnamentInterval m_o) { o = m_o; }

public:
    OrnamentIntervalWrapper() = default;
    OrnamentIntervalWrapper(const mu::engraving::OrnamentInterval& m_o)
        : o(m_o) {}

    mu::engraving::OrnamentInterval ornamentInterval() const { return o; }
    IntervalStep step() const { return IntervalStep(o.step); }
    IntervalType type() const { return IntervalType(o.type); }
    bool isPerfect() const { return o.isPerfect(); }
    /// \endcond
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates OrnamentIntervalWrapper
//---------------------------------------------------------

inline OrnamentIntervalWrapper* wrap(mu::engraving::OrnamentInterval o)
{
    OrnamentIntervalWrapper* w = new OrnamentIntervalWrapper(o);
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

//---------------------------------------------------------
//   IntervalWrapper
///   Interval wrapper object available to QML plugins.
///   Use PluginAPI::PluginAPI::interval to create an
///   interval for usage within your plugin:
///   \code
///   var interval = interval(6, 9); // Major 6th
///   \endcode
//---------------------------------------------------------

class IntervalWrapper : public QObject
{
    Q_OBJECT
    /// Diatonic steps
    Q_PROPERTY(int diatonic READ diatonic)
    /// Chromatic steps
    Q_PROPERTY(int chromatic READ chromatic)
    /// Whether this interval is perfect
    Q_PROPERTY(bool isZero READ isZero)

    mu::engraving::Interval i;

    /// \cond MS_INTERNAL
public slots:
    void setInterval(engraving::Interval m_i) { i = m_i; }

public:
    IntervalWrapper() = default;
    IntervalWrapper(const mu::engraving::Interval& m_i)
        : i(m_i) {}

    mu::engraving::Interval interval() const { return i; }
    int diatonic() const { return i.diatonic; }
    int chromatic() const { return i.chromatic; }
    bool isZero() const { return i.isZero(); }
    /// \endcond

    /// Flip (invert) the interval.
    Q_INVOKABLE void flip() { i.flip(); }
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates IntervalWrapper
//---------------------------------------------------------

inline IntervalWrapper* wrap(mu::engraving::Interval i)
{
    IntervalWrapper* w = new IntervalWrapper(i);
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}
}
