//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: edittempo.h 1840 2009-05-20 11:57:51Z wschweer $
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

#ifndef __EDITTEMPO_H__
#define __EDITTEMPO_H__

#include "ui_edittempo.h"

//---------------------------------------------------------
//   EditTempo
//---------------------------------------------------------

class EditTempo : public QDialog, private Ui::EditTempoBase {
      Q_OBJECT
      QString _text;
      double _bpm;

   private slots:
      void selectTempo(int);
      void itemDoubleClicked(QListWidgetItem*);
      void textChanged(const QString&);
      void bpmChanged(double);

   public:
      EditTempo(QWidget* parent);
      double bpm() const { return _bpm; }
      QString text() const { return _text; }
      };

#endif

