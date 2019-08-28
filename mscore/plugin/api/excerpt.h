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

    // Map of existing Excerpt wrappers.
    static QHash<Ms::Excerpt*, Excerpt*> _wrapMap;

 protected:
    /// \cond MS_INTERNAL
    Ms::Excerpt* const e;
    /// \endcond

 public:
    /// \cond MS_INTERNAL
    static Excerpt* wrap(Ms::Excerpt* excerpt);

    Excerpt(Ms::Excerpt* _e = nullptr)
       : QObject(), e(_e) {}
    Excerpt(const Excerpt&) = delete;
    Excerpt& operator=(const Excerpt&) = delete;
    virtual ~Excerpt() { _wrapMap.remove(e); }

    Score* partScore();
    QString title() { return e->title(); }
    /// \endcond

    /// Checks whether two variables represent the same object. \since MuseScore 3.3
    Q_INVOKABLE bool is(Ms::PluginAPI::Excerpt* other) { return other && e == other->e; }
};

//---------------------------------------------------------
//   excerptWrap
///   \cond PLUGIN_API \private \endcond
///   \relates Excerpt
//---------------------------------------------------------

extern Excerpt* excerptWrap(Ms::Excerpt* e);

//---------------------------------------------------------
//   qml access to containers of Excerpt
//
//   QmlExcerptsListAccess provides a convenience interface
//   for QQmlListProperty providing read-only access to
//   plugins for Excerpts containers.
//
//---------------------------------------------------------

class QmlExcerptsListAccess : public QQmlListProperty<Excerpt> {
public:
      QmlExcerptsListAccess(QObject* obj, QList<Ms::Excerpt*>& container)
            : QQmlListProperty<Excerpt>(obj, &container, &count, &at) {};

      static int count(QQmlListProperty<Excerpt>* l)     { return int(static_cast<QList<Ms::Excerpt*>*>(l->data)->size()); }
      static Excerpt* at(QQmlListProperty<Excerpt>* l, int i)  { return excerptWrap((*(static_cast<QList<Ms::Excerpt*>*>(l->data)))[i]); }
      };

inline QmlExcerptsListAccess wrapExcerptsContainerProperty(QObject* obj, QList<Ms::Excerpt*>& c)
      {
      return QmlExcerptsListAccess(obj, c);
      }

} // namespace PluginAPI
} // namespace Ms
#endif
