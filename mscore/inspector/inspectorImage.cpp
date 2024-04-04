//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorImage.h"
#include "libmscore/image.h"

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

      updateAspectRatio(); // initiate aspectRatio
      connect(b.lockAspectRatio, &QCheckBox::toggled, this, &InspectorImage::lockAspectRatioClicked);

      connect(b.size->xVal, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this, &InspectorImage::widthChanged);
      connect(b.size->yVal, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this, &InspectorImage::heightChanged);

      mapSignals(iiList, ppList);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorImage::valueChanged(int idx)
      {
      InspectorElementBase::valueChanged(idx);
      setElement();     // DEBUG
      }

//---------------------------------------------------------
//   updateAspectRatio
//---------------------------------------------------------

void InspectorImage::updateAspectRatio()
      {
      Image* image = toImage(inspector->element());
      qreal widthVal = image->size().width();
      qreal heightVal = image->size().height();

      _aspectRatio = !qFuzzyIsNull(heightVal) ? widthVal / heightVal : 1.0;
      }

//---------------------------------------------------------
//   lockAspectRatioClicked
//---------------------------------------------------------

void InspectorImage::lockAspectRatioClicked(bool checked)
      {
      if (checked)
            updateAspectRatio();
      }

//---------------------------------------------------------
//   widthChanged
//---------------------------------------------------------

void InspectorImage::widthChanged(qreal val)
      {
      Image* image = toImage(inspector->element());
      if (image->lockAspectRatio() && !qFuzzyIsNull(val)) // to avoid stack overflow
            b.size->yVal->setValue(val / _aspectRatio);
      }

//---------------------------------------------------------
//   heightChanged
//---------------------------------------------------------

void InspectorImage::heightChanged(qreal val)
      {
      Image* image = toImage(inspector->element());
      if (image->lockAspectRatio() && !qFuzzyIsNull(val)) // to avoid stack overflow
            b.size->xVal->setValue(val * _aspectRatio);
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

