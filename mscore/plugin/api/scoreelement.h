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

#include "libmscore/property.h"

namespace Ms {

class ScoreElement;

namespace PluginAPI {

//---------------------------------------------------------
//   Ownership
//    Represents ownership policy regarding the underlying
//    libmscore objects.
//---------------------------------------------------------

enum class Ownership {
      PLUGIN,
      SCORE,
      };

//---------------------------------------------------------
//   ScoreElement
//    Wrapper class for ScoreElement
//---------------------------------------------------------

class ScoreElement : public QObject {
      Q_OBJECT
      Q_PROPERTY(int       type READ type)
      Q_PROPERTY(QString   name READ name)

      Ownership _ownership;

   protected:
      Ms::ScoreElement* const e;

   public:
      ScoreElement(Ms::ScoreElement* _e = nullptr, Ownership own = Ownership::PLUGIN)
         : QObject(), _ownership(own), e(_e) {}
      ScoreElement(const ScoreElement&) = delete;
      ScoreElement& operator=(const ScoreElement&) = delete;
      virtual ~ScoreElement();

      Ownership ownership() const { return _ownership; }
      void setOwnership(Ownership o) { _ownership = o; }

      Ms::ScoreElement* element() { return e; };
      const Ms::ScoreElement* element() const { return e; };

      QString name() const;
      int type() const;
      //@ Returns the human-readable name of the element type
      Q_INVOKABLE QString userName() const;

      QVariant get(Ms::Pid pid) const;
      void set(Ms::Pid pid, QVariant val);
      };

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

template <class Wrapper, class T>
Wrapper* wrap(T* t, Ownership own = Ownership::SCORE)
      {
      Wrapper* w = t ? new Wrapper(t, own) : nullptr;
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

extern ScoreElement* wrap(Ms::ScoreElement* se, Ownership own = Ownership::SCORE);

//---------------------------------------------------------
//   qml access to containers
//
//   QmlListAccess provides a convenience interface for
//   QQmlListProperty providing read-only access to plugins
//   for various items containers.
//---------------------------------------------------------

template <typename T, class Container>
class QmlListAccess : public QQmlListProperty<T> {
public:
      QmlListAccess(QObject* obj, Container& container)
            : QQmlListProperty<T>(obj, &container, &count, &at) {};

      static int count(QQmlListProperty<T>* l)     { return int(static_cast<Container*>(l->data)->size()); }
      static T* at(QQmlListProperty<T>* l, int i)  { return wrap<T>(static_cast<Container*>(l->data)->at(i), Ownership::SCORE); }
      };

template<typename T, class Container>
QmlListAccess<T, Container> wrapContainerProperty(QObject* obj, Container& c)
      {
      return QmlListAccess<T, Container>(obj, c);
      }
} // namespace PluginAPI
} // namespace Ms
#endif
