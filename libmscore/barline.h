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

// used while reading a score for a default spanTo (to last staff line) toward a staff not yet read;
// fixed once all staves are read

static const int UNKNOWN_BARLINE_TO = -4;

//---------------------------------------------------------
//   @@ BarLine
//---------------------------------------------------------

class BarLine : public Element {
      Q_OBJECT

      BarLineType _barLineType;
      bool _customSpan;
      bool _customSubtype;
      int _span;
      int _spanFrom, _spanTo;

      // static variables used while dragging
      static int _origSpan, _origSpanFrom, _origSpanTo;     // original span value before editing
      static qreal yoff1, yoff2;          // used during drag edit to extend y1 and y2
      static bool  ctrlDrag;              // used to mark if [CTRL] has been used while dragging

      void getY(qreal*, qreal*) const;
      ElementList _el;        ///< fermata or other articulations

      void drawDots(QPainter* painter, qreal x) const;
      void updateCustomSpan();

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual void draw(QPainter*) const;
      virtual Space space() const;
      virtual QPointF pagePos() const;      ///< position in canvas coordinates
      virtual void layout();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual QPainterPath shape() const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
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

      virtual bool isEditable() const { return parent()->type() == SEGMENT; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      int tick() const;

      ElementList* el()                  { return &_el; }
      const ElementList* el() const      { return &_el; }

      QString barLineTypeName() const;
      void setBarLineType(const QString& s);
      void setBarLineType(BarLineType i) { _barLineType = i;      }
      BarLineType barLineType() const    { return _barLineType;  }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID propertyId) const;

      static void  setCtrlDrag(bool val)  { ctrlDrag = val; }
      static qreal layoutWidth(Score*, BarLineType, qreal mag);
      };


}     // namespace Ms
#endif

