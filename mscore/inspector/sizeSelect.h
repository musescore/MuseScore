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

#ifndef __SIZE_SELECT_H__
#define __SIZE_SELECT_H__

#include "ui_size_select.h"

namespace Ms {

//---------------------------------------------------------
//   SizeSelect
//---------------------------------------------------------

class SizeSelect : public QWidget, public Ui::SizeSelect {
      Q_OBJECT

      void blockSize(bool val);

   private slots:
      void _sizeChanged();

   signals:
      void valueChanged(const QVariant&);

   public:
      SizeSelect(QWidget* parent);
      void setSuffix(const QString&);
      QVariant value() const;
      void setValue(const QVariant&);
      void setLock(bool val);
      };

}


#endif


