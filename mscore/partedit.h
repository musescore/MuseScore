//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#if 0

#ifndef __ILEDIT_H__
#define __ILEDIT_H__

#include "ui_partedit.h"

namespace Ms {

class Score;
struct Channel;
class Part;

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

class PartEdit : public QWidget, public Ui::PartEditBase {
      //Q_OBJECT

      Channel* channel;
      Part* part;

   private slots:
      void patchChanged(int);
      void volChanged(double);
      void panChanged(double);
      void reverbChanged(double);
      void chorusChanged(double);
      void muteChanged(bool);
      void soloToggled(bool);
      void drumsetToggled(bool);

   public slots:
      void updateSolo();

   signals:
      void soloChanged();

   public:
      PartEdit(QWidget* parent = 0);
      void setPart(Part*, Channel*);
      };

//---------------------------------------------------------
//   InstrumentListEditor
//---------------------------------------------------------

class InstrumentListEditor : public QScrollArea
      {
      //Q_OBJECT
      Score*       cs;
      QScrollArea* sa;
      QVBoxLayout* vb;

      virtual void closeEvent(QCloseEvent*);

   private slots:
      void updateSolo();

   public slots:
      void patchListChanged();

   signals:
      void soloChanged();

   public:
      InstrumentListEditor(QWidget* parent);
      void updateAll(Score*);
      };


} // namespace Ms
#endif
#endif
