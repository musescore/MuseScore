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
#include "elementlayout.h"

namespace Ms {

//---------------------------------------------------------
//   @@ BSymbol
///    base class for Symbol and Image
//---------------------------------------------------------

class BSymbol : public Element, public ElementLayout {
      Q_OBJECT

      QList<Element*> _leafs;
      bool _systemFlag;

   public:
      BSymbol(Score* s);
      BSymbol(const BSymbol&);

      BSymbol &operator=(const BSymbol&) = delete;

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual QRectF drag(EditData*) override;

      void writeProperties(Xml& xml) const;
      bool readProperties(XmlReader&);

      const QList<Element*>& leafs() const { return _leafs; }
      QList<Element*>& leafs()             { return _leafs; }
      virtual QPointF pagePos() const override;
      virtual QPointF canvasPos() const override;
      virtual QLineF dragAnchor() const override;
      Segment* segment() const            { return (Segment*)parent(); }
      bool systemFlag() const             { return _systemFlag; }
      void setSystemFlag(bool val)        { _systemFlag = val;  }
      };

}     // namespace Ms
#endif

