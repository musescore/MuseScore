//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: score.h 5242 2012-01-23 17:25:56Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MASTERPALETTE_H__
#define __MASTERPALETTE_H__

#include "ui_masterpalette.h"

namespace Ms {

class Palette;
class TimeDialog;
class KeyEditor;

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

class MasterPalette : public QWidget, Ui::MasterPalette
      {
      Q_OBJECT

      TimeDialog* timeDialog;
      KeyEditor* keyEditor;

      virtual void closeEvent(QCloseEvent*);
      Palette* createPalette(int w, int h, bool grid, double mag = 1.0);
      void addPalette(Palette* sp);

   signals:
      void closed(bool);

   public:
      MasterPalette(QWidget* parent = 0);
      void selectItem(const QString& s);
      QString selectedItem();
      };

} // namespace Ms
#endif

