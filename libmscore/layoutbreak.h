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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "element.h"

class QPainter;

namespace Ms {

// layout break subtypes:

//---------------------------------------------------------
//   @@ LayoutBreak
///    symbols for line break, page break etc.
//
//   @P layoutBreakType enum PAGE, LINE, SECTION
//---------------------------------------------------------

class LayoutBreak : public Element {
      Q_OBJECT

   public:
      enum LayoutBreakType {
            PAGE, LINE, SECTION
            };
   private:
      Q_PROPERTY(LayoutBreakType layoutBreakType READ layoutBreakType WRITE undoSetLayoutBreakType)
      Q_ENUMS(LayoutBreakType)

      LayoutBreakType _layoutBreakType;
      qreal lw;
      QPainterPath path;
      QPainterPath path2;
      qreal _pause;
      bool _startWithLongNames;
      bool _startWithMeasureOne;

      virtual void draw(QPainter*) const;
      void layout0();
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

   public:
      LayoutBreak(Score* = 0);
      virtual LayoutBreak* clone() const { return new LayoutBreak(*this); }

      virtual ElementType type() const { return LAYOUT_BREAK; }
      virtual bool systemFlag() const  { return true;  }

      void setLayoutBreakType(LayoutBreakType);
      LayoutBreakType layoutBreakType() const  { return _layoutBreakType; }
      void undoSetLayoutBreakType(LayoutBreakType);

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      Measure* measure() const            { return (Measure*)parent();   }
      qreal pause() const                 { return _pause;               }
      void setPause(qreal v)              { _pause = v;                  }
      bool startWithLongNames() const     { return _startWithLongNames;  }
      void setStartWithLongNames(bool v)  { _startWithLongNames = v;     }
      bool startWithMeasureOne() const    { return _startWithMeasureOne; }
      void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v;    }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::LayoutBreak::LayoutBreakType)

#endif
