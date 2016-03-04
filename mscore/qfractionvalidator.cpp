//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "qfractionvalidator.h"
#include "libmscore/scale.h"

namespace Ms {

QFractionValidator::QFractionValidator(QObject* parent)
      : QDoubleValidator(parent)
      {
      }

void QFractionValidator::setStoringMode(int val)
      {
      storingMode = val;
      }

QValidator::State QFractionValidator::validate(QString& value, int& pos) const
      {
      if (!value.contains('/'))
            return QDoubleValidator::validate(value, pos);
      else if (value.endsWith('/')) {
            int newPos = 0;
            QString newValue = value;
            newValue.truncate(value.size() - 1);
            value += "1";
            return intValidator.validate(newValue, newPos);
            }
      else if (storingMode != Scale::DELTA_CENTS) {
            QStringList values = value.split('/');
            if (values.size() != 2)
                  return Invalid;

            int newPos = 0;
            if (Acceptable == intValidator.validate(values[0], newPos) &&
                Acceptable == intValidator.validate(values[1], newPos))
                return Acceptable;
            }
      return Invalid;
      }
}
