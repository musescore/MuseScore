//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: barline.h 5242 2012-01-23 17:25:56Z wschweer $
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

class MuseScoreView;
class Segment;
class QPainter;

//---------------------------------------------------------
//   @@ BarLine
//---------------------------------------------------------

class BarLine : public Element {
      Q_OBJECT

      BarLineType _subtype;
      bool _customSpan;
      int _span;
      int _spanFrom, _spanTo;
      static qreal yoff1, yoff2;          // used during drag edit to extend y1 and y2
      static bool  ctrlDrag;              // used to mark if [CTRL] has been used while dragging

      void getY(qreal*, qreal*) const;
      ElementList _el;        ///< fermata or other articulations

      void drawDots(QPainter* painter, qreal x) const;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
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
      void setCustomSpan(bool val)  { _customSpan = val;    }
      void setSpan(int val)         { _span = val;          }
      void setSpanFrom(int val)     { _spanFrom = val;      }
      void setSpanTo(int val)       { _spanTo = val;        }
      bool customSpan() const       { return _customSpan;   }
      int span() const              { return _span;         }
      int spanFrom() const          { return _spanFrom;     }
      int spanTo() const            { return _spanTo;       }

      virtual bool isEditable() const { return true; }
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      int tick() const;

      ElementList* el()                { return &_el; }
      const ElementList* el() const    { return &_el; }

      QString subtypeName() const;
      void setSubtype(const QString& s);
      void setSubtype(BarLineType i)   { _subtype = i;      }
      BarLineType subtype() const      { return _subtype;  }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);

      static void  setCtrlDrag(bool val)  { ctrlDrag = val; }
      static qreal layoutWidth(Score*, BarLineType, qreal mag);
      };

#endif

