//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: midifile.h 4720 2011-08-31 18:10:05Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include "libmscore/sig.h"
#include "libmscore/event.h"

const int MIDI_CHANNEL = 16;

//---------------------------------------------------------
//   MidiType
//---------------------------------------------------------

enum MidiType {
      MT_UNKNOWN = 0, MT_GM = 1, MT_GS = 2, MT_XG = 4
      };

class MidiFile;
class Xml;
class Staff;
class Score;

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

class MidiTrack {
      MidiFile* mf;
      EventList _events;
      int _outChannel;
      int _outPort;
      QString _name;
      QString _comment;
      bool _drumTrack;
      bool _hasKey;
      int _staffIdx;
      Staff* _staff;
      int _program;

   protected:
      void readXml(QDomElement);

   public:
      int maxPitch;
      int minPitch;
      int medPitch;

      MidiTrack(MidiFile*);
      ~MidiTrack();

      bool empty() const;
      const EventList& events() const   { return _events;     }
      EventList& events()               { return _events;     }
      int outChannel() const            { return _outChannel; }
      void setOutChannel(int n);
      int outPort() const               { return _outPort;    }
      void setOutPort(int n)            { _outPort = n;       }
      QString name() const              { return _name;       }
      void setName(const QString& s)    { _name = s;          }
      QString comment() const           { return _comment;    }
      void setComment(const QString& s) { _comment = s;       }
      void insert(const Event& e)       { _events.insert(e);  }
      void append(const Event& e)       { _events.append(e);  }

      void addCtrl(int tick, int channel, int type, int value);

      void mergeNoteOnOff();
      void cleanup();
      inline int division() const;
      void changeDivision(int newDivision);
      void move(int ticks);
      bool isDrumTrack() const;
      void extractTimeSig(TimeSigMap* sig);
      void quantize(int startTick, int endTick, EventList* dst);
      int getInitProgram();
      void findChords();
      int separateVoices(int);
      void setHasKey(bool val) { _hasKey = val;    }
      bool hasKey() const      { return _hasKey;   }
      int staffIdx() const     { return _staffIdx; }
      void setStaffIdx(int v)  { _staffIdx = v;    }
      Staff* staff() const     { return _staff;    }
      void setStaff(Staff* st) { _staff = st;      }
      int program() const      { return _program;  }
      void setProgram(int v)   { _program = v;     }

      friend class MidiFile;
      };

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile {
      TimeSigMap _siglist;
      QIODevice* fp;
      QList<MidiTrack*> _tracks;
      int _division;
      int _format;               ///< midi file format (0-2)
      bool _noRunningStatus;     ///< do not use running status on output
      MidiType _midiType;

      // values used during read()
      int status;                ///< running status
      int sstatus;               ///< running status (not reset after meta or sysex events)
      int click;                 ///< current tick position in file
      qint64 curPos;             ///< current file byte position
      int _shortestNote;

      void writeEvent(const Event& event);

   protected:
      // write
      bool write(const void*, qint64);
      void writeShort(int);
      void writeLong(int);
      bool writeTrack(const MidiTrack*);
      void putvl(unsigned);
      void put(unsigned char c) { write(&c, 1); }
      void writeStatus(int type, int channel);

      // read
      void read(void*, qint64);
      int getvl();
      int readShort();
      int readLong();
      bool readEvent(Event*);
      bool readTrack();
      void skip(qint64);

      void resetRunningStatus() { status = -1; }

   public:
      MidiFile();
      bool read(QIODevice*);
      bool write(QIODevice*);
      void readXml(QDomElement);

      QList<MidiTrack*>* tracks()   { return &_tracks;  }
      MidiType midiType() const     { return _midiType; }
      void setMidiType(MidiType mt) { _midiType = mt;   }
      int format() const            { return _format;   }
      void setFormat(int fmt)       { _format = fmt;    }
      int division() const          { return _division; }
      void setDivision(int val)     { _division = val;  }
      void changeDivision(int val);
      void process1();
      void sortTracks();
      void separateChannel();
      void move(int ticks);
      TimeSigMap siglist() const     { return _siglist;         }
      int noRunningStatus() const     { return _noRunningStatus; }
      void setNoRunningStatus(bool v) { _noRunningStatus = v;    }
      void processMeta(Score*, MidiTrack* track, const Event& e);
      void setShortestNote(int v)     { _shortestNote = v;    }
      int shortestNote() const        { return _shortestNote; }
      void convertTrack(Score* score, MidiTrack* midiTrack);

      friend class EventData;
      friend class MidiTrack;
      };

int MidiTrack::division() const { return mf->division(); }

#endif

