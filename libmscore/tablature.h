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

//---------------------------------------------------------
//   StringData
//---------------------------------------------------------

class StringData {
      QList<int>  stringTable;
      int         _frets;

      static bool bFretting;

public:
      StringData();
      StringData(int numFrets, int numStrings, int strings[]);
      StringData(int numFrets, QList<int>& strings);
      bool        convertPitch(int pitch, int* string, int* fret) const;
      int         fret(int pitch, int string) const;
      void        fretChords(Chord * chord) const;
      int         getPitch(int string, int fret) const;
      int         strings() const         { return stringTable.size(); }
      QList<int>  stringList() const      { return stringTable; }
      QList<int>&  stringList()           { return stringTable; }
      int         frets() const           { return _frets; }
      void        setFrets(int val)       { _frets = val; }
      void        read(XmlReader&);
      void        write(Xml&) const;
//      void        readMusicXML(XmlReader& de);
      void        writeMusicXML(Xml& xml) const;
      };

extern StringData emptyStringData;

}     // namespace Ms
#endif

