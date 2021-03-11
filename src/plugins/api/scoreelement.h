//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_SCOREELEMENT_H__
#define __PLUGIN_API_SCOREELEMENT_H__

#include <QVariant>
#include <QQmlListProperty>
#include <QQmlEngine>

#include "libmscore/property.h"

namespace Ms {
class ScoreElement;

namespace PluginAPI {
//---------------------------------------------------------
//   Ownership
///   \cond PLUGIN_API \private \endcond
///   \internal
///   Represents ownership policy regarding the underlying
///   libmscore objects.
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
     * Type of this element. See PluginAPI::PluginAPI::Element
     * for the list of possible values.
     */
    Q_PROPERTY(int type READ type)
    /**
     * Name of this element's type, not localized.
     * Use ScoreElement::userName() to obtain a localized
     * element name suitable for usage in a user interface.
     */
    Q_PROPERTY(QString name READ name)

    Ownership _ownership;

    qreal spatium() const;

protected:
    /// \cond MS_INTERNAL
    Ms::ScoreElement* const e;
    /// \endcond

public:
    /// \cond MS_INTERNAL
    ScoreElement(Ms::ScoreElement* _e = nullptr, Ownership own = Ownership::PLUGIN)
        : QObject(), _ownership(own), e(_e) {}
    ScoreElement(const ScoreElement&) = delete;
    ScoreElement& operator=(const ScoreElement&) = delete;
    virtual ~ScoreElement();

    Ownership ownership() const { return _ownership; }
    void setOwnership(Ownership o) { _ownership = o; }

    Ms::ScoreElement* element() { return e; }
    const Ms::ScoreElement* element() const { return e; }

    QString name() const;
    int type() const;

    QVariant get(Ms::Pid pid) const;
    void set(Ms::Pid pid, QVariant val);
    /// \endcond

    Q_INVOKABLE QString userName() const;
    /// Checks whether two variables represent the same object. \since MuseScore 3.3
    Q_INVOKABLE bool is(Ms::PluginAPI::ScoreElement* other) { return other && element() == other->element(); }
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

extern ScoreElement* wrap(Ms::ScoreElement* se, Ownership own = Ownership::SCORE);

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
        : QQmlListProperty<T>(obj, const_cast<void*>(static_cast<const void*>(&container)), &count, &at) {}

    static int count(QQmlListProperty<T>* l) { return int(static_cast<Container*>(l->data)->size()); }
    static T* at(QQmlListProperty<T>* l, int i)
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
} // namespace PluginAPI
} // namespace Ms
#endif
