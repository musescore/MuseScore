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

#ifndef MU_ENGRAVING_APIV1_SCOREELEMENT_H
#define MU_ENGRAVING_APIV1_SCOREELEMENT_H

#include <QQmlEngine>
#include <QQmlListProperty>
#include <QVariant>

#include "engraving/dom/property.h"

namespace mu::engraving {
class EngravingObject;
}

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
//   Ownership
///   \cond PLUGIN_API \private \endcond
///   \internal
///   Represents ownership policy regarding the underlying
///   engraving objects.
//---------------------------------------------------------

enum class Ownership {
    PLUGIN,
    SCORE,
};

//---------------------------------------------------------
//   ScoreElement
///   Base class for most of object wrappers exposed to QML
//---------------------------------------------------------

class ScoreElement : public QObject
{
    Q_OBJECT
    /**
     * Type of this element. See PluginAPI::PluginAPI::EngravingItem
     * for the list of possible values.
     */
    Q_PROPERTY(int type READ type)
    /**
     * Name of this element's type, not localized.
     * Use ScoreElement::userName() to obtain a localized
     * element name suitable for usage in a user interface.
     */
    Q_PROPERTY(QString name READ name)

    Ownership m_ownership;

    qreal spatium() const;

protected:
    /// \cond MS_INTERNAL
    mu::engraving::EngravingObject* const e;
    /// \endcond

public:
    /// \cond MS_INTERNAL
    ScoreElement(mu::engraving::EngravingObject* m_e = nullptr, Ownership own = Ownership::PLUGIN)
        : QObject(), m_ownership(own), e(m_e) {}
    ScoreElement(const ScoreElement&) = delete;
    ScoreElement& operator=(const ScoreElement&) = delete;
    virtual ~ScoreElement();

    Ownership ownership() const { return m_ownership; }
    void setOwnership(Ownership o) { m_ownership = o; }

    mu::engraving::EngravingObject* element() { return e; }
    const mu::engraving::EngravingObject* element() const { return e; }

    QString name() const;
    int type() const;

    QVariant get(mu::engraving::Pid pid) const;
    void set(mu::engraving::Pid pid, const QVariant& val);
    /// \endcond

    Q_INVOKABLE QString userName() const;
    /// Checks whether two variables represent the same object. \since MuseScore 3.3
    Q_INVOKABLE bool is(apiv1::ScoreElement* other) { return other && element() == other->element(); }
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \internal
///   \relates ScoreElement
//---------------------------------------------------------

template<class Wrapper, class T>
Wrapper* wrap(T* t, Ownership own = Ownership::SCORE)
{
    Wrapper* w = t ? new Wrapper(t, own) : nullptr;
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

extern ScoreElement* wrap(mu::engraving::EngravingObject* se, Ownership own = Ownership::SCORE);

//---------------------------------------------------------
//   customWrap
///   \cond PLUGIN_API \private \endcond
///   \internal
///   Can be used to construct wrappers which do not
///   support standard ownership logic or require
///   additional arguments for initialization.
//---------------------------------------------------------

template<class Wrapper, class T, typename ... Args>
Wrapper* customWrap(T* t, Args... args)
{
    Wrapper* w = t ? new Wrapper(t, std::forward<Args>(args)...) : nullptr;
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

//---------------------------------------------------------
///   QML access to containers.
///   A wrapper which provides read-only access for various
///   items containers.
//---------------------------------------------------------

template<typename T, class Container>
class QmlListAccess : public QQmlListProperty<T>
{
public:
    /// \cond MS_INTERNAL
    QmlListAccess(QObject* obj, Container& container)
        : QQmlListProperty<T>(obj,
                              const_cast<void*>(static_cast<const void*>(&container)),
                              &count,
                              &at) {}

    static qsizetype count(QQmlListProperty<T>* l)
    {
        return static_cast<Container*>(l->data)->size();
    }

    static T* at(QQmlListProperty<T>* l, qsizetype i)
    {
        auto el = static_cast<Container*>(l->data)->at(i);
        // If a polymorphic wrap() function is available
        // for the requested type, use it for wrapping.
        if constexpr (std::is_same<T*, decltype(wrap(el, Ownership::SCORE))>::value) {
            return static_cast<T*>(wrap(el, Ownership::SCORE));
        } else { // Otherwise, wrap directly to the requested wrapper type.
            return wrap<T>(el, Ownership::SCORE);
        }
    }

    /// \endcond
};

/** \cond PLUGIN_API \private \endcond */
template<typename T, class Container>
QmlListAccess<T, Container> wrapContainerProperty(QObject* obj, Container& c)
{
    return QmlListAccess<T, Container>(obj, c);
}
}

#endif
