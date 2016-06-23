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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"

class QPainter;

namespace Ms {

class MuseScoreView;
class Segment;

static const int DEFAULT_BARLINE_TO             = 4 * 2;
static const int MIN_BARLINE_FROMTO_DIST        = 2;
static const int MIN_BARLINE_SPAN_FROMTO        = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static const int BARLINE_SPAN_1LINESTAFF_FROM   = -4;
static const int BARLINE_SPAN_1LINESTAFF_TO     = 4;

// data for some preset bar line span types
static const int BARLINE_SPAN_TICK1_FROM        = -1;
static const int BARLINE_SPAN_TICK1_TO          = 1;
static const int BARLINE_SPAN_TICK2_FROM        = -2;
static const int BARLINE_SPAN_TICK2_TO          = 2;
static const int BARLINE_SPAN_SHORT1_FROM       = 2;
static const int BARLINE_SPAN_SHORT1_TO         = 6;
static const int BARLINE_SPAN_SHORT2_FROM       = 1;
static const int BARLINE_SPAN_SHORT2_TO         = 7;

// used while reading a score for a default spanTo (to last staff line) toward a staff not yet read;
// fixed once all staves are read

static const int UNKNOWN_BARLINE_TO             = -6;

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
//   @P barLineType  enum  (BarLineType.NORMAL, .DOUBLE, .START_REPEAT, .END_REPEAT, .BROKEN, .END, .END_START_REPEAT, .DOTTED)
//---------------------------------------------------------

class BarLine : public Element {
      Q_OBJECT

      Q_PROPERTY(Ms::MSQE_BarLineType::E barLineType READ qmlBarLineType)
      Q_ENUMS(Ms::MSQE_BarLineType::E)

      BarLineType _barLineType { BarLineType::NORMAL };
      int _span                { 1 };           // number of staves spanned by the barline
      int _spanFrom            { 0 };           // line number on start and end staves
      int _spanTo              { DEFAULT_BARLINE_TO };
      bool _customSpan         { false };

      // static variables used while dragging
      static int _origSpan, _origSpanFrom, _origSpanTo;     // original span value before editing
      static qreal yoff1, yoff2;          // used during drag edit to extend y1 and y2
      static bool  ctrlDrag;              // used to mark if [CTRL] has been used while dragging
      static bool  shiftDrag;             // used to mark if [SHIFT] has been used while dragging

      void getY(qreal*, qreal*) const;
      ElementList _el;        ///< fermata or other articulations

      void drawDots(QPainter* painter, qreal x) const;

   public:
      BarLine(Score* s = 0);
      BarLine &operator=(const BarLine&) = delete;

      virtual BarLine* clone() const override     { return new BarLine(*this); }
      virtual Element::Type type() const override { return Element::Type::BAR_LINE; }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void draw(QPainter*) const override;
      virtual QPointF pagePos() const override;      ///< position in canvas coordinates
      virtual void layout() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual QPainterPath outline() const override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual bool isEditable() const override    { return true; }

      Segment* segment() const        { return (Segment*)parent(); }

      void setSpan(int val)           { _span = val;          }
      void setSpanFrom(int val)       { _spanFrom = val;      }
      void setSpanTo(int val)         { _spanTo = val;        }
      int span() const                { return _span;         }
      int spanFrom() const            { return _spanFrom;     }
      int spanTo() const              { return _spanTo;       }
      bool customSpan() const         { return _customSpan;   }
      void setCustomSpan(bool v)      { _customSpan = v;     }

      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual void editDrag(const EditData&) override;
      virtual void endEditDrag() override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }

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

      Ms::MSQE_BarLineType::E qmlBarLineType() const { return static_cast<Ms::MSQE_BarLineType::E>(_barLineType); }

      virtual int subtype() const override         { return int(_barLineType); }
      virtual QString subtypeName() const override { return qApp->translate("barline", barLineTypeName().toUtf8()); }


      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;

      virtual qreal mag() const override;

      static void  setCtrlDrag(bool val)   { ctrlDrag = val; }
      static void  setShiftDrag(bool val)  { shiftDrag = val; }
      static qreal layoutWidth(Score*, BarLineType, qreal mag);

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;

      virtual QString accessibleInfo() const override;
      virtual QString accessibleExtraInfo() const override;

      static const std::vector<BarLineTableItem> barLineTable;
      };
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::MSQE_BarLineType::E);

#endif

