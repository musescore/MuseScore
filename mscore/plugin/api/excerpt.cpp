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

//---------------------------------------------------------
//   Excerpt::partScore
//---------------------------------------------------------

Score* Excerpt::partScore()
      {
      return wrap<Score>(e->partScore(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

Excerpt* excerptWrap(Ms::Excerpt* e)
      {
      return excerptWrap<Excerpt>(e);
      }

}
}
