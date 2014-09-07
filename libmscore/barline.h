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

static const int DEFAULT_BARLINE_TO      = 4 * 2;
static const int MIN_BARLINE_FROMTO_DIST = 2;
static const int MIN_BARLINE_SPAN_FROMTO = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static const int BARLINE_SPAN_1LINESTAFF_FROM = -4;
static const int BARLINE_SPAN_1LINESTAFF_TO   = 4;

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

static const int UNKNOWN_BARLINE_TO = -6;

//---------------------------------------------------------
//   @@ BarLine
//---------------------------------------------------------

class BarLine : public Element {
      Q_OBJECT

      BarLineType _barLineType;
      bool _customSpan;
      bool _customSubtype;
      int _span;                    // number of staves spanned by the barline
      int _spanFrom, _spanTo;       // line number on start and end staves

      // static variables used while dragging
      static int _origSpan, _origSpanFrom, _origSpanTo;     // original span value before editing
      static qreal yoff1, yoff2;          // used during drag edit to extend y1 and y2
      static bool  ctrlDrag;              // used to mark if [CTRL] has been used while dragging
      static bool  shiftDrag;             // used to mark if [SHIFT] has been used while dragging

      void getY(qreal*, qreal*) const;
      ElementList _el;        ///< fermata or other articulations

      void drawDots(QPainter* painter, qreal x) const;
      void updateCustomSpan();
      void updateCustomType();
      void updateGenerated(bool canBeTrue = true);

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&) = delete;

      virtual BarLine* clone() const     { return new BarLine(*this); }
      virtual Element::Type type() const { return Element::Type::BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual void draw(QPainter*) const;
      virtual Space space() const;
      virtual QPointF pagePos() const;      ///< position in canvas coordinates
      virtual QPointF canvasPos() const;    ///< position in canvas coordinates
      virtual void layout();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual QPainterPath shape() const;

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&);
      void setCustomSpan(bool val)    { _customSpan = val;    }
      void setCustomSubtype(bool val) { _customSubtype = val; }
      void setSpan(int val)           { _span = val;        updateCustomSpan();     }
      void setSpanFrom(int val)       { _spanFrom = val;    updateCustomSpan();     }
      void setSpanTo(int val)         { _spanTo = val;      updateCustomSpan();     }
      bool customSpan() const         { return _customSpan;   }
      bool customSubtype() const      { return _customSubtype;}
      int span() const                { return _span;         }
      int spanFrom() const            { return _spanFrom;     }
      int spanTo() const              { return _spanTo;       }

      virtual bool isEditable() const { return parent()->type() == Element::Type::SEGMENT; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, int*, QRectF*) const override;
      int tick() const;

      ElementList* el()                  { return &_el; }
      const ElementList* el() const      { return &_el; }

      static QString userTypeName(BarLineType);
      static QString userTypeName2(BarLineType);

      QString barLineTypeName() const;
      void setBarLineType(const QString& s);
      void setBarLineType(BarLineType i) { _barLineType = i;     updateCustomType();      }
      BarLineType barLineType() const    { return _barLineType;  }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;

      virtual qreal mag() const;

      static void  setCtrlDrag(bool val)  { ctrlDrag = val; }
      static void  setShiftDrag(bool val)  { shiftDrag = val; }
      static qreal layoutWidth(Score*, BarLineType, qreal mag);

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      QString accessibleInfo() override;
      QString accessibleExtraInfo() override;
      };

typedef struct {
      BarLineType type;
      const char* name;
      } barLineTableItem;
extern const barLineTableItem barLineTable[];
unsigned int barLineTableSize();

}     // namespace Ms
#endif

