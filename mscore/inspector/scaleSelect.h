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

#ifndef __SCALE_SELECT_H__
#define __SCALE_SELECT_H__

#include "ui_scale_select.h"

namespace Ms {

//---------------------------------------------------------
//   ScaleSelect
//---------------------------------------------------------

class ScaleSelect : public QWidget, public Ui::ScaleSelect {
      Q_OBJECT

      bool _lock;
      void blockScale(bool val);

   private slots:
      void xScaleChanged();
      void yScaleChanged();

   signals:
      void scaleChanged(const QSizeF&);

   public:
      ScaleSelect(QWidget* parent);
      QSizeF scale() const;
      void setScale(const QSizeF&);
      void setLock(bool);
      };

}


#endif


