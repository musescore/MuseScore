//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: volta.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "textline.h"

class Score;
class Xml;
class Volta;
class Measure;

enum VoltaType {
      VOLTA_OPEN, VOLTA_CLOSED
      };

extern void vdebug(int n);
extern LineSegment* voltaDebug;

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

class VoltaSegment : public TextLineSegment {
      Q_OBJECT


   public:
      VoltaSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const  { return new VoltaSegment(*this); }
      Volta* volta() const                 { return (Volta*)spanner(); }
      };

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

class Volta : public TextLine {
      Q_OBJECT

      VoltaType _subtype;
      QList<int> _endings;

   public:
      Volta(Score* s);
      virtual Volta* clone()     const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual LineSegment* createLineSegment();
      virtual void layout();

      virtual void write(Xml&) const;
      virtual void read(const QDomElement& e);

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l) { _endings = l;    }
      void setText(const QString& s);
      QString text() const;

      void setSubtype(VoltaType val);
      VoltaType subtype() const            { return _subtype; }

      bool hasEnding(int repeat) const;
      Measure* startMeasure() const    { return (Measure*)startElement(); }
      Measure* endMeasure() const      { return (Measure*)endElement(); }
      void setStartMeasure(Measure* m) { setStartElement((Element*)m); }
      void setEndMeasure(Measure* m)   { setEndElement((Element*)m);   }
      };

#endif

