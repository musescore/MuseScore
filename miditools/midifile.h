//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

#include <map>
#include <list>
#include <QString>
#include <QIODevice>

#include "midievent.h"
#include "tempomap.h"

class MidiFile;

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

class MidiTrack {
      MidiFile* mf;
      std::multimap<int, MidiEvent> _events;

   public:
      MidiTrack(MidiFile*);
      ~MidiTrack();

      const std::multimap<int, MidiEvent>& events() const { return _events;     }
      std::multimap<int, MidiEvent>& events() { return _events;     }
      };

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile {
      TempoMap _tempoMap;
      QIODevice* fp;
      std::list<MidiTrack*> _tracks;
      int _division;
      int _format;               ///< midi file format (0-2)

      int status;                ///< running status
      int sstatus;               ///< running status (not reset after meta or sysex events)
      int click;                 ///< current tick position in file
      qint64 curPos;             ///< current file byte position

   protected:
      // read
      void read(void*, qint64);
      int getvl();
      int readShort();
      int readLong();
      int readEvent(MidiEvent*);
      bool readTrack();
      void skip(qint64);

      // write
      bool write(const void*, qint64);
      void writeShort(int);
      void writeLong(int);
      bool writeTrack(const MidiTrack*);
      void writeEvent(const MidiEvent&);
      void putvl(unsigned);
      void put(unsigned char c) { write(&c, 1); }
      void writeStatus(MidiEventType, int channel);

      void resetRunningStatus() { status = -1; }

   public:
      MidiFile();
      bool read(const QString& path);
      bool read(QIODevice*);

      bool write(const QString& path);
      bool write(QIODevice*);

      const std::list<MidiTrack*>& tracks() const { return _tracks;   }
      std::list<MidiTrack*>& tracks()             { return _tracks;   }

      int format() const                { return _format;   }
      void setFormat(int fmt)           { _format = fmt;    }
      int division() const              { return _division; }
      void setDivision(int val)         { _division = val;  }
      const TempoMap& tempoMap() const  { return _tempoMap; }
      };

#endif

