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

#include "excerpt.h"
#include "score.h"
#include "libmscore/score.h"

namespace Ms {
namespace PluginAPI {

QHash<Ms::Excerpt*, Excerpt*> Excerpt::_wrapMap;

//---------------------------------------------------------
//   Excerpt::partScore
//---------------------------------------------------------

Score* Excerpt::partScore()
      {
      return Ms::PluginAPI::wrap<Score>(e->partScore(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Excerpt::wrap
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

Excerpt* Excerpt::wrap(Ms::Excerpt* excerpt)
      {
      // Use the existing wrapper for this object if it exists.
      if (_wrapMap.contains(excerpt))
            return _wrapMap.value(excerpt);
      // Create a new wrapper.
      Excerpt* w = new Excerpt(excerpt);
      _wrapMap[excerpt] = w;
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

//---------------------------------------------------------
//   excerptWrap
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

Excerpt* excerptWrap(Ms::Excerpt* e)
      {
      return Excerpt::wrap(e);
      }

}
}
