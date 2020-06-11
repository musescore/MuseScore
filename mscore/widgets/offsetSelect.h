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

#ifndef __OFFSET_SELECT_H__
#define __OFFSET_SELECT_H__

#include "ui_offset_select.h"

namespace Ms {

//---------------------------------------------------------
//   OffsetSelect
//---------------------------------------------------------

class OffsetSelect : public QWidget, public Ui::OffsetSelect {
      Q_OBJECT

      void blockOffset(bool val);

   private slots:
      void _offsetChanged();

   signals:
      void offsetChanged(const QPointF&);

   public:
      OffsetSelect(QWidget* parent);
      void setSuffix(const QString&);
      QPointF offset() const;
      void setOffset(const QPointF&);
      void showRaster(bool);
      };

}


#endif


