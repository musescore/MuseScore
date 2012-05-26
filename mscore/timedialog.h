//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: timedialog.h 4196 2011-04-14 16:11:07Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __TIMEDIALOG_H__
#define __TIMEDIALOG_H__

#include "ui_timedialog.h"

class Palette;
class PaletteScrollArea;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

class TimeDialog : public QWidget, Ui::TimeDialogBase {
      Q_OBJECT

      Palette* sp;
      bool _dirty;

   private slots:
      void addClicked();
      void zChanged(int);
      void nChanged(int);

   public:
      TimeDialog(QWidget* parent = 0);
      bool dirty() const  { return _dirty; }
      void save();
      };

#endif
