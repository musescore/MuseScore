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

#include "sizeSelect.h"
#include "icons.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   SizeSelect
//---------------------------------------------------------

SizeSelect::SizeSelect(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      connect(xVal, SIGNAL(valueChanged(double)), SLOT(_sizeChanged()));
      connect(yVal, SIGNAL(valueChanged(double)), SLOT(_sizeChanged()));
      }

//---------------------------------------------------------
//   setSuffix
//---------------------------------------------------------

void SizeSelect::setSuffix(const QString& s)
      {
      xVal->setSuffix(s);
      yVal->setSuffix(s);
      }

//---------------------------------------------------------
//   _sizeChanged
//---------------------------------------------------------

void SizeSelect::_sizeChanged()
      {
      emit valueChanged(QSizeF(xVal->value(), yVal->value()));
      }

//---------------------------------------------------------
//   blockOffset
//---------------------------------------------------------

void SizeSelect::blockSize(bool val)
      {
      xVal->blockSignals(val);
      yVal->blockSignals(val);
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

QVariant SizeSelect::value() const
      {
      return QSizeF(xVal->value(), yVal->value());
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void SizeSelect::setValue(const QVariant& v)
      {
      QSizeF s = v.toSizeF();
      blockSize(true);
      xVal->setValue(s.width());
      yVal->setValue(s.height());
      blockSize(false);
      }

}


