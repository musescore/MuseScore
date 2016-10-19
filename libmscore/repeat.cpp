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

#include "repeat.h"
#include "sym.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "mscore.h"
#include "undo.h"
#include <QPointF>

namespace Ms {

//---------------------------------------------------------
//   RepeatMeasure
///   default size is a single-measure repeat
///   default slashes is 1.
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score, int repeatMeasureSize, int slashes)
   : ChordRest(score)
      {
      _repeatMeasureSize = repeatMeasureSize;
      _repeatMeasureSlashes = slashes;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      }

RepeatMeasure::RepeatMeasure(const RepeatMeasure& rm, bool link)
   : ChordRest(rm, link)
      {
      if (link)
            score()->undo(new Link(const_cast<RepeatMeasure*>(&rm), this)); //don't know need to do this linking here, but just following rest's constructor
      _repeatMeasureSize    = rm._repeatMeasureSize;
      _repeatMeasureSlashes = rm._repeatMeasureSlashes;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant RepeatMeasure::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  return _repeatMeasureSize;
            case P_ID::REPEAT_MEASURE_SLASHES:
                  return _repeatMeasureSlashes;
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RepeatMeasure::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  return 1;
            case P_ID::REPEAT_MEASURE_SLASHES:
                  return 1;
            default:
                  return ChordRest::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool RepeatMeasure::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  _repeatMeasureSize = v.toInt();
                  break;

            case P_ID::REPEAT_MEASURE_SLASHES:
                  _repeatMeasureSlashes = v.toInt();
                  break;
            default:
                  return ChordRest::setProperty(propertyId, v);
            }
      return true;
      }

//--------------------------------------------------
//   write
//---------------------------------------------------------

void RepeatMeasure::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperty(xml, P_ID::REPEAT_MEASURE_SIZE);
      writeProperty(xml, P_ID::REPEAT_MEASURE_SLASHES);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RepeatMeasure::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "repeatMeasureSize")
                  _repeatMeasureSize = e.readInt();
            else if (tag == "repeatMeasureSlashes")
                  _repeatMeasureSlashes = e.readInt();
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   symbol
///   returns SymId of the repeat symbol glyph if have one with the number of slahes
//---------------------------------------------------------

