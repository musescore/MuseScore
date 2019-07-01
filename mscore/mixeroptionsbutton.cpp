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

#include "mixeroptionsbutton.h"
#include "mixeroptions.h"
#include "mixer.h"

namespace Ms {
      MixerOptionsButton::MixerOptionsButton(QWidget* parent) : QToolButton (parent)
      {
            setCheckable(true);//TODO: in the ui file already?!
      }


      void MixerOptionsButton::setupMenu()
      {
            createMenuActionsAndGroupings();
            adjustMenuActionsInLineWithOptions();
            layoutMenuActions();
            setupSignalsAndSlots();
      }

      void MixerOptionsButton::setupSignalsAndSlots()
      {
            connect(this, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));

            connect(showDetailsToTheSide, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(showTrackColors, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(showMidiOptions, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(showMasterVolume, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));

            connect(makePanSecondarySlider, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(makeReverbSecondarySlider, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(makeChorusSecondarySlider, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));

            connect(overallVolumeOverrideMode, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(overallVolumeRatioMode, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
            connect(overallVolumeFirstMode, SIGNAL(toggled(bool)), this, SLOT(optionChangeRequest(bool)));
      }

      void MixerOptionsButton::adjustMenuActionsInLineWithOptions()
      {
            MixerOptions* options = Mixer::getOptions();

            setChecked(options->showingDetails());

            showDetailsToTheSide->setChecked(options->showDetailsOnTheSide());
            showMidiOptions->setChecked(options->showMidiOptions());
            showMasterVolume->setChecked(options->showMasterVolume());
            showTrackColors->setChecked(options->showTrackColors());

            overallVolumeOverrideMode->setChecked(options->mode() == MixerVolumeMode::Override);
            overallVolumeRatioMode->setChecked(options->mode() == MixerVolumeMode::Ratio);
            overallVolumeFirstMode->setChecked(options->mode() == MixerVolumeMode::PrimaryInstrument);

            //TODO::setup the secondary slider modes

      }

      void MixerOptionsButton::adjustOptionsInLineWithMenu()
      {
            MixerOptions* options = Mixer::getOptions();

            options->setShowingDetails(this->isChecked());

            options->setDetailsOnTheSide(showDetailsToTheSide->isChecked());
            options->setMidiOptions(showMidiOptions->isChecked());
            options->setShowMasterVolume(showMasterVolume->isChecked());
            options->setTrackColors(showTrackColors->isChecked());

            MixerVolumeMode mode;
            if (overallVolumeOverrideMode->isChecked()) {
                  mode = MixerVolumeMode::Override;
            }
            else if (overallVolumeRatioMode->isChecked()) {
                  mode = MixerVolumeMode::Ratio;
            }
            else if (overallVolumeFirstMode->isChecked()) {
                  mode = MixerVolumeMode::PrimaryInstrument;
            }
            else {
                  // something's gone awry - let's just fix it up
                  overallVolumeOverrideMode->setChecked(true);
                  overallVolumeRatioMode->setChecked(false);
                  overallVolumeFirstMode->setChecked(false);
                  mode = MixerVolumeMode::Override;
            }
            options->setMode(mode);

            //TODO::setup the secondary slider modes

      }


      void MixerOptionsButton::layoutMenuActions()
      {
            menu = new QMenu(this);
            setMenu(menu);

            menu->addSection(tr("Appearance"));
            menu->addAction(showDetailsToTheSide);
            menu->addSeparator();
            menu->addAction(showMidiOptions);
            menu->addAction(showTrackColors);
            menu->addAction(showMasterVolume);
            menu->addAction(showSecondSlider);

            menu->addSection(tr("Secondary Slider"));
            menu->addActions({makePanSecondarySlider, makeReverbSecondarySlider, makeChorusSecondarySlider});

            menu->addSection(tr("Slider Behavior"));
            menu->addAction(overallVolumeFirstMode);
            menu->addAction(overallVolumeOverrideMode);
            menu->addAction(overallVolumeRatioMode);
      }

      void MixerOptionsButton::createMenuActionsAndGroupings()
      {
            showDetailsToTheSide = new QAction(tr("Show Details to the Side"));
            showDetailsToTheSide->setCheckable(true);

            showTrackColors = new QAction(tr("Show Track Colors"));
            showTrackColors->setCheckable(true);

            showMasterVolume = new QAction(tr("Show Master Volume"));
            showMasterVolume->setCheckable(true);

            showMidiOptions = new QAction(tr("Show Midi Options (in More…)"));
            showMidiOptions->setCheckable(true);

            showSecondSlider = new QAction(tr("Show Second Slider in Mixer (not impl yet)"));
            showSecondSlider->setCheckable(true);

            overallVolumeOverrideMode = new QAction(tr("Override"));
            overallVolumeRatioMode = new QAction(tr("Relative"));
            overallVolumeFirstMode = new QAction(tr("Primary only"));

            overallVolumeOverrideMode->setCheckable(true);
            overallVolumeRatioMode->setCheckable(true);
            overallVolumeFirstMode->setCheckable(true);

            modeGroup = new QActionGroup(this);
            modeGroup->addAction(overallVolumeOverrideMode);
            modeGroup->addAction(overallVolumeRatioMode);
            modeGroup->addAction(overallVolumeFirstMode);

            makePanSecondarySlider = new QAction(tr("Pan"));
            makePanSecondarySlider->setCheckable(true);

            makeReverbSecondarySlider = new QAction(tr("Reverb"));
            makeReverbSecondarySlider->setCheckable(true);

            makeChorusSecondarySlider = new QAction(tr("Chorus"));
            makeChorusSecondarySlider->setCheckable(true);

            secondarySliderGroup = new QActionGroup(this);
            secondarySliderGroup->addAction(makePanSecondarySlider);
            secondarySliderGroup->addAction(makeReverbSecondarySlider);
            secondarySliderGroup->addAction(makeChorusSecondarySlider);
      }




      void MixerOptionsButton::optionChangeRequest(bool state)
      {
            adjustOptionsInLineWithMenu();
            mixer->updateUiOptions();
      }

     

}
