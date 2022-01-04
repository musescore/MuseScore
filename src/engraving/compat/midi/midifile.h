/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include "libmscore/sig.h"
#include "event.h"

namespace Ms {
const int MIDI_CHANNEL = 16;

//---------------------------------------------------------
//   MidiType
//---------------------------------------------------------

enum class MidiType : char {
    UNKNOWN = 0, GM = 1, GS = 2, XG = 4
};

class MidiFile;

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

class MidiTrack
{
    std::multimap<int, MidiEvent> _events;
    int _outChannel;
    int _outPort;
    bool _drumTrack;

public:
    MidiTrack();
    ~MidiTrack();

    bool empty() const;
    const std::multimap<int, MidiEvent>& events() const { return _events; }
    std::multimap<int, MidiEvent>& events() { return _events; }

    int outChannel() const { return _outChannel; }
    void setOutChannel(int n);
    int outPort() const { return _outPort; }
    void setOutPort(int n) { _outPort = n; }

    bool drumTrack() const { return _drumTrack; }

    void insert(int tick, const MidiEvent&);
    void mergeNoteOnOffAndFindMidiType(MidiType* mt);
};

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile
{
    QIODevice* fp;
    QList<MidiTrack> _tracks;
    int _division;
    bool _isDivisionInTps;         ///< ticks per second, alternative - ticks per beat
    int _format;                 ///< midi file format (0-2)
    bool _noRunningStatus;       ///< do not use running status on output
    MidiType _midiType;

    // values used during read()
    int status;                  ///< running status
    int sstatus;                 ///< running status (not reset after meta or sysex events)
    int click;                   ///< current tick position in file
    qint64 curPos;               ///< current file byte position

    void writeEvent(const MidiEvent& event);

protected:
    // write
    bool write(const void*, qint64);
    void writeShort(int);
    void writeLong(int);
    bool writeTrack(const MidiTrack&);
    void putvl(unsigned);
    void put(unsigned char c) { write(&c, 1); }
    void writeStatus(int type, int channel);

    // read
    void read(void*, qint64);
    int getvl();
    int readShort();
    int readLong();
    bool readEvent(MidiEvent*);
    bool readTrack();
    void skip(qint64);

    void resetRunningStatus() { status = -1; }

public:
    MidiFile();
    bool read(QIODevice*);
    bool write(QIODevice*);

    QList<MidiTrack>& tracks() { return _tracks; }
    const QList<MidiTrack>& tracks() const { return _tracks; }

    MidiType midiType() const { return _midiType; }
    void setMidiType(MidiType mt) { _midiType = mt; }

    int format() const { return _format; }
    void setFormat(int fmt) { _format = fmt; }

    int division() const { return _division; }
    bool isDivisionInTps() const { return _isDivisionInTps; }
    void setDivision(int val) { _division = val; }
    void separateChannel();
};
}
#endif
