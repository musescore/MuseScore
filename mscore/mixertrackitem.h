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

#include "libmscore/instrument.h"
#include <memory>

namespace Ms {

class Part;
class Instrument;
class Channel;
class MidiMapping;
class MixerTrackItem;

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

      bool isPart();
      QList<Channel*> secondaryPlaybackChannels();
      QList<Channel*> playbackChannels(Part* part);

public:
      MixerTrackItem(TrackType trackType, Part* part, Instrument* _instr, Channel* _chan);

      TrackType trackType() { return _trackType; }
      Part* part() { return _part; }
      Instrument* instrument() { return _instrument; }
      Channel* channel() { return _channel; }
      MidiMapping *midiMap();
      int color();

      void setColor(int valueRgb);
      void setVolume(char value);
      void setPan(char value);
      void setChorus(char value);
      void setReverb(char value);

      template <class ChannelWriter, class ChannelReader>
      void adjustValue(int newValue, ChannelReader reader, ChannelWriter writer);

      void setMute(bool value);
      void setSolo(bool value);

      char getVolume();
      bool getMute();
      bool getSolo();
      char getPan();
      };
}

#endif // __MIXERTRACKITEM_H__
