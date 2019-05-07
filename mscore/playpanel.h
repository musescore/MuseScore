//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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
#include "enableplayforwidget.h"


namespace Ms {

class Score;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

class PlayPanel : public QDockWidget, private Ui::PlayPanelBase {
      Q_OBJECT
      int cachedTickPosition;
      int cachedTimePosition;
      bool tempoSliderIsPressed;
      EnablePlayForWidget* enablePlay;
      static constexpr double MUTE = 0.00 ;
      static constexpr double N = 20.0 ;
      double vol;
      double svol;


      Score* cs;
      virtual void closeEvent(QCloseEvent*);
      virtual void hideEvent (QHideEvent* event);
      virtual void showEvent(QShowEvent *);
      virtual bool eventFilter(QObject *, QEvent *);
      virtual void keyPressEvent(QKeyEvent*) override;
      void updateTimeLabel(int sec);
      void updatePosLabel(int utick);

   private slots:
      void volumeChanged(double,int);
      void metronomeGainChanged(double val, int);
      void relTempoChanged(double,int);
      void relTempoChanged();
      void tempoSliderReleased(int);
      void tempoSliderPressed(int);
      void volLabel();
      void volSpinBoxEdited();

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate()  { retranslateUi(this); }

   signals:
      void relTempoChanged(double);
      void metronomeGainChanged(float);
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
      bool isTempoSliderPressed() {return tempoSliderIsPressed;}
      };


} // namespace Ms
#endif

