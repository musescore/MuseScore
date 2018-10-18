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

#ifndef __BSYMBOL_H__
#define __BSYMBOL_H__

#include "element.h"

namespace Ms {

//---------------------------------------------------------
//   @@ BSymbol
///    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public Element {
      QList<Element*> _leafs;
      Align _align;

   public:
      BSymbol(Score* s, ElementFlags f = ElementFlag::NOTHING);
      BSymbol(const BSymbol&);

      BSymbol &operator=(const BSymbol&) = delete;

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;
      virtual QRectF drag(EditData&) override;

      void writeProperties(XmlWriter& xml) const;
      bool readProperties(XmlReader&);

      Align align() const { return _align; }
      void setAlign(Align a) { _align = a; }

      const QList<Element*>& leafs() const { return _leafs; }
      QList<Element*>& leafs()             { return _leafs; }
      virtual QPointF pagePos() const override;
      virtual QPointF canvasPos() const override;
      virtual QLineF dragAnchor() const override;
      Segment* segment() const            { return (Segment*)parent(); }
      };

}     // namespace Ms
#endif

