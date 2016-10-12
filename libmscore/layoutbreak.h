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
//   @P layoutBreakType  enum (LayoutBreak.PAGE, LayoutBreak.LINE, LayoutBreak.SECTION)
//---------------------------------------------------------

class LayoutBreak : public Element {
      Q_OBJECT

   public:
      enum Type {
            PAGE, LINE, SECTION, NOBREAK
            };
   private:
      Q_PROPERTY(Ms::LayoutBreak::Type layoutBreakType READ layoutBreakType WRITE undoSetLayoutBreakType)
      Q_ENUMS(Type)

      qreal lw;
      QPainterPath path;
      QPainterPath path2;
      qreal _pause;
      bool _startWithLongNames;
      bool _startWithMeasureOne;
      Type _layoutBreakType;

      virtual void draw(QPainter*) const override;
      void layout0();
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;

   public:
      LayoutBreak(Score* = 0);
      LayoutBreak(const LayoutBreak&);
      virtual LayoutBreak* clone() const override { return new LayoutBreak(*this); }

      virtual Element::Type type() const override { return Element::Type::LAYOUT_BREAK; }
      virtual bool systemFlag() const override    { return true;  }

      void setLayoutBreakType(Type);
      Type layoutBreakType() const  { return _layoutBreakType; }
      void undoSetLayoutBreakType(Type);

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      Measure* measure() const            { return (Measure*)parent();   }
      qreal pause() const                 { return _pause;               }
      void setPause(qreal v)              { _pause = v;                  }
      bool startWithLongNames() const     { return _startWithLongNames;  }
      void setStartWithLongNames(bool v)  { _startWithLongNames = v;     }
      bool startWithMeasureOne() const    { return _startWithMeasureOne; }
      void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v;    }

      bool isPageBreak() const    { return _layoutBreakType == PAGE;    }
      bool isLineBreak() const    { return _layoutBreakType == LINE;    }
      bool isSectionBreak() const { return _layoutBreakType == SECTION; }
      bool isNoBreak() const      { return _layoutBreakType == NOBREAK; }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::LayoutBreak::Type);

#endif
