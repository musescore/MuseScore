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

#ifndef __ILEDIT_H__
#define __ILEDIT_H__

#include "ui_mixer.h"
#include "libmscore/instrument.h"
#include "enableplayforwidget.h"
namespace Ms {

class Score;
struct Channel;
class Part;

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

class PartEdit : public QWidget, public Ui::PartEditBase {
      Q_OBJECT

      Channel* channel;
      Part* part;

      QList<QToolButton*> voiceButtons;


   private slots:
      void patchChanged(int, bool syncControls = true);
      void volChanged(double, bool syncControls = true);
      void panChanged(double, bool syncControls = true);
      void reverbChanged(double, bool syncControls = true);
      void chorusChanged(double, bool syncControls = true);
      void muteChanged(bool, bool syncControls = true);
      void soloToggled(bool, bool syncControls = true);
      void drumsetToggled(bool, bool syncControls = true);
      void midiChannelChanged(int);
      void sync(bool syncControls);
      void expandToggled(bool);
      void playbackVoiceChanged();

   public slots:

   signals:
      void soloChanged(bool);

   public:
      PartEdit(QWidget* parent = 0);
      void setPart(Part*, Channel*);
      };

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

class Mixer : public QScrollArea
      {
      Q_OBJECT
      MasterScore* cs;
      QScrollArea* sa;
      QVBoxLayout* vb;
      EnablePlayForWidget* enablePlay;

      virtual void closeEvent(QCloseEvent*);
      virtual void showEvent(QShowEvent*) override;
      virtual bool eventFilter(QObject*, QEvent*) override;
      virtual void keyPressEvent(QKeyEvent*) override;
      void readSettings();

   private slots:
      void updateSolo(bool);

   public slots:
      void patchListChanged();
      void midiPrefsChanged(bool showMidiControls);

   signals:
      void closed(bool);

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate(bool firstTime = false);

   public:
      Mixer(QWidget* parent);
      void updateAll(MasterScore*);
      PartEdit* partEdit(int index);
      void writeSettings();
      };


} // namespace Ms
#endif

