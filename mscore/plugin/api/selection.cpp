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

#include "libmscore/select.h"
#include "selection.h"
#include "score.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   QmlPlayEventsListAccess::append
//---------------------------------------------------------

Selection* selectionWrap(Ms::Selection* select)
      {
      Selection* w = new Selection(select);
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

//---------------------------------------------------------
//   QmlSelElementsListAccess::at
//---------------------------------------------------------

Element* QmlSelElementsListAccess::at(QQmlListProperty<Element>* l, int i)
      {
      Ms::Selection* sel = static_cast<Ms::Selection*>(l->data);
      return wrap(sel->elements()[i]);
      }

//---------------------------------------------------------
//   QmlSelElementsListAccess::count
//---------------------------------------------------------

int QmlSelElementsListAccess::count(QQmlListProperty<Element>* l)
      {
      Ms::Selection* sel = static_cast<Ms::Selection*>(l->data);
      return sel->elements().length();
      }

//---------------------------------------------------------
//   QmlSelElementsListAccess::clear
//---------------------------------------------------------

void QmlSelElementsListAccess::clear(QQmlListProperty<Element>* l)
      {
      Ms::Selection* sel = static_cast<Ms::Selection*>(l->data);
      sel->clear();
      }

//---------------------------------------------------------
//   QmlSelElementsListAccess::append
//---------------------------------------------------------

void QmlSelElementsListAccess::append(QQmlListProperty<Element>* l, Element *e)
      {
      Ms::Selection* sel = static_cast<Ms::Selection*>(l->data);
      sel->add(e->element());
      }

}
}
