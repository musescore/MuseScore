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
#include "libmscore/elementlayout.h"
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

      connect(xVal, SIGNAL(valueChanged(double)), SLOT(_scaleChanged()));
      connect(yVal, SIGNAL(valueChanged(double)), SLOT(_scaleChanged()));
      }

//---------------------------------------------------------
//   _scaleChanged
//---------------------------------------------------------

void ScaleSelect::_scaleChanged()
      {
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
//   blockOffset
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


