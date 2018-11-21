//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mixer.h 4388 2011-06-18 13:17:58Z wschweer $
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
#include <QObject>

namespace Ms {

class Part;
class Instrument;
class Channel;
struct MidiMapping;
class MixerTrackItem;


//---------------------------------------------------------
//   MixerTrackItem
//---------------------------------------------------------

class MixerTrackItem : public QObject
      {
      Q_OBJECT

public:
      enum class TrackType { PART, CHANNEL };
      Q_ENUM(TrackType)

private:
      TrackType _trackType;
      Part* _part;

      Instrument* _instr;
      Channel* _chan;

public:
      MixerTrackItem(TrackType tt, Part* part, Instrument* _instr, Channel* _chan, QObject* parent = nullptr);

      TrackType trackType() { return _trackType; }
      Part* part() { return _part; }
      Instrument* instrument() { return _instr; }
      Channel* chan() { return _chan; }
      Channel* focusedChan();
      MidiMapping *midiMap();
      int color();

public slots:
      void setColor(int valueRgb);
      void setVolume(char value);
      void setPan(char value);
      void setChorus(char value);
      void setReverb(char value);

      void setMute(bool value);
      void setSolo(bool value);
      };
}

#endif // __MIXERTRACKITEM_H__
