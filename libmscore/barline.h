//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"
#include "mscore.h"

namespace Ms {

class MuseScoreView;
class Segment;

static const int MIN_BARLINE_FROMTO_DIST        = 2;
static const int MIN_BARLINE_SPAN_FROMTO        = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static const int BARLINE_SPAN_1LINESTAFF_FROM   = -4;
static const int BARLINE_SPAN_1LINESTAFF_TO     = 4;

// data for some preset bar line span types
static const int BARLINE_SPAN_TICK1_FROM        = -1;
static const int BARLINE_SPAN_TICK1_TO          = -7;
static const int BARLINE_SPAN_TICK2_FROM        = -2;
static const int BARLINE_SPAN_TICK2_TO          = -6;
static const int BARLINE_SPAN_SHORT1_FROM       = 2;
static const int BARLINE_SPAN_SHORT1_TO         = -2;
static const int BARLINE_SPAN_SHORT2_FROM       = 1;
static const int BARLINE_SPAN_SHORT2_TO         = -1;

//---------------------------------------------------------
//   BarLineTableItem
//---------------------------------------------------------

struct BarLineTableItem {
      BarLineType type;
      const char* userName;       // user name, translatable
      const char* name;
      };

//---------------------------------------------------------
//   @@ BarLine
//
//   @P barLineType  enum  (BarLineType.NORMAL, .DOUBLE, .START_REPEAT, .END_REPEAT, .BROKEN, .END, .DOTTED)
//---------------------------------------------------------

class BarLine final : public Element {
      int _spanStaff          { 0 };       // span barline to next staff if true, values > 1 are used for importing from 2.x
      char _spanFrom          { 0 };       // line number on start and end staves
      char _spanTo            { 0 };
      BarLineType _barLineType { BarLineType::NORMAL };
      mutable qreal y1;
      mutable qreal y2;
      ElementList _el;        ///< fermata or other articulations

      void getY() const;
      void drawDots(QPainter* painter, qreal x) const;
      void drawTips(QPainter* painter, bool reversed, qreal x) const;
      bool isTop() const;
      bool isBottom() const;
      void drawEditMode(QPainter*, EditData&);

   public:
      BarLine(Score* s = 0);
      virtual ~BarLine();
      BarLine(const BarLine&);
      BarLine &operator=(const BarLine&) = delete;

      virtual BarLine* clone() const override     { return new BarLine(*this); }
      virtual ElementType type() const override   { return ElementType::BAR_LINE; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void draw(QPainter*) const override;
      virtual QPointF pagePos() const override;      ///< position in canvas coordinates
      virtual void layout() override;
      void layout2();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual bool isEditable() const override    { return true; }

      Segment* segment() const        { return toSegment(parent()); }
      Measure* measure() const        { return toMeasure(parent()->parent()); }

      void setSpanStaff(int val)      { _spanStaff = val;     }
      void setSpanFrom(int val)       { _spanFrom = val;      }
      void setSpanTo(int val)         { _spanTo = val;        }
      int spanStaff() const           { return _spanStaff;    }
      int spanFrom() const            { return _spanFrom;     }
      int spanTo() const              { return _spanTo;       }

      virtual void startEdit(EditData& ed) override;
      virtual void endEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void endEditDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual Shape shape() const override;

      ElementList* el()                  { return &_el; }
      const ElementList* el() const      { return &_el; }

      static QString userTypeName(BarLineType);
      static const BarLineTableItem* barLineTableItem(unsigned);

      QString barLineTypeName() const;
      static QString barLineTypeName(BarLineType t);
      void setBarLineType(const QString& s);
      void setBarLineType(BarLineType i) { _barLineType = i;     }
      BarLineType barLineType() const    { return _barLineType;  }
      static BarLineType barLineType(const QString&);

      virtual int subtype() const override         { return int(_barLineType); }
      virtual QString subtypeName() const override { return qApp->translate("barline", barLineTypeName().toUtf8()); }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid propertyId) const override;
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps);
      using ScoreElement::undoChangeProperty;

      static qreal layoutWidth(Score*, BarLineType);

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;

      virtual QString accessibleInfo() const override;
      virtual QString accessibleExtraInfo() const override;

      static const std::vector<BarLineTableItem> barLineTable;
      };
}     // namespace Ms

#endif

