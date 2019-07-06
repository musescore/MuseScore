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

bool MixerOptions::secondaryModeOn()
      {
      return _secondaryModeLock ? !_secondaryMode : _secondaryMode;
      }

      void MixerOptions::readSettings() {

            QSettings settings("MuseScore","Mixer");
            settings.beginGroup("MixerOptions");
            _showTrackColors = settings.value("showTrackColors", true).toBool();
            _detailsOnTheSide = settings.value("detailsOnTheSide", false).toBool();
            _showMidiOptions = settings.value("showMidiOptions", true).toBool();
            _showingDetails = settings.value("showingDetails", true).toBool();
            _showMasterVolume = settings.value("showMasterVolume", true).toBool();
            _mode = static_cast<MixerVolumeMode>(settings.value("sliderMode", static_cast<int>(MixerVolumeMode::Ratio)).toInt());
            _secondarySlider = static_cast<MixerSecondarySlider>(settings.value("secondarySlider", static_cast<int>(MixerSecondarySlider::Pan)).toInt());
            settings.endGroup();
      }

      void MixerOptions::writeSettings() {
            QSettings settings("MuseScore","Mixer");
            settings.beginGroup("MixerOptions");
            settings.setValue("showTrackColors", _showTrackColors);
            settings.setValue("detailsOnTheSide", _detailsOnTheSide);
            settings.setValue("showMidiOptions", _showMidiOptions);
            settings.setValue("showingDetails", _showingDetails);
            settings.setValue("showMasterVolume", _showMasterVolume);
            settings.setValue("sliderMode", static_cast<int>(_mode));
            settings.setValue("secondarySlider", static_cast<int>(_secondarySlider));
            settings.endGroup();
      }

} // namespace Ms
