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

namespace Ms {

class Element;
class Xml;
class XmlReader;

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

class ElementLayout {

   protected:
      Align  _align;
      QPointF _offset;              // inch or spatium
      OffsetType _offsetType;
      QPointF _reloff;

   public:
      ElementLayout();
      ElementLayout(Align a, const QPointF& o, OffsetType ot, const QPointF& r)
         : _align(a), _offset(o), _offsetType(ot), _reloff(r) {}

      Align align() const                 { return _align;        }
      OffsetType offsetType() const       { return _offsetType;   }
      qreal xOffset() const               { return _offset.x();   }
      qreal yOffset() const               { return _offset.y();   }
      const QPointF& offset() const       { return _offset;      }
      QPointF offset(qreal) const;
      const QPointF& reloff() const       { return _reloff;       }
      void setReloff(const QPointF& val)  { _reloff = val;        }
      void setAlign(Align val)            { _align  = val;        }
      void setXoff(qreal val)             { _offset.rx() = val;   }
      void setYoff(qreal val)             { _offset.ry() = val;        }
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

