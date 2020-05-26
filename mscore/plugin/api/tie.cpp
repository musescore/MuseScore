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

#include "tie.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   Tie::startNote
//---------------------------------------------------------

Note* Tie::startNote() { return wrap<Note>(toTie(e)->startNote()); }

//---------------------------------------------------------
//   Tie::endNote
//---------------------------------------------------------

Note* Tie::endNote() { return wrap<Note>(toTie(e)->endNote()); }

//---------------------------------------------------------
//   tieWrap
//---------------------------------------------------------

Tie* tieWrap(Ms::Tie* tie) { return wrap<Tie>(tie); }
}
}
