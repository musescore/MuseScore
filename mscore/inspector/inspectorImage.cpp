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

enum ImageControl : char {
      COLOR, VISIBLE, OFF_X, OFF_Y,                   // Element controls
      AUTOSCALE, SIZE_W, SIZE_H, SCALE_W, SCALE_H,    // Image controls
      LOCK_RATIO, SIZE_IS_SPATIUM
      };

//---------------------------------------------------------
//   InspectorImage
//---------------------------------------------------------

InspectorImage::InspectorImage(QWidget* parent)
   : InspectorElementBase(parent)
      {
      b.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { P_ID::AUTOSCALE,         0, false, b.autoscale,       b.resetAutoscale       },
            { P_ID::SIZE,              0, false, b.sizeWidth,       0                      },
            { P_ID::SIZE,              1, false, b.sizeHeight,      0                      },
            { P_ID::SCALE,             0, false, b.scaleWidth,      0                      },
            { P_ID::SCALE,             1, false, b.scaleHeight,     0                      },
            { P_ID::LOCK_ASPECT_RATIO, 0, false, b.lockAspectRatio, b.resetLockAspectRatio },
            { P_ID::SIZE_IS_SPATIUM,   0, false, b.sizeIsSpatium,   b.resetSizeIsSpatium   }
            };

      mapSignals(iiList);
      }

//---------------------------------------------------------
//   updateScaleFromSize
//---------------------------------------------------------

void InspectorImage::updateScaleFromSize(const QSizeF& sz)
      {
      Image* image = static_cast<Image*>(inspector->element());
      QSizeF scale;
      if (image->isValid())
            scale = image->scaleForSize(sz);

      QDoubleSpinBox* b1 = b.scaleWidth;
      QDoubleSpinBox* b2 = b.scaleHeight;
      b1->blockSignals(true);
      b2->blockSignals(true);
      b1->setValue(scale.width());
      b2->setValue(scale.height());
      b1->blockSignals(false);
      b2->blockSignals(false);
      }

//---------------------------------------------------------
//   updateSizeFromScale
//---------------------------------------------------------

void InspectorImage::updateSizeFromScale(const QSizeF& scale)
      {
      Image* image = static_cast<Image*>(inspector->element());
      QSizeF size;
      if (image->isValid())
            size = image->sizeForScale(scale);

      QDoubleSpinBox* b1 = b.sizeWidth;
      QDoubleSpinBox* b2 = b.sizeHeight;
      b1->blockSignals(true);
      b2->blockSignals(true);
      b1->setValue(size.width());
      b2->setValue(size.height());
      b1->blockSignals(false);
      b2->blockSignals(false);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorImage::valueChanged(int idx)
      {
      QDoubleSpinBox* b1 = b.sizeWidth;
      QDoubleSpinBox* b2 = b.sizeHeight;
      QDoubleSpinBox* b3 = b.scaleWidth;
      QDoubleSpinBox* b4 = b.scaleHeight;
      Image* image = static_cast<Image*>(inspector->element());
      if (idx == ImageControl::AUTOSCALE) {
            bool v = !b.autoscale->isChecked();
            b1->setEnabled(v);
            b2->setEnabled(v);
            b.scaleWidth->setEnabled(v);
            b.scaleHeight->setEnabled(v);
            }
      if (idx == ImageControl::SIZE_W) {
            if (b.lockAspectRatio->isChecked()) {
                  QSizeF sz = image->getProperty(P_ID::SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal h     = b1->value() / ratio;
                  b2->blockSignals(true);
                  b2->setValue(h);
                  b2->blockSignals(false);
                  InspectorBase::valueChanged(SIZE_H);
                  }
            updateScaleFromSize(QSizeF(b1->value(), b2->value()));
            }
      else if (idx == ImageControl::SIZE_H) {
            if (b.lockAspectRatio->isChecked()) {
                  QSizeF sz   = image->getProperty(P_ID::SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal w     = b2->value() * ratio;
                  b1->blockSignals(true);
                  b1->setValue(w);
                  b1->blockSignals(false);
                  InspectorBase::valueChanged(ImageControl::SIZE_W);
                  }
            updateScaleFromSize(QSizeF(b1->value(), b2->value()));
            }
      else if (idx == ImageControl::SCALE_W) {
            if (b.lockAspectRatio->isChecked()) {
/* ImageControl::LOCK_RATIO keeps original ratio:
//      NEEDS case "else if(idx == ImageControl::LOCK_RATIO) ..." to restore original ratio on checking ImageControl::LOCK_RATIO
                  b4->blockSignals(true);
                  b4->setValue(b3->value());
                  b4->blockSignals(false);*/
/* ImageControl::LOCK_RATIO keeps current ratio: */
                  QSizeF sz   = inspector->element()->getProperty(P_ID::SCALE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal w     = b3->value() / ratio;
                  b4->blockSignals(true);
                  b4->setValue(w);
                  b4->blockSignals(false);
                  InspectorBase::valueChanged(ImageControl::SCALE_H);
                  }
            updateSizeFromScale(QSizeF(b3->value(), b4->value()));
            }
      else if (idx == SCALE_H) {
            if (b.lockAspectRatio->isChecked()) {
/* ImageControl::LOCK_RATIO keeps original ratio:
//      NEEDS case "else if(idx == ImageControl::LOCK_RATIO) ..." to restore original ratio on checking ImageControl::LOCK_RATIO
                  b3->blockSignals(true);
                  b3->setValue(b4->value());
                  b3->blockSignals(false);*/
/* ImageControl::LOCK_RATIO keeps current ratio: */
                  QSizeF sz   = inspector->element()->getProperty(P_ID::SCALE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal w     = b4->value() * ratio;
                  b3->blockSignals(true);
                  b3->setValue(w);
                  b3->blockSignals(false);
                  InspectorBase::valueChanged(ImageControl::SCALE_W);
                  }
            updateSizeFromScale(QSizeF(b3->value(), b4->value()));
            }
      else if (idx == SIZE_IS_SPATIUM) {
            QCheckBox* cb = static_cast<QCheckBox*>(iList[idx].w);
            qreal _spatium = inspector->element()->spatium();
            b1->blockSignals(true);
            b2->blockSignals(true);
            if (cb->isChecked()) {
                  b1->setSuffix("sp");
                  b2->setSuffix("sp");
                  b1->setValue(b1->value() * DPMM / _spatium);
                  b2->setValue(b2->value() * DPMM / _spatium);
                  }
            else {
                  b1->setSuffix("mm");
                  b2->setSuffix("mm");
                  b1->setValue(b1->value() * _spatium / DPMM);
                  b2->setValue(b2->value() * _spatium / DPMM);
                  }
            b1->blockSignals(false);
            b2->blockSignals(false);
            }
      InspectorElementBase::valueChanged(idx);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorImage::setElement(Element* e)
      {
      Image* image = static_cast<Image*>(e);
      QDoubleSpinBox* b1 = static_cast<QDoubleSpinBox*>(iList[SIZE_W].w);
      QDoubleSpinBox* b2 = static_cast<QDoubleSpinBox*>(iList[SIZE_H].w);
      if (image->sizeIsSpatium()) {
            b1->setSuffix("sp");
            b2->setSuffix("sp");
            }
      else {
            b1->setSuffix("mm");
            b2->setSuffix("mm");
            }
      bool v = !image->autoScale();
      b1->setEnabled(v);
      b2->setEnabled(v);
      iList[SCALE_H].w->setEnabled(v);
      iList[SCALE_W].w->setEnabled(v);

      InspectorElementBase::setElement();
      }
}

