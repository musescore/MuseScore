//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "libmscore/element.h"
#include "inspectorElementBase.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorElementBase
//---------------------------------------------------------

InspectorElementBase::InspectorElementBase(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      e.offset->showRaster(true);
      iList = {
            { Pid::VISIBLE,   0, e.visible,    e.resetVisible   },
            { Pid::Z,         0, e.z,          e.resetZ         },
            { Pid::COLOR,     0, e.color,      e.resetColor     },
            { Pid::OFFSET,    0, e.offset,     e.resetOffset    },
            { Pid::AUTOPLACE, 0, e.autoplace,  e.resetAutoplace },
            };
      pList = { { e.title, e.panel } };
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorElementBase::setElement()
      {
      InspectorBase::setElement();
      if (inspector->element()->sizeIsSpatiumDependent())
            e.offset->setSuffix("sp");
      else
            e.offset->setSuffix("mm");
      }

} // namespace Ms


