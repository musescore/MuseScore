//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BB_H__
#define __BB_H__

#include "midi/midifile.h"

namespace Ms {

const int MAX_BARS = 255;

class BBFile;
struct MNote;
class Score;

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

class BBTrack {
      BBFile* bb;
      EventList _events;
      int _outChannel;
      bool _drumTrack;

      void quantize(int startTick, int endTick, EventList* dst);

   public:
      BBTrack(BBFile*);
      ~BBTrack();
      bool empty() const;
      const EventList events() const    { return _events;     }
      EventList& events()               { return _events;     }
      int outChannel() const            { return _outChannel; }
      void setOutChannel(int n)         { _outChannel = n;    }
      void insert(const Event& e)       { _events.insert(e);  }
      void append(const Event& e)       { _events.append(e);  }

      void findChords();
      int separateVoices(int);
      void cleanup();

      friend class BBFile;
      };

//---------------------------------------------------------
//   BBChord
//---------------------------------------------------------

struct BBChord {
      int beat;
      unsigned char bass;
      unsigned char root;
      unsigned char extension;
      BBChord() {
            beat = 0;
            bass = 0;
            root = 0;
            extension = 0;
            }
      };

//---------------------------------------------------------
//   BBStyle
//---------------------------------------------------------

struct BBStyle {
      int timesigZ, timesigN;
      };

//---------------------------------------------------------
//   BBStyle
//---------------------------------------------------------

static const BBStyle styles[] = {
      {  4, 4 },   // Jazz Swing
      { 12, 8 },   // Country 12/8
      {  4, 4 },   // Country 4/4
      {  4, 4 },   // Bossa Nova
      {  4, 4 },   // Ethnic
      {  4, 4 },   // Blues Shuffle
      {  4, 4 },   // Blues Straight
      {  3, 4 },   // Waltz
      {  4, 4 },   // Pop Ballad
      {  4, 4 },   // should be Rock Shuffle
      {  4, 4 },   // lite Rock
      {  4, 4 },   // medium Rock
      {  4, 4 },   // Heavy Rock
      {  4, 4 },   // Miami Rock
      {  4, 4 },   // Milly Pop
      {  4, 4 },   // Funk
      {  3, 4 },   // Jazz Waltz
      {  4, 4 },   // Rhumba
      {  4, 4 },   // Cha Cha
      {  4, 4 },   // Bouncy
      {  4, 4 },   // Irish
      { 12, 8 },   // Pop Ballad 12/8
      { 12, 8 },   // Country12/8 old
      {  4, 4 },   // Reggae
      };

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

class BBFile {
      QString _path;
      unsigned char _version;
      char* _title;
      int _style, _key, _bpm;

      unsigned char _barType[MAX_BARS];
      QList<BBChord> _chords;

      int _startChorus;
      int _endChorus;
      int _repeats;
#if 0 // yet(?) unused
      int _flags;
#endif
      char* _styleName;
      QList<BBTrack*> _tracks;
      int _measures;
      TimeSigMap _siglist;

      QByteArray ba;
      const unsigned char* a;
      int size;
      int bbDivision;

      int timesigZ() { return styles[_style].timesigZ; }
      int timesigN() { return styles[_style].timesigN; }
      int processPendingNotes(Score*, QList<MNote*>* notes, int, int);

   public:
      BBFile();
      ~BBFile();
      bool read(const QString&);
      QList<BBTrack*>* tracks()      { return &_tracks;     }
      int measures() const           { return _measures;    }
      const char* title() const      { return _title;       }
      TimeSigMap siglist() const { return _siglist;     }
      QList<BBChord> chords()        { return _chords;      }
      int startChorus() const        { return _startChorus; }
      int endChorus() const          { return _endChorus;   }
      int repeats() const            { return _repeats;     }
      int key() const                { return _key;         }
      void convertTrack(Score* score, BBTrack* track, int staffIdx);
      };


} // namespace Ms
#endif

