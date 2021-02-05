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

#ifndef __TABLATURE_H__
#define __TABLATURE_H__

#include "xml.h"

namespace Ms {

class Chord;
class Note;

//---------------------------------------------------------
//   StringData
//---------------------------------------------------------

// defines the string of an instrument
struct instrString {
      instrString(int p=0, bool o=false, int s=0) : pitch(p), open(o), startFret(s) {};
      int   pitch;      // the pitch of the string
      bool  open;       // true: string is open | false: string is fretted
      int   startFret;  // banjo 5th string starts on 5th fret

      bool operator==(const instrString& d) const { return d.pitch == pitch && d.open == open; }
      };

class StringData {
//      QList<int>  stringTable { 40, 45, 50, 55, 59, 64 };   // guitar is default
//      int         _frets = 19;
      QList<instrString>  stringTable {  };                   // no strings by default
      int         _frets = 0;

      static bool bFretting;

      bool        convertPitch(int pitch, int pitchOffset, int* string, int* fret) const;
      int         fret(int pitch, int string, int pitchOffset) const;
      int         getPitch(int string, int fret, int pitchOffset) const;
      void        sortChordNotes(QMap<int, Note *>& sortedNotes, const Chord* chord, int pitchOffset, int* count) const;

public:
      StringData() {}
      StringData(int numFrets, int numStrings, int strings[]);
      StringData(int numFrets, QList<instrString>& strings);
      void        set(const StringData& src);
      bool        convertPitch(int pitch, Staff* staff, const Fraction& tick, int* string, int* fret) const;
      int         fret(int pitch, int string, Staff* staff, const Fraction& tick) const;
      void        fretChords(Chord * chord) const;
      int         getPitch(int string, int fret, Staff* staff, const Fraction& tick) const;
      static int  pitchOffsetAt(Staff* staff, const Fraction& tick);
      int         strings() const                   { return stringTable.size(); }
      int         frettedStrings() const;
      const QList<instrString>&  stringList() const { return stringTable; }
      QList<instrString>&  stringList()         { return stringTable; }
      int         frets() const                 { return _frets; }
      void        setFrets(int val)             { _frets = val; }
      void        read(XmlReader&);
      void        write(XmlWriter&) const;
      void        writeMusicXML(XmlWriter& xml) const;
      bool operator==(const StringData& d) const { return d._frets == _frets && d.stringTable == stringTable; }
      void        configBanjo5thString();
      int         adjustBanjo5thFret(int fret) const;
      bool        isFiveStringBanjo() const;
      };

}     // namespace Ms
#endif

