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

#ifndef __MIXEROPTIONSBUTTON_H__
#define __MIXEROPTIONSBUTTON_H__

namespace Ms {

class Mixer;
      
class MixerOptionsButton : public QToolButton
      {
      Q_OBJECT

      QMenu* menu;

      QAction* showDetailsToTheSide;
      QAction* showMidiOptions;
      QAction* showTrackColors;
      QAction* showMasterVolume;
      QAction* showSecondSlider;

      QAction* overallVolumeOverrideMode;
      QAction* overallVolumeRatioMode;
      QAction* overallVolumeFirstMode;
      QActionGroup* modeGroup;

      QAction* makePanSecondarySlider;
      QAction* makeReverbSecondarySlider;
      QAction* makeChorusSecondarySlider;
      QActionGroup* secondarySliderGroup;

      Mixer* mixer;
      void setupMenu();
      void createMenuActionsAndGroupings();
      void adjustMenuActionsInLineWithOptions();
      void adjustOptionsInLineWithMenu();
      void layoutMenuActions();
      void setupSignalsAndSlots();

      public slots:
      void optionChangeRequest(bool);

public:
      MixerOptionsButton(QWidget *parent = nullptr);
      void setTarget(Mixer* target) { mixer = target; setupMenu(); }
      };

} // Ms namespace
#endif /* __MIXEROPTIONSBUTTON_H__ */
