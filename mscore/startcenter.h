//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STARTCENTER_H__
#define __STARTCENTER_H__

#include "ui_startcenter.h"

namespace Ms {

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

class Startcenter : public QDialog, public Ui::Startcenter
      {
      Q_OBJECT

      virtual void closeEvent(QCloseEvent*);

   signals:
      void closed(bool);

   public:
      Startcenter();
      };
}


#endif

