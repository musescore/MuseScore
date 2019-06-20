//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_EXCERPT_H__
#define __PLUGIN_API_EXCERPT_H__

#include "libmscore/excerpt.h"

namespace Ms {

namespace PluginAPI {

class Score;

//---------------------------------------------------------
//   Excerpt
//    Wrapper class for Excerpt
//
//   This is based on the wrapper in scoreelement.h, which
//   we cannot use here, because Ms::Excerpt is not derived
//   from Ms::ScoreElement.
//   Since a plugin should never need to create an Excerpt
//   instance by itself, we don't care for Ownership here.
//---------------------------------------------------------

class Excerpt : public QObject {
    Q_OBJECT
    /** The score object for this part */
    Q_PROPERTY(Ms::PluginAPI::Score* partScore READ partScore)
    /** The title of this part */
    Q_PROPERTY(QString               title     READ title)

    /// \cond MS_INTERNAL
 protected:
    Ms::Excerpt* const e;

 public:
    Excerpt(Ms::Excerpt* _e = nullptr)
       : QObject(), e(_e) {}
    Excerpt(const Excerpt&) = delete;
    Excerpt& operator=(const Excerpt&) = delete;
    virtual ~Excerpt() {}

    Score* partScore();
    QString title() { return e->title(); }
    /// \endcond
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates Excerpt
//---------------------------------------------------------

template <class Wrapper, class T>
Wrapper* excerptWrap(T* t)
      {
      Wrapper* w = t ? new Wrapper(t) : nullptr;
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

extern Excerpt* excerptWrap(Ms::Excerpt* e);

//---------------------------------------------------------
//   qml access to containers of Excerpt
//
//   QmlExcerptsListAccess provides a convenience interface
//   for QQmlListProperty providing read-only access to
//   plugins for Excerpts containers.
//
//   based on QmlListAccess in scoreelement.h
//---------------------------------------------------------

template <typename T, class Container>
class QmlExcerptsListAccess : public QQmlListProperty<T> {
public:
      QmlExcerptsListAccess(QObject* obj, Container& container)
            : QQmlListProperty<T>(obj, &container, &count, &at) {};

      static int count(QQmlListProperty<T>* l)     { return int(static_cast<Container*>(l->data)->size()); }
      static T* at(QQmlListProperty<T>* l, int i)  { return excerptWrap<T>(static_cast<Container*>(l->data)->at(i)); }
      };

/** \cond PLUGIN_API \private \endcond */
template<typename T, class Container>
QmlExcerptsListAccess<T, Container> wrapExcerptsContainerProperty(QObject* obj, Container& c)
      {
      return QmlExcerptsListAccess<T, Container>(obj, c);
      }

} // namespace PluginAPI
} // namespace Ms
#endif
