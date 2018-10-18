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
///    Fretboard diagram
//
//   @P userMag    qreal
//   @P strings    int  number of strings
//   @P frets      int  number of frets
//   @P barre      int  barre
//   @P fretOffset int
//---------------------------------------------------------

class FretDiagram final : public Element {
      int _strings;
      int maxStrings     { 0 };
      int _frets;
      int _fretOffset    { 0  };
      int _maxFrets      { 24 };
      int _barre         { 0 };

      char* _dots        { 0 };
      char* _marker      { 0 };
      char* _fingering   { 0 };

      Harmony* _harmony  { 0 };

      qreal lw1;
      qreal lw2;             // top line
      qreal stringDist;
      qreal fretDist;
      QFont font;
      qreal _userMag     { 1.0   };             // allowed 0.1 - 10.0
      int _numPos;

   public:
      FretDiagram(Score* s);
      FretDiagram(const FretDiagram&);
      ~FretDiagram();
      virtual void draw(QPainter*) const override;
      virtual FretDiagram* clone() const override { return new FretDiagram(*this); }

      Segment* segment() { return toSegment(parent()); }

      static FretDiagram* fromString(Score* score, const QString &s);

      virtual ElementType type() const override { return ElementType::FRET_DIAGRAM; }
      virtual void layout() override;
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual QLineF dragAnchor() const override;
      virtual QPointF pagePos() const override;

      // read / write MusicXML
      void readMusicXML(XmlReader& de);
      void writeMusicXML(XmlWriter& xml) const;

      int strings() const    { return _strings; }
      int frets()   const    { return _frets; }
      void setStrings(int n);
      void setFrets(int n)        { _frets = n; }
      void setDot(int string, int fret);
      void setBarre(int fret)     { _barre = fret; }
      void setMarker(int string, int marker);
      void setFingering(int string, int finger);
      int fretOffset() const      { return _fretOffset; }
      void setFretOffset(int val) { _fretOffset = val;  }
      int maxFrets() const        { return _maxFrets;   }
      void setMaxFrets(int val)   { _maxFrets = val;    }

      char dot(int s) const       { return _dots      ? _dots[s]      : 0; }
      char marker(int s) const    { return _marker    ? _marker[s]    : 0; }
      char fingering(int s) const { return _fingering ? _fingering[s] : 0; }
      int barre() const           { return _barre; }

      Harmony* harmony() const { return _harmony; }

      void init(Ms::StringData *, Chord*);

      virtual void add(Element*) override;
      virtual void remove(Element*) override;

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      qreal userMag() const         { return _userMag;   }
      void setUserMag(qreal m)      { _userMag = m;      }
      };


}     // namespace Ms
#endif
