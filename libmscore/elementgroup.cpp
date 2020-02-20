//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "elementgroup.h"
#include "element.h"

namespace Ms {

void SingleElementGroup::startDrag(EditData& ed)
      {
      e->startDrag(ed);
      }

QRectF SingleElementGroup::drag(EditData& ed)
      {
      return e->drag(ed);
      }

void SingleElementGroup::endDrag(EditData& ed)
      {
      e->endDrag(ed);
      e->triggerLayout();
      }

} // namespace Ms
