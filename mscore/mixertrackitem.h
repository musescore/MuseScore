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

      void setMute(bool value);
      void setSolo(bool value);

      char getVolume();
      char getChorus();
      char getReverb();

      bool getMute();
      bool getSolo();
      char getPan();

      };


/*
 MixerTreeWidgetItem
 
 subclass of QTreeWidget that:
 - will construct a MixerTreeWidgetItem from a Part (including any children)
 - host a MixerTrackItem for processing user changes and updating controls
   when changes are signalled from outwith the mixer
*/
class MixerTrackChannel;
      
class MixerTreeWidgetItem : public QTreeWidgetItem
      {
      MixerTrackItem* _mixerTrackItem;
      MixerTrackChannel* _mixerTrackChannel;

   public:
      MixerTreeWidgetItem(Part* part, Score* score, QTreeWidget* parent);
      MixerTreeWidgetItem(Channel* channel, Instrument* instrument, Part* part);
      ~MixerTreeWidgetItem();

      MixerTrackItem* mixerTrackItem() { return _mixerTrackItem; };
      MixerTrackChannel* mixerTrackChannel();
      };


}
#endif // __MIXERTRACKITEM_H__
