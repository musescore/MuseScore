//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorImage.h"
#include "musescore.h"
#include "libmscore/image.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorImage
//---------------------------------------------------------

InspectorImage::InspectorImage(QWidget* parent)
   : InspectorElementBase(parent)
      {
      b.setupUi(addWidget());

      Element* el = inspector->element();
      bool inFrame = el->parent()->isHBox() || el->parent()->isVBox();
      bool sameTypes = true;

      for (const auto& ee : *inspector->el()) {
            if ((ee->parent()->isHBox() || ee->parent()->isVBox()) != inFrame) {
                  sameTypes = false;
                  break;
                  }
            }
      if (sameTypes) {
            b.autoscale->setDisabled(!inFrame);
            b.resetAutoscale->setDisabled(!inFrame);
            }

      const std::vector<InspectorItem> iiList = {
            { Pid::AUTOSCALE,         0, b.autoscale,       b.resetAutoscale       },
            { Pid::SIZE,              0, b.size,            b.resetSize            },
            { Pid::LOCK_ASPECT_RATIO, 0, b.lockAspectRatio, b.resetLockAspectRatio },
            { Pid::SIZE_IS_SPATIUM,   0, b.sizeIsSpatium,   b.resetSizeIsSpatium   }
            };
      const std::vector<InspectorPanel> ppList = { { b.title, b.panel } };

      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorImage::valueChanged(int idx)
      {
      InspectorBase::valueChanged(idx);
      setElement();     // DEBUG
      }

//---------------------------------------------------------
//   postInit
//---------------------------------------------------------

void InspectorImage::postInit()
      {
      Image* image = toImage(inspector->element());
      bool v = !b.autoscale->isChecked();
      b.size->setEnabled(v);
      b.sizeIsSpatium->setEnabled(v);

      b.size->setSuffix(image->sizeIsSpatium() ? "sp" : "mm");
      }
}

