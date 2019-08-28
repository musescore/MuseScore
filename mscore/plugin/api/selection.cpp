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

#include "selection.h"

namespace Ms {
namespace PluginAPI {

QHash<Ms::Selection*, Selection*> Selection::_wrapMap;

//---------------------------------------------------------
//   Selection::wrap
//---------------------------------------------------------

Selection* Selection::wrap(Ms::Selection* select)
      {
      // Use the existing wrapper for this object if it exists.
      if (_wrapMap.contains(select))
            return _wrapMap.value(select);
      // Create a new wrapper.
      Selection* w = new Selection(select);
      _wrapMap[select] = w;
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

//---------------------------------------------------------
//   selectionWrap
//---------------------------------------------------------

Selection* selectionWrap(Ms::Selection* select)
      {
      return Selection::wrap(select);
      }

}
}
