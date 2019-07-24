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

}
}
