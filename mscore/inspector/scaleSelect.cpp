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

#include "scaleSelect.h"
#include "libmscore/types.h"
#include "icons.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   ScaleSelect
//---------------------------------------------------------

ScaleSelect::ScaleSelect(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      _lock = false;
      connect(xVal, SIGNAL(valueChanged(double)), SLOT(xScaleChanged()));
      connect(yVal, SIGNAL(valueChanged(double)), SLOT(yScaleChanged()));
      }

//---------------------------------------------------------
//   setLock
//---------------------------------------------------------

void ScaleSelect::setLock(bool val)
      {
      if (_lock != val) {
            _lock = val;
            }
      }

//---------------------------------------------------------
//   _scaleChanged
//---------------------------------------------------------

void ScaleSelect::xScaleChanged()
      {
      if (_lock) {
            blockScale(true);
            yVal->setValue(xVal->value());
            blockScale(false);
            }
      emit scaleChanged(QSizeF(xVal->value(), yVal->value()));
      }

void ScaleSelect::yScaleChanged()
      {
      if (_lock) {
            blockScale(true);
            xVal->setValue(yVal->value());
            blockScale(false);
            }
      emit scaleChanged(QSizeF(xVal->value(), yVal->value()));
      }

//---------------------------------------------------------
//   scale
//---------------------------------------------------------

QSizeF ScaleSelect::scale() const
      {
      return QSizeF(xVal->value(), yVal->value());
      }

//---------------------------------------------------------
//   blockScale
//---------------------------------------------------------

void ScaleSelect::blockScale(bool val)
      {
      xVal->blockSignals(val);
      yVal->blockSignals(val);
      }

//---------------------------------------------------------
//   setScale
//---------------------------------------------------------

void ScaleSelect::setScale(const QSizeF& o)
      {
      blockScale(true);
      xVal->setValue(o.width());
      yVal->setValue(o.height());
      blockScale(false);
      }

}


