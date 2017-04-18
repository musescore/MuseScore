//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __TIMEDIALOG_H__
#define __TIMEDIALOG_H__

#include "ui_timedialog.h"
#include "libmscore/fraction.h"

namespace Ms {

class Palette;
class PaletteScrollArea;
class TimeSig;
class Score;
class Chord;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

class TimeDialog : public QWidget, Ui::TimeDialogBase {
      Q_OBJECT

      PaletteScrollArea* _timePalette;
      Palette* sp;
      bool _dirty;
      TimeSig* timesig;

      int denominator() const;
      int denominator2Idx(int) const;

   private slots:
      void addClicked();
      void zChanged(int);
      void nChanged(int);
      void paletteChanged(int idx);
      void setDirty() { _dirty = true; }

   public:
      TimeDialog(QWidget* parent = 0);
      bool dirty() const { return _dirty; }
      void save();
      };
}

#endif
