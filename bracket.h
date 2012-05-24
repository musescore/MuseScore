//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: bracket.h 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "element.h"
#include "mscore.h"

class MuseScoreView;
class System;
class QPainter;

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public Element {
      BracketType _subtype;

      qreal h2;

      int _column, _span;

      QPainterPath path;
      qreal yoff;

   public:
      Bracket(Score*);
      virtual Bracket* clone() const { return new Bracket(*this); }

      virtual ElementType type() const { return BRACKET; }
      BracketType subtype() const      { return _subtype; }
      void setSubtype(BracketType t)   { _subtype = t; }

      int span() const       { return _span; }
      void setSpan(int val)  { _span = val; }
      int level() const      { return _column; }
      void setLevel(int v)   { _column = v; }
      System* system() const { return (System*)parent(); }

      virtual void setHeight(qreal);
      virtual qreal width() const;

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int, int, Qt::KeyboardModifiers, const QString&);
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      };

#endif

