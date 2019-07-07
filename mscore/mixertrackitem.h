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

#ifndef __MIXERTRACKITEM_H__
#define __MIXERTRACKITEM_H__

namespace Ms {

class Score;
class Part;
class Instrument;
class Channel;
class MidiMapping;
class MixerTrackItem;
struct MidiPatch;

//---------------------------------------------------------
//   MixerTrackItem
//---------------------------------------------------------

class MixerTrackItem
      {
public:
      enum class TrackType { PART, CHANNEL };

private:
      TrackType _trackType;
      Part* _part;

      Instrument* _instrument;
      Channel* _channel;

      Channel* playbackChannel(const Channel* channel);

      QList<Channel*> secondaryPlaybackChannels();
      QList<Channel*> playbackChannels(Part* part);
      QList<Channel*> playbackChannels();

      template <class ChannelWriter, class ChannelReader>
      int adjustValue(int proposedValue, ChannelReader reader, ChannelWriter writer);
      template <class ChannelWriter, class ChannelReader>
      int relativeAdjust(int mainSliderDelta, ChannelReader reader, ChannelWriter writer);
      const int panAdjustment();

      bool isCurrentPatch(const MidiPatch* patch);
      QString adjustedPatchName(const MidiPatch* patch, std::vector<QString> usedNames);

public:
      MixerTrackItem(TrackType trackType, Part* part, Instrument* _instr, Channel* _chan);

      MixerTrackItem(Part* part, Score* score);
      TrackType trackType() { return _trackType; }
      Part* part() { return _part; }
      Instrument* instrument() { return _instrument; }
      Channel* channel() { return _channel; }
      MidiMapping *midiMap();
      int color();
      bool isPart();

      QString detailedToolTip();

      void setColor(int valueRgb);

      int setVolume(int value);    // returns the value actually used (which may differ from value passed)
      int setPan(int value);       // returns the value actually used (which may differ from value passed)
      int setChorus(int value);    // returns the value actually used (which may differ from value passed)
      int setReverb(int value);    // returns the value actually used (which may differ from value passed)

      void setName(QString string);
      QString getName();

      QString getChannelName();    // no setting - the user can't alter this

      void setMute(bool value);
      void setSolo(bool value);

      char getVolume();
      char getChorus();
      char getReverb();

      bool getMute();
      bool getSolo();
      char getPan();

      void setMidiChannelAndPort(int channel, int port);
      int getMidiChannel();
      int getMidiPort();

      void toggleMutedVoice(int staffIndex, int voiceIndex, bool shouldMute);
      QList<QList<bool>> getMutedVoices();

      void populatePatchCombo(QComboBox* patchCombo);
      void changePatch(int itemIndex, QComboBox* patchCombo);
      bool getUseDrumset();
      void setUseDrumset(bool useDrumset);
            
      };

}
#endif // __MIXERTRACKITEM_H__
