//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FRET_H__
#define __FRET_H__

#include "element.h"

namespace Ms {

class StringData;
class Chord;
class Harmony;

//---------------------------------------------------------
//   @@ FretDiagram
//---------------------------------------------------------

class FretDiagram : public Element {
      Q_OBJECT

      int _strings;
      int maxStrings;
      int _frets;
      int _fretOffset;
      int _maxFrets;
      char* _dots;
      char* _marker;
      char* _fingering;
      Harmony* _harmony;

      qreal lw1;
      qreal lw2;             // top line
      qreal stringDist;
      qreal fretDist;
      QFont font;

   public:
      FretDiagram(Score* s);
      FretDiagram(const FretDiagram&);
      ~FretDiagram();
      virtual void draw(QPainter*) const;
      virtual FretDiagram* clone() const { return new FretDiagram(*this); }

      virtual ElementType type() const   { return FRET_DIAGRAM; }
      virtual void layout();
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual QLineF dragAnchor() const;
      virtual QPointF pagePos() const;

      // read / write MusicXML
      void readMusicXML(XmlReader& de);
      void writeMusicXML(Xml& xml) const;

      int strings() const    { return _strings; }
      int frets()   const    { return _frets; }
      void setStrings(int n);
      void setFrets(int n)   { _frets = n; }
      void setDot(int string, int fret);
      void setMarker(int string, int marker);
      void setFingering(int string, int finger);
      int fretOffset() const      { return _fretOffset; }
      void setFretOffset(int val) { _fretOffset = val;  }
      int maxFrets() const        { return _maxFrets;   }
      void setMaxFrets(int val)   { _maxFrets = val;    }

      char* dots()      { return _dots;   }
      char* marker()    { return _marker; }
      Harmony* harmony() const { return _harmony; }

      char* fingering() { return _fingering; }
      void init(Ms::StringData *, Chord*);

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      };


}     // namespace Ms
#endif
