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

#ifndef __QFRACTION_VALIDATOR_H__
#define __QFRACTION_VALIDATOR_H__

namespace Ms {

//---------------------------------------------------------
//   ExportMidi
//---------------------------------------------------------

class QFractionValidator : public QDoubleValidator {
      Q_OBJECT

   public:
      explicit QFractionValidator(QObject* parent = 0);

      QValidator::State validate(QString& value, int& pos) const;

      void setStoringMode(int val);

   private:
      QIntValidator intValidator;
      int storingMode;
};
      
} // namespace Ms
#endif
