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

#ifndef __ELEMENTLAYOUT_H__
#define __ELEMENTLAYOUT_H__

#include "mscore.h"
#include "element.h"

namespace Ms {

//class Element;
class Xml;
class XmlReader;

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

class ElementLayout {

   protected:
      Align  _align;
      qreal _offsetX;              // inch or spatium
      qreal _offsetYAbove;
      qreal _offsetYBelow;
      OffsetType _offsetType;
      QPointF _reloff;

   public:
      ElementLayout();
      ElementLayout(Align a, qreal offsetX, qreal offsetYAbove, qreal offsetYBelow, OffsetType ot, const QPointF& reloff)
            : _align(a), _offsetX(offsetX), _offsetYAbove(offsetYAbove), _offsetYBelow(offsetYBelow), _offsetType(ot), _reloff(reloff) {}
      virtual ~ElementLayout() {}

      Align align() const                 { return _align;        }
      OffsetType offsetType() const       { return _offsetType;   }
      qreal xOffset() const               { return _offsetX;   }
      virtual qreal yOffset() const = 0;
      qreal yOffset(Element::Placement placement) const
                                          { return placement == Element::ABOVE ? _offsetYAbove : _offsetYBelow; }
      QPointF offset(Element::Placement plac, Staff* st) const;
      QPointF offset(Element::Placement plac, Staff* st, qreal sp) const;
      const QPointF& reloff() const       { return _reloff;       }
      void setReloff(const QPointF& val)  { _reloff = val;        }
      void setAlign(Align val)            { _align  = val;        }
      void setXoff(qreal val)             { _offsetX = val;       }
      void setYoff(qreal val, Element::Placement placement)
                                          { (placement == Element::ABOVE ? _offsetYAbove : _offsetYBelow) = val; }
      void setOffsetType(OffsetType val)  { _offsetType = val;    }
      void layout(Element*) const;
      void writeProperties(Xml& xml) const;
      bool readProperties(XmlReader& e);

      void setRxoff(qreal v)              { _reloff.rx() = v; }
      void setRyoff(qreal v)              { _reloff.ry() = v; }

      qreal rxoff() const                 { return _reloff.x(); }
      qreal ryoff() const                 { return _reloff.y(); }
      };


}     // namespace Ms
#endif

