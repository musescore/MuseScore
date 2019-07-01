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

#include "mixeroptions.h"

namespace Ms {
      MixerOptions::MixerOptions()
      {
            readSettings();
      }

      void MixerOptions::readSettings() {

            QSettings settings("MuseScore","Mixer");
            settings.beginGroup("MixerOptions");
            _showTrackColors = settings.value("showTrackColors", true).toBool();
            _detailsOnTheSide = settings.value("detailsOnTheSide", false).toBool();
            _showMidiOptions = settings.value("showMidiOptions", true).toBool();
            _showingDetails = settings.value("showingDetails", true).toBool();
            _showMasterVolume = settings.value("showMasterVolume", true).toBool();
            int rawMode = settings.value("sliderMode", 1).toInt();
            settings.endGroup();

            // hack because C++ documentation is horrible and I want it done
            // fix up later
            MixerVolumeMode convertEnum;

            switch (rawMode) {
                  case 1:
                        convertEnum = MixerVolumeMode::Override;
                        break;
                  case 2:
                        convertEnum = MixerVolumeMode::Ratio;
                        break;
                  case 3:
                        convertEnum = MixerVolumeMode::PrimaryInstrument;
                        break;
                  default:
                        convertEnum = MixerVolumeMode::Override;
            }

            _mode = convertEnum;

      }

      void MixerOptions::writeSettings() {
            QSettings settings("MuseScore","Mixer");
            settings.beginGroup("MixerOptions");
            settings.setValue("showTrackColors", _showTrackColors);
            settings.setValue("detailsOnTheSide", _detailsOnTheSide);
            settings.setValue("showMidiOptions", _showMidiOptions);
            settings.setValue("showingDetails", _showingDetails);
            settings.setValue("showMasterVolume", _showMasterVolume);

            // hack because C++ documentation is horrible and I want it done
            // fix up later
            int convertEnum = 0;
            switch (_mode) {
                  case MixerVolumeMode::Override:
                        convertEnum = 1;
                        break;
                  case MixerVolumeMode::Ratio:
                        convertEnum = 2;
                        break;
                  case MixerVolumeMode::PrimaryInstrument:
                        convertEnum = 3;
                        break;
            }

            settings.setValue("sliderMode", convertEnum);
            settings.endGroup();
      }

} // namespace Ms
