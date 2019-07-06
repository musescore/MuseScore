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

#ifndef __MIXER_H__
#define __MIXER_H__

#include "ui_mixer.h"

namespace Ms {

class Score;
class Channel;
class Part;

class EnablePlayForWidget;
class MixerDetails;
class MidiMapping;
class MixerKeyboardControlFilter;
class MixerContextMenu;
class MixerOptions;
class MixerMasterChannel;

//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------


class Mixer : public QDockWidget, public Ui::Mixer
      {
      Q_OBJECT

      public:
            enum class NudgeDirection : int { Up, Down };

      Score* _score = nullptr;                        // playback score
      Score* _activeScore = nullptr;                  // may be a _score itself or its excerpt;
      QGridLayout* gridLayout;                        // main layout - used to show/hide & position details panel
      MixerMasterChannel* masterChannelWidget;        // master volume + play / loop widget

      static MixerOptions* options;                   // UI options, e.g. show/hide track colors, slider modes

      EnablePlayForWidget* enablePlay;

      int savedSelectionTopLevelIndex;
      int savedSelectionChildIndex;

      void setupSlotsAndSignals();
      void setupAdditionalUi();
      void disableMixer();                            // gray out everything when no score or part is open
      void showDetails(bool);
      void setPlaybackScore(Score*);

      void enterSecondarySliderMode(bool enter);
      void updateMixerTreeHeaders();
      void updateTreeSlidersAppearance();

      QTimer* shiftKeyMonitorTimer;
      MixerKeyboardControlFilter* keyboardFilter;     // process key presses for the mixer AND the details panel
      virtual void closeEvent(QCloseEvent*) override;
      virtual void showEvent(QShowEvent*) override;
      virtual void hideEvent(QHideEvent*) override;
      virtual bool eventFilter(QObject*, QEvent*) override;
      virtual void keyPressEvent(QKeyEvent*) override;

   private slots:
      void on_partOnlyCheckBox_toggled(bool checked);
      void itemCollapsedOrExpanded(QTreeWidgetItem*);
      void shiftKeyMonitor();

   public slots:
      void updateTracks();
      void midiPrefsChanged(bool showMidiControls);
      void masterVolumeChanged(double val);
      void currentMixerTreeItemChanged();
      void synthGainChanged(float val);

      void saveTreeSelection();
      void restoreTreeSelection();
      void adjustHeaderWidths();
            
   signals:
      void closed(bool);

   protected:
      virtual void changeEvent(QEvent *event) override;
      void retranslate(bool firstTime = false);

   public:
      Mixer(QWidget* parent);
      void setScore(Score*);

      MixerDetails* mixerDetails;                                 // TODO: mixerDetails - does it NEED to be public?
      void updateUiOptions();
      void nudgeSecondarySlider(NudgeDirection direction);
      void nudgeMainSlider(NudgeDirection direction);
      int nudge(int currentValue, NudgeDirection direction, int lowerLimit, int upperLimit);

      static MixerOptions* getOptions() { return options; };
            
      };

//MARK:- Class MixkerKeyboardControlFilter

class MixerKeyboardControlFilter : public QObject
      {
      Q_OBJECT
      Mixer* mixer;
   protected:
      bool eventFilter(QObject *obj, QEvent *event) override;
      
   public:
      MixerKeyboardControlFilter(Mixer* mixer);
      };


} // namespace Ms
#endif

