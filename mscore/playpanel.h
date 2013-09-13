//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: playpanel.h 4722 2011-09-01 12:26:07Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __PLAYPANEL_H__
#define __PLAYPANEL_H__

#include "ui_playpanel.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

class PlayPanel : public QWidget, private Ui::PlayPanelBase {
      Q_OBJECT
      int cachedTickPosition;
      int cachedTimePosition;

      Score* cs;
      virtual void closeEvent(QCloseEvent*);
      virtual void hideEvent (QHideEvent* event);

   private slots:
      void volumeChanged(double,int);
      void relTempoChanged(double,int);
      void relTempoChanged();

   signals:
      void relTempoChanged(double);
      void posChange(int);
      void gainChange(float);
      void closed(bool);

   public slots:
      void setGain(float);
      void setPos(int);
      void heartBeat(int rpos, int apos, int samples);

   public:
      PlayPanel(QWidget* parent = 0);
      ~PlayPanel();

      void setTempo(double);
      void setRelTempo(qreal);

      void setEndpos(int);
      void setScore(Score* s);
   private:
      void updateTimeLabel(int sec);
      void updatePosLabel(int utick);
      };


} // namespace Ms
#endif

