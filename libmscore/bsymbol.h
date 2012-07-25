//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: bsymbol.h 5253 2012-01-25 20:40:25Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BSYMBOL_H__
#define __BSYMBOL_H__

#include "element.h"
#include "elementlayout.h"

//---------------------------------------------------------
//   @@ BSymbol
///    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public Element, public ElementLayout {
      Q_OBJECT

      QList<Element*> _leafs;
      int _z;                     ///< stacking order when drawing or selecting;
                                  ///< elements are drawn from high number to low number;
                                  ///< default is type() * 100;

   public:
      BSymbol(Score* s) : Element(s) { setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE); }
      BSymbol(const BSymbol&);

      BSymbol &operator=(const BSymbol&);

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual QRectF drag(const EditData& pos);

      const QList<Element*>& leafs() const { return _leafs; }
      QList<Element*>& leafs()             { return _leafs; }
      virtual QPointF pagePos() const;
      virtual QPointF canvasPos() const;
      virtual QLineF dragAnchor() const;
      Segment* segment() const            { return (Segment*)parent(); }
      virtual int z() const               { return _z; }
      void setZ(int val)                  { _z = val;  }
      };

#endif

