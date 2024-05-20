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

#ifndef MU_ENGRAVING_APIV1_EXCERPT_H
#define MU_ENGRAVING_APIV1_EXCERPT_H

#include <QQmlEngine>
#include <QQmlListProperty>

#include "engraving/dom/excerpt.h"

namespace mu::engraving::apiv1 {
class Score;

//---------------------------------------------------------
//   Excerpt
//    Wrapper class for Excerpt
//
//   This is based on the wrapper in scoreelement.h, which
//   we cannot use here, because mu::engraving::Excerpt is not derived
//   from mu::engraving::ScoreElement.
//   Since a plugin should never need to create an Excerpt
//   instance by itself, we don't care for Ownership here.
//---------------------------------------------------------

class Excerpt : public QObject
{
    Q_OBJECT
    /** The score object for this part */
    Q_PROPERTY(apiv1::Score * partScore READ partScore)
    /** The title of this part */
    Q_PROPERTY(QString title READ title)

protected:
    /// \cond MS_INTERNAL
    mu::engraving::Excerpt* const e;
    /// \endcond

public:
    /// \cond MS_INTERNAL
    Excerpt(mu::engraving::Excerpt* _e = nullptr)
        : QObject(), e(_e) {}
    Excerpt(const Excerpt&) = delete;
    Excerpt& operator=(const Excerpt&) = delete;
    virtual ~Excerpt() {}

    Score* partScore();
    QString title() { return e->name(); }
    /// \endcond

    /// Checks whether two variables represent the same object. \since MuseScore 3.3
    Q_INVOKABLE bool is(apiv1::Excerpt* other) { return other && e == other->e; }
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates Excerpt
//---------------------------------------------------------

template<class Wrapper, class T>
Wrapper* excerptWrap(T* t)
{
    Wrapper* w = t ? new Wrapper(t) : nullptr;
    // All wrapper objects should belong to JavaScript code.
    QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
    return w;
}

extern Excerpt* excerptWrap(mu::engraving::Excerpt* e);

//---------------------------------------------------------
//   qml access to containers of Excerpt
//
//   QmlExcerptsListAccess provides a convenience interface
//   for QQmlListProperty providing read-only access to
//   plugins for Excerpts containers.
//
//   based on QmlListAccess in scoreelement.h
//---------------------------------------------------------

template<typename T, class Container>
class QmlExcerptsListAccess : public QQmlListProperty<T>
{
public:
    QmlExcerptsListAccess(QObject* obj, Container& container)
        : QQmlListProperty<T>(obj, &container, &count, &at) {}

    static qsizetype count(QQmlListProperty<T>* l)
    {
        return static_cast<Container*>(l->data)->size();
    }

    static T* at(QQmlListProperty<T>* l, qsizetype i)
    {
        return excerptWrap<T>(static_cast<Container*>(l->data)->at(i));
    }
};

/** \cond PLUGIN_API \private \endcond */
template<typename T, class Container>
QmlExcerptsListAccess<T, Container> wrapExcerptsContainerProperty(QObject* obj, Container& c)
{
    return QmlExcerptsListAccess<T, Container>(obj, c);
}
}

#endif
