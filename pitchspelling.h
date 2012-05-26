//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: pitchspelling.h 5163 2011-12-30 09:57:08Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PITCHSPELLING_H__
#define __PITCHSPELLING_H__

class MidiNote;
class Note;
class Event;

//---------------------------------------------------------
//   pitch2tpc
//    Returns a default tpc for a given midi pitch.
//    Midi pitch 60 is middle C.
//---------------------------------------------------------

inline static int pitch2tpc(int pitch)
      {
      return (((((pitch % 12) * 7) % 12) + 5) % 12) + 9;
      }

int pitch2tpc2(int pitch, bool preferSharp);

const int INVALID_TPC = -2;

extern int pitch2tpc(int pitch, int key);

extern void spell(QList<Event>& notes, int);
extern void spell(QList<Note*>& notes);
extern int computeWindow(const QList<Note*>& notes, int start, int end);
extern int tpc(int idx, int pitch, int opt);
extern int pitch2line(int pitch);
extern QString tpc2name(int tpc, bool germanNames);
extern void tpc2name(int tpc, bool germanNames, QChar* name, int* acc);
extern int step2tpc(const QString& stepName, int alter);
extern int step2tpc(int step, int alter);
extern int tpc2pitch(int tpc);
extern int tpc2step(int tpc);
extern int tpc2alter(int tpc);
extern QString tpc2stepName(int tpc);
extern bool tpcIsValid(int val);

#endif