SymId RepeatMeasure::symbol() const
      {
      if (repeatMeasureSlashes() == 1)
            return SymId::repeat1Bar;
      else if (repeatMeasureSlashes() == 2)
            return SymId::repeat2Bars;
      else if (repeatMeasureSlashes() == 4)
            return SymId::repeat4Bars;
      else
            return SymId::noSym; // no symbol available with the particular number of slahes
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter* painter) const
      {
      // if no glpyh is available, draw by hand, else use glpyh.
      if (symbol() == SymId::noSym) {
            painter->setBrush(QBrush(curColor()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(path);
            }
      else {
            painter->setPen(curColor());
            drawSymbol(symbol(), painter, QPointF());
            }

      // draw number of measures above the repeat measure's parent y position (i.e. above the top of the staff)
      if (_repeatMeasureSize > 1) { // maybe use some other condition about toggling display of number...maybe customized via inspector or preferences or style? ...use same condition in layout

            std::vector<Ms::SymId> repeatMeasureSizeSymbols = toTimeSigString(QString("%1").arg(_repeatMeasureSize));
            qreal numberYOffset = - spatium() * 1.5 - y();                                // place above the top of staff
            qreal numberXOffset = bbox().width() * .5 - symBbox(repeatMeasureSizeSymbols).width() * .5; // center justification
            painter->setPen(curColor());
            drawSymbols(repeatMeasureSizeSymbols, painter, QPointF(numberXOffset, numberYOffset));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout()
      {
      if (symbol() == SymId::noSym) {
            path      = QPainterPath();

            // total width of symbol = (lw + ls) * (_repeatMeasureSlashes - 1) + w;
            qreal sp  = spatium();
            qreal top = -sp;       // top of symbol is one sp above center
            qreal w   = sp * 2.4;  // tip-to-tip horizontal span of each slash
            qreal h   = sp * 2.0;  // vertical span of each slash
            qreal lw  = sp * .50;  // slash width
            qreal ls  = sp * .50;  // space between slashes
            qreal r   = sp * .20;  // dot radius

            qreal xoffset = 0.0;
            // draw each slash
            for (int i=0; i<_repeatMeasureSlashes; i++ ) {
                  path.moveTo(xoffset + w - lw, top);
                  path.lineTo(xoffset + w     , top);
                  path.lineTo(xoffset     + lw, top + h);
                  path.lineTo(xoffset         , top + h);
                  path.closeSubpath();
                  xoffset += lw + ls;
                  }

            // dots on each side
            path.addEllipse(QRectF(                  + w * .25 - r, top + h * .25 - r, r * 2.0, r * 2.0 ));
            path.addEllipse(QRectF(xoffset - lw - ls + w * .75 - r, top + h * .75 - r, r * 2.0, r * 2.0 ));

            setbbox(path.boundingRect());
            }
      else {
            // adjust y offset if symbol in palette so vertically centered (why I need to do this, I'm not sure)
            if (parent() == 0)
                  setPos(0.0, 1.0 * spatium());

            // figure out box for the particular symbol
            QRectF bbox = symBbox(symbol());
            bbox.moveTop(-bbox.top()); // for some reason the symbol returns a negative value for top
            setbbox(bbox);
            }

      if (_repeatMeasureSize > 1) { // maybe some condition here to toggle display of number...but make sure same condition as in draw
            // add approximate space above symbol for display of number
            addbbox(QRectF(0, - y() - 3.0 * spatium(), width(), spatium() * 2.0));
            }
      }
//---------------------------------------------------------
//   sumMeasureWidthsMutltiMeasureRepeatHalfway
///   helper function to determine horizontal offset a multi-measure repeat should be displayed based on the laid-out widths of subsequent measures.
///   If encounter last measure of the system before reach center of covered measures, then will place element at final barline of system
//---------------------------------------------------------

qreal RepeatMeasure::sumMeasureWidthsMutltiMeasureRepeatHalfway(const Measure* const startingMeasure, const Measure* const lastMeasureOfSystem)
      {
      qreal sumMeasureWidths = 0.0; //-m->x();
      const MeasureBase* m = startingMeasure;
      for (int i = 0; i < _repeatMeasureSize / 2; i++) { // todo: handle situation where HBox is inbetween

            sumMeasureWidths += m->width(); // add this measure's width to cumulative sum

            if (m == lastMeasureOfSystem || (m->next() && m->next()->isHBox()))  // if forced line break or layout determined system was over or HBox
                  return sumMeasureWidths;

            m = m->next();
            if (m == 0) {
                  qWarning("Somehow have a multi-measure repeat at end of score, such that multi-meas repeat covers measures outside of range");
                  return sumMeasureWidths; // just use the width have summed so far
                  }
            }

      if (_repeatMeasureSize & 1)
            return sumMeasureWidths + m->width() / 2.0; // if odd number of measures, midX will be halfway through the middle measure
      else
            return sumMeasureWidths; // if even, will place elemnt right on barline
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

Fraction RepeatMeasure::duration() const
      {
      if (measure())
            return measure()->stretchedLen(staff()) * _repeatMeasureSize;
      return Fraction(0, 1);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString RepeatMeasure::accessibleInfo() const
      {
      return Element::accessibleInfo();
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void RepeatMeasure::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      ChordRest::scanElements(data, func, all);
      func(data, this);
      }

//////////
// the following are copied from Rest code...may need to modify for repeat measure
//////////

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal RepeatMeasure::centerX() const
      {
      return 0.0;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal RepeatMeasure::upPos() const
      {
      return 0.0;
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal RepeatMeasure::downPos() const
      {
      return 0.0;
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int RepeatMeasure::upLine() const
      {
      return 0.0;
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int RepeatMeasure::downLine() const
      {
      return 0.0;
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF RepeatMeasure::stemPos() const
      {
      return QPointF(0,0);
      }

//---------------------------------------------------------
//   stemPosBeam
//---------------------------------------------------------

QPointF RepeatMeasure::stemPosBeam() const
      {
      return QPointF(0,0);
      }

//---------------------------------------------------------
//   stemPosX
//---------------------------------------------------------

qreal RepeatMeasure::stemPosX() const
      {
      return 0.0;
      }

}

