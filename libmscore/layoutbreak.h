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

namespace Ms {

// layout break subtypes:

//---------------------------------------------------------
//   @@ LayoutBreak
///    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak final : public Element {
      Q_GADGET
   public:
      enum Type {
            ///.\{
            PAGE, LINE, SECTION, NOBREAK
            ///\}
            };
   private:
      Q_ENUM(Type);

      qreal lw;
      QPainterPath path;
      QPainterPath path2;
      qreal _pause;
      bool _startWithLongNames;
      bool _startWithMeasureOne;
      bool _firstSystemIndentation;
      Type _layoutBreakType;

      void draw(QPainter*) const override;
      void layout0();
      void spatiumChanged(qreal oldValue, qreal newValue) override;

   public:
      LayoutBreak(Score* = 0);
      LayoutBreak(const LayoutBreak&);

      LayoutBreak* clone() const override { return new LayoutBreak(*this); }
      ElementType type() const override   { return ElementType::LAYOUT_BREAK; }
      int subtype() const override        { return static_cast<int>(_layoutBreakType); }

      void setLayoutBreakType(Type);
      Type layoutBreakType() const  { return _layoutBreakType; }

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;
      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      MeasureBase* measure() const           { return (MeasureBase*)parent();  }
      qreal pause() const                    { return _pause;                  }
      void setPause(qreal v)                 { _pause = v;                     }
      bool startWithLongNames() const        { return _startWithLongNames;     }
      void setStartWithLongNames(bool v)     { _startWithLongNames = v;        }
      bool startWithMeasureOne() const       { return _startWithMeasureOne;    }
      void setStartWithMeasureOne(bool v)    { _startWithMeasureOne = v;       }
      bool firstSystemIndentation() const    { return _firstSystemIndentation; }
      void setFirstSystemIndentation(bool v) { _firstSystemIndentation = v;    }

      bool isPageBreak() const    { return _layoutBreakType == PAGE;    }
      bool isLineBreak() const    { return _layoutBreakType == LINE;    }
      bool isSectionBreak() const { return _layoutBreakType == SECTION; }
      bool isNoBreak() const      { return _layoutBreakType == NOBREAK; }

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      Pid propertyId(const QStringRef& xmlName) const override;
      };


}     // namespace Ms

#endif
