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


#ifndef __MIXEROPTIONS_H__
#define __MIXEROPTIONS_H__

namespace Ms {

class MixerOptions
{
   public:
      enum class MixerSecondarySlider : int { Pan = 1, Reverb, Chorus };
      enum class MixerVolumeMode : int { Ratio = 1, Override, PrimaryInstrument };

   private:

      bool _showTrackColors;
      bool _detailsOnTheSide;
      bool _showMidiOptions;
      bool _showingDetails;
      bool _showMasterVolume;
      MixerVolumeMode _mode;
      MixerSecondarySlider _secondarySlider;
      bool _secondaryMode = false;
      bool _secondaryModeLock = false;

   public:
      MixerOptions();
      bool showTrackColors() { return _showTrackColors; };
      bool showDetailsOnTheSide() { return _detailsOnTheSide; };
      bool showMidiOptions() { return _showMidiOptions; };
      bool showingDetails() { return _showingDetails; };
      bool showMasterVolume() { return _showMasterVolume; };
      bool secondaryModeLock() {return _secondaryModeLock; };
      MixerVolumeMode mode() { return _mode; };
      MixerSecondarySlider secondarySlider() { return _secondarySlider; };

      void setTrackColors(bool show) { _showTrackColors = show; writeSettings(); };
      void setDetailsOnTheSide(bool show) { _detailsOnTheSide = show; writeSettings(); };
      void setMidiOptions(bool show) { _showMidiOptions = show; writeSettings(); };
      void setShowingDetails(bool show) { _showingDetails = show; writeSettings(); }
      void setShowMasterVolume(bool show) { _showMasterVolume = show; writeSettings(); };
      void setMode(MixerVolumeMode mode) { _mode = mode; writeSettings(); };
      void setSecondarySlider(MixerSecondarySlider secondary) { _secondarySlider = secondary; }
      void setSecondaryModeLock(bool lock) { _secondaryModeLock = lock; };
      bool secondaryModeOn();
      void setSecondaryModeOn(bool on) {_secondaryMode = on;};
      void readSettings();
      void writeSettings();

};


} // namespace Ms
#endif /* __MIXEROPTIONS_H__ */
