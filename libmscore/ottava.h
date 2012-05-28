//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: ottava.h 5161 2011-12-29 17:26:34Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "textline.h"

//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
      int offset;
      unsigned start;
      unsigned end;
      };

class Ottava;

//---------------------------------------------------------
//   OttavaSegment
//---------------------------------------------------------

class OttavaSegment : public TextLineSegment {
      Q_OBJECT

   protected:

   public:
      OttavaSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const { return new OttavaSegment(*this); }
      Ottava* ottava() const               { return (Ottava*)spanner(); }
      };

//---------------------------------------------------------
//   Ottava
//    brackets
//---------------------------------------------------------

class Ottava : public TextLine {
      Q_OBJECT

      int _subtype;

   protected:
      QString text;
      int _pitchShift;
      mutable qreal textHeight;     ///< cached value

      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      virtual Ottava* clone() const    { return new Ottava(*this); }
      virtual ElementType type() const { return OTTAVA; }
      void setSubtype(int val);
      int subtype() const { return _subtype; }
      virtual LineSegment* createLineSegment();
      virtual void layout();
      int pitchShift() const { return _pitchShift; }
      virtual void endEdit();
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement& de);
      };

#endif

