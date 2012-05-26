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

enum {
      AUTOSCALE, SIZE_W, SIZE_H, SCALE_W, SCALE_H,
      LOCK_RATIO, SIZE_IS_SPATIUM
      };

//---------------------------------------------------------
//   InspectorImage
//---------------------------------------------------------

InspectorImage::InspectorImage(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);

      iList[AUTOSCALE].t = P_AUTOSCALE;
      iList[AUTOSCALE].w = b.autoscale;
      iList[AUTOSCALE].r = b.resetAutoscale;

      iList[SIZE_W].t  = P_SIZE;
      iList[SIZE_W].sv = 0;
      iList[SIZE_W].w  = b.sizeWidth;
      iList[SIZE_W].r  = 0;

      iList[SIZE_H].t  = P_SIZE;
      iList[SIZE_H].sv = 1;
      iList[SIZE_H].w  = b.sizeHeight;
      iList[SIZE_H].r  = 0;

      iList[SCALE_W].t  = P_SCALE;
      iList[SCALE_W].sv = 0;
      iList[SCALE_W].w  = b.scaleWidth;
      iList[SCALE_W].r  = 0;

      iList[SCALE_H].t  = P_SCALE;
      iList[SCALE_H].sv = 1;
      iList[SCALE_H].w  = b.scaleHeight;
      iList[SCALE_H].r  = 0;

      iList[LOCK_RATIO].t = P_LOCK_ASPECT_RATIO;
      iList[LOCK_RATIO].w = b.lockAspectRatio;
      iList[LOCK_RATIO].r = b.resetLockAspectRatio;

      iList[SIZE_IS_SPATIUM].t = P_SIZE_IS_SPATIUM;
      iList[SIZE_IS_SPATIUM].w = b.sizeIsSpatium;
      iList[SIZE_IS_SPATIUM].r = b.resetSizeIsSpatium;

      mapSignals();
      }

//---------------------------------------------------------
//   updateScaleFromSize
//---------------------------------------------------------

void InspectorImage::updateScaleFromSize(const QSizeF& sz)
      {
      Image* image = static_cast<Image*>(inspector->element());
      QSizeF scale = image->scaleForSize(sz);

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
      QSizeF size = image->sizeForScale(scale);

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
      if (idx == AUTOSCALE) {
            bool v = !b.autoscale->isChecked();
            b1->setEnabled(v);
            b2->setEnabled(v);
            b.scaleWidth->setEnabled(v);
            b.scaleHeight->setEnabled(v);
            }
      if (idx == SIZE_W) {
            if (b.lockAspectRatio->isChecked()) {
                  QSizeF sz = image->getProperty(P_SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal h     = b1->value() / ratio;
                  b2->blockSignals(true);
                  b2->setValue(h);
                  b2->blockSignals(false);
                  }
            updateScaleFromSize(QSizeF(b1->value(), b2->value()));
            }
      else if (idx == SIZE_H) {
            if (b.lockAspectRatio->isChecked()) {
                  QSizeF sz   = image->getProperty(P_SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();
                  qreal w     = b2->value() * ratio;
                  b1->blockSignals(true);
                  b1->setValue(w);
                  b1->blockSignals(false);
                  }
            updateScaleFromSize(QSizeF(b1->value(), b2->value()));
            }
      else if (idx == SCALE_W) {
            if (b.lockAspectRatio->isChecked()) {
                  // scale width was changed, fix scale height
                  QSizeF sz   = inspector->element()->getProperty(P_SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();

                  qreal h     = b.scaleWidth->value() / ratio;
                  b.scaleHeight->blockSignals(true);
                  b.scaleHeight->setValue(h);
                  b.scaleHeight->blockSignals(false);
                  }
            updateSizeFromScale(QSizeF(b3->value(), b4->value()));
            }
      else if (idx == SCALE_H) {
            if (b.lockAspectRatio->isChecked()) {
                  // scale height was changed, fix scale width
                  QSizeF sz   = inspector->element()->getProperty(P_SIZE).toSizeF();
                  qreal ratio = sz.width() / sz.height();

                  qreal w     = b.scaleHeight->value() * ratio;
                  b.scaleWidth->blockSignals(true);
                  b.scaleWidth->setValue(w);
                  b.scaleWidth->blockSignals(false);
                  }
            updateSizeFromScale(QSizeF(b3->value(), b4->value()));
            }
      else if (idx == SIZE_IS_SPATIUM) {
            QCheckBox* cb = static_cast<QCheckBox*>(iList[idx].w);
            qreal _spatium = inspector->element()->spatium();
            if (cb->isChecked()) {
                  b1->setSuffix("sp");
                  b2->setSuffix("sp");
                  b1->setValue(b1->value() * MScore::DPMM / _spatium);
                  b2->setValue(b2->value() * MScore::DPMM / _spatium);
                  }
            else {
                  b1->setSuffix("mm");
                  b2->setSuffix("mm");
                  b1->setValue(b1->value() * _spatium / MScore::DPMM);
                  b2->setValue(b2->value() * _spatium / MScore::DPMM);
                  }
            }
      InspectorBase::valueChanged(idx);
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

      InspectorBase::setElement(e);
      }

