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

#include "ui_parteditbase.h"
#include "ui_mixer.h"
#include "libmscore/instrument.h"
#include "enableplayforwidget.h"
#include "mixertrackgroup.h"
#include <QWidget>
#include <QScrollArea>
#include <QList>

namespace Ms {

class Score;
class Channel;
class Part;
class PartEdit;
class MixerDetails;
class MixerTrack;
struct MidiMapping;

double volumeToUserRange(char v);
double panToUserRange(char v);
double chorusToUserRange(char v);
double reverbToUserRange(char v);

//0 to 100
char userRangeToVolume(double v);
//-180 to 180
char userRangeToPan(double v);
//0 to 100
char userRangeToChorus(double v);
//0 to 100
char userRangeToReverb(double v);


//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

class Mixer : public QWidget, public Ui::Mixer, public MixerTrackGroup
      {
      Q_OBJECT

      MasterScore* _score;
      QScrollArea* sa;
      QHBoxLayout* trackAreaLayout;
      EnablePlayForWidget* enablePlay;

      MixerDetails* mixerDetails;

      bool showExpanded;
      QSet<Part*> expandedParts;
      QWidget* trackHolder;
      QList<MixerTrack*> trackList;

      virtual void closeEvent(QCloseEvent*) override;
      virtual void showEvent(QShowEvent*) override;
      virtual bool eventFilter(QObject*, QEvent*) override;
      virtual void keyPressEvent(QKeyEvent*) override;
      void readSettings();

   public slots:
      void updateTracks();
      void midiPrefsChanged(bool showMidiControls);
      void masterVolumeChanged(double val);
      void synthGainChanged(float val);


   signals:
      void closed(bool);

   protected:
      virtual void changeEvent(QEvent *event) override;
      void retranslate(bool firstTime = false);

   public:
      Mixer(QWidget* parent);
      void setScore(MasterScore*);
      PartEdit* getPartAtIndex(int index);
      void writeSettings();
      void expandToggled(Part* part, bool expanded) override;
      void notifyTrackSelected(MixerTrack* track) override;
      };


} // namespace Ms
#endif

