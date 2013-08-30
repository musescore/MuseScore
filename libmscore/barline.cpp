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

#include "barline.h"
#include "score.h"
#include "sym.h"
#include "staff.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "articulation.h"
#include "stafftype.h"

namespace Ms {

//---------------------------------------------------------
//   static members init
//---------------------------------------------------------

qreal BarLine::yoff1 = 0.0;
qreal BarLine::yoff2 = 0.0;

bool BarLine::ctrlDrag = false;
int  BarLine::_origSpan, BarLine::_origSpanFrom, BarLine::_origSpanTo;

//---------------------------------------------------------
//   barLineNames
//    must be synchronized with enum BarLineType
//---------------------------------------------------------

static const char* barLineNames[] = {
      "normal", "double", "start-repeat", "end-repeat", "dashed", "end",
      "end-start-repeat", "dotted"
      };

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setBarLineType(NORMAL_BAR);
      _span     = 1;
      _spanFrom = 0;
      _spanTo   = DEFAULT_BARLINE_TO;
      _customSpan = false;
      _customSubtype = false;
      setHeight(DEFAULT_BARLINE_TO/2 * spatium()); // for use in palettes
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal BarLine::mag() const
      {
      qreal m = staff() ? staff()->mag() : 1.0;
      return m;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF BarLine::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system;
      if (parent()->type() != SEGMENT)
            system = static_cast<System*>(parent());
      else
            system = static_cast<Segment*>(parent())->measure()->system();

      qreal yp = y();
      if (system)
            yp += system->staffYpage(staffIdx());
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF BarLine::canvasPos() const
      {
      QPointF p(pagePos());
      Element* e = parent();
      while (e) {
            if (e->type() == PAGE) {
                  p += e->pos();
                  break;
                  }
            e = e->parent();
            }
      return p;
      }

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

void BarLine::getY(qreal* y1, qreal* y2) const
      {
      qreal _spatium = spatium();
      if (parent()) {
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            if (staffIdx2 >= score()->nstaves()) {
                  qDebug("BarLine: bad _span %d", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  }
            Measure* measure;
            System* system;
            qreal yp = 0.0;
            if (parent()->type() == SEGMENT) {
                  Segment* segment = static_cast<Segment*>(parent());
                  measure = segment->measure();
                  system  = measure->system();
                  }
            else {
                  system  = static_cast<System*>(parent());
                  measure = system->firstMeasure();
                  }
            if (measure) {
                  StaffLines* l1 = measure->staffLines(staffIdx1);
                  StaffLines* l2 = measure->staffLines(staffIdx2);

                  if (system)
                        yp += system->staff(staffIdx1)->y();
                  *y1 = l1->y1() - yp;
                  Staff* staff1 = score()->staff(staffIdx1);
                  *y1 += (_spanFrom * staff1->lineDistance() * staff1->spatium()) / 2;
                  *y2 = l2->y1() - yp;
                  Staff* staff2 = score()->staff(staffIdx2);
                  *y2 += (_spanTo   * staff2->lineDistance() * staff2->spatium()) / 2;
                  }
            }
      else {
            // for use in palette
            *y1 = _spanFrom * _spatium / 2;
            *y2 = _spanTo   * _spatium / 2;
            }

      if (selected()) {
            *y1 += yoff1;
            *y2 += yoff2;
            }
      }

//---------------------------------------------------------
//   drawDots
//---------------------------------------------------------

void BarLine::drawDots(QPainter* painter, qreal x) const
      {
      const Sym& dotsym = symbols[score()->symIdx()][dotSym];
      qreal mags = magS();
      qreal _spatium = spatium();

      if (parent() == 0) {    // for use in palette
            dotsym.draw(painter, mags, QPointF(x, 1.5 * _spatium));
            dotsym.draw(painter, mags, QPointF(x, 2.5 * _spatium));
            }
      else if (parent()->type() == SEGMENT) {
            System* s = static_cast<Segment*>(parent())->measure()->system();
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            int sp = _span;
            if (staffIdx2 >= score()->nstaves()) {
                  qDebug("BarLine: bad _span %d", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  sp = staffIdx2 - staffIdx1 + 1;
                  }
            qreal dy  = s->staff(staffIdx1)->y();
            for (int i = 0; i < sp; ++i) {
                  Staff* staff  = score()->staff(staffIdx1 + i);
                  StaffType* st = staff->staffType();
                  qreal doty1   = st->doty1() * _spatium;
                  qreal doty2   = st->doty2() * _spatium;

                  qreal staffy  = s->staff(staffIdx1 + i)->y() - dy;

                  dotsym.draw(painter, mags, QPointF(x, staffy + doty1));
                  dotsym.draw(painter, mags, QPointF(x, staffy + doty2));
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(QPainter* painter) const
      {
      qreal _spatium = score()->spatium();

      qreal lw = score()->styleS(ST_barWidth).val() * _spatium;
      qreal y1, y2;
      getY(&y1, &y2);

      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      switch(barLineType()) {
            case BROKEN_BAR:
                  pen.setStyle(Qt::DashLine);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case DOTTED_BAR:
                  pen.setStyle(Qt::DotLine);
                  painter->setPen(pen);

            case NORMAL_BAR:
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case END_BAR:
                  {
                  qreal lw2 = score()->styleS(ST_endBarWidth).val() * _spatium;
                  qreal d   = score()->styleS(ST_endBarDistance).val() * _spatium;

                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  qreal x = d + lw2 * .5 + lw;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  lw      = point(score()->styleS(ST_doubleBarWidth));
                  qreal d = point(score()->styleS(ST_doubleBarDistance));

                  pen.setWidthF(lw);
                  painter->setPen(pen);
                  qreal x = lw * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  x += d + lw;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case START_REPEAT:
                  {
                  qreal lw2 = point(score()->styleS(ST_endBarWidth));
                  qreal d1  = point(score()->styleS(ST_endBarDistance));

                  qreal x2   =  lw2 * .5;                               // thick line (lw2)
                  qreal x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  qreal x0   =  lw2 + d1 + lw + d1;                     // dot position

                  drawDots(painter, x0);

                  painter->drawLine(QLineF(x1, y1, x1, y2));

                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x2, y1, x2, y2));

                  if (score()->styleB(ST_repeatBarTips)) {
                        qreal mags = magS();
                        symbols[score()->symIdx()][brackettipsRightUp].draw(painter, mags, QPointF(0.0, y1));
                        symbols[score()->symIdx()][brackettipsRightDown].draw(painter, mags, QPointF(0.0, y2));
                        }
                  }
                  break;

            case END_REPEAT:
                  {
                  qreal mags = magS();
                  qreal lw2  = point(score()->styleS(ST_endBarWidth));
                  qreal d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  qreal dotw = dotsym.width(mags);
                  qreal x1   =  dotw + d1 + lw * .5;
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;

                  drawDots(painter, 0.0);
                  painter->drawLine(QLineF(x1, y1, x1, y2));
                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x2, y1, x2, y2));

                  if (score()->styleB(ST_repeatBarTips)) {
                        qreal x = x2 + lw2 * .5;
                        symbols[score()->symIdx()][brackettipsLeftUp].draw(painter, mags, QPointF(x, y1));
                        symbols[score()->symIdx()][brackettipsLeftDown].draw(painter, mags, QPointF(x, y2));
                        }
                  }
                  break;

            case END_START_REPEAT:
                  {
                  qreal mags = magS();
                  qreal lw2  = point(score()->styleS(ST_endBarWidth));
                  qreal d1   = point(score()->styleS(ST_endBarDistance));
                  const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                  qreal dotw = dotsym.width(mags);

                  qreal x1   =  dotw + d1 + lw * .5;                                // thin bar
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;                     // thick bar
                  qreal x3   =  dotw + d1 + lw + d1 + lw2 + d1 + lw * .5;           // thin bar
                  qreal x4   =  dotw + d1 + lw + d1 + lw2 + d1 + lw + d1;           // dot position

                  drawDots(painter, .0);
                  drawDots(painter, x4);
                  painter->drawLine(QLineF(x1, y1, x1, y2));

                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x2, y1, x2, y2));

                  pen.setWidthF(lw);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x3, y1, x3, y2));

                  if (score()->styleB(ST_repeatBarTips)) {
                        qreal x = x2;
                        symbols[score()->symIdx()][brackettipsRightUp].draw(painter, mags, QPointF(x, y1));
                        symbols[score()->symIdx()][brackettipsRightDown].draw(painter, mags, QPointF(x, y2));
                        symbols[score()->symIdx()][brackettipsLeftUp].draw(painter, mags, QPointF(x, y1));
                        symbols[score()->symIdx()][brackettipsLeftDown].draw(painter, mags, QPointF(x, y2));
                        }
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(Xml& xml) const
      {
      xml.stag("BarLine");
      xml.tag("subtype", barLineTypeName());
      if (_customSubtype)
            xml.tag("customSubtype", _customSubtype);
      // if any span value is different from staff's, output all values
      if (  (staff() && (  _span != staff()->barLineSpan()
                           || _spanFrom != staff()->barLineFrom()
                           || _spanTo != staff()->barLineTo()
                         )
             )
            || !staff())            // (palette bar lines have no staff: output all values)
            xml.tag(QString("span from=\"%1\" to=\"%2\"").arg(_spanFrom).arg(_spanTo), _span);
      // if no custom value, output _span only (as in previous code)
      else
            xml.tag("span", _span);
      foreach(const Element* e, _el)
            e->write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(XmlReader& e)
      {
      // if bar line belongs to a staff, span values default to staff values
      if (staff()) {
            _span     = staff()->barLineSpan();
            _spanFrom = staff()->barLineFrom();
            _spanTo   = staff()->barLineTo();
            }
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype") {
                  bool ok;
                  const QString& val(e.readElementText());
                  int i = val.toInt(&ok);
                  if (!ok)
                        setBarLineType(val);
                  else {
                        BarLineType ct = NORMAL_BAR;
                        switch (i) {
                              default:
                              case  0: ct = NORMAL_BAR; break;
                              case  1: ct = DOUBLE_BAR; break;
                              case  2: ct = START_REPEAT; break;
                              case  3: ct = END_REPEAT; break;
                              case  4: ct = BROKEN_BAR; break;
                              case  5: ct = END_BAR; break;
                              case  6: ct = END_START_REPEAT; break;
                              case  7: ct = DOTTED_BAR; break;
                              }
                        setBarLineType(ct);
                        }
                  if (parent() && parent()->type() == SEGMENT) {
                        Measure* m = static_cast<Segment*>(parent())->measure();
                        if (barLineType() != m->endBarLineType())
                              _customSubtype = true;
                        }
                  }
            else if (tag == "customSubtype")
                  _customSubtype = e.readInt();
            else if (tag == "span") {
                  _spanFrom   = e.intAttribute("from", _spanFrom);
                  _spanTo     = e.intAttribute("to", _spanTo);
                  _span       = e.readInt();
                  // WARNING: following statements assume staff and staff bar line spans are correctly set
                  if (staff() && (_span != staff()->barLineSpan()
                     || _spanFrom != staff()->barLineFrom() || _spanTo != staff()->barLineTo()))
                        _customSpan = true;
                  }
            else if (tag == "Articulation") {
                  Articulation* a = new Articulation(score());
                  a->read(e);
                  add(a);
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space BarLine::space() const
      {
      return Space(0.0, width());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      if (type == BAR_LINE) {
            if (parent() && parent()->type() == SEGMENT)
                  return true;
            if (parent() && parent()->type() == SYSTEM) {
                  BarLine* b = static_cast<BarLine*>(e);
                  return (b->barLineType() == BROKEN_BAR || b->barLineType() == DOTTED_BAR
                     || b->barLineType() == NORMAL_BAR || b->barLineType() == DOUBLE_BAR
                     || b->spanFrom() != 0 || b->spanTo() != DEFAULT_BARLINE_TO);
                  }
            }
      else {
            return (type == ARTICULATION
               && parent()
               && parent()->type() == SEGMENT
               && static_cast<Segment*>(parent())->segmentType() == Segment::SegEndBarLine);
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(const DropData& data)
      {
      Element* e = data.element;
      int type = e->type();
      if (type == BAR_LINE) {
            BarLine* bl = static_cast<BarLine*>(e);
            BarLineType st = bl->barLineType();
            // if no change in subtype or no change in span, do nothing
            if (st == barLineType() && bl->spanFrom() == 0 && bl->spanTo() == DEFAULT_BARLINE_TO) {
                  delete e;
                  return 0;
                  }
            // system left-side bar line
            if (parent()->type() == SYSTEM) {
                  BarLine* b = static_cast<System*>(parent())->barLine();
                  score()->undoChangeProperty(b, P_SUBTYPE, int(bl->barLineType()));
                  delete e;
                  return 0;
                  }

            //parent is a segment
            Measure* m = static_cast<Segment*>(parent())->measure();

            // check if the new property can apply to this single bar line
            bool oldRepeat = (barLineType() == START_REPEAT || barLineType() == END_REPEAT
                        || barLineType() == END_START_REPEAT);
            bool newRepeat = (bl->barLineType() == START_REPEAT || bl->barLineType() == END_REPEAT
                        || bl->barLineType() == END_START_REPEAT);
            // if repeats are not involved or drop refers to span rather than subtype =>
            // single bar line drop
            if( (!oldRepeat && !newRepeat) || (bl->spanFrom() != 0 || bl->spanTo() != DEFAULT_BARLINE_TO) ) {
                  // if drop refers to span, update this bar line span
                  if(bl->spanFrom() != 0 || bl->spanTo() != DEFAULT_BARLINE_TO) {
                        // if dropped spanFrom or spanTo are below the middle of standard staff (5 lines)
                        // adjust to the number of syaff lines
                        int bottomSpan = (staff()->lines()-1) * 2;
                        int spanFrom   = bl->spanFrom() > 4 ? bottomSpan - (8 - bl->spanFrom()) : bl->spanFrom();
                        int spanTo     = bl->spanTo() > 4 ? bottomSpan - (8 - bl->spanTo()) : bl->spanTo();
                        score()->undoChangeSingleBarLineSpan(this, 1, spanFrom, spanTo);
                        }
                  // if drop refer to subtype, update this bar line subtype
                  else {
//                        score()->undoChangeBarLine(m, bl->barLineType());
                        score()->undoChangeProperty(this, P_SUBTYPE, int(bl->barLineType()));
                        }
                  delete e;
                  return 0;
                  }

            // drop applies to all bar lines of the measure
            if (st == START_REPEAT) {
                  m = m->nextMeasure();
                  if (m == 0) {
                        delete e;
                        return 0;
                        }
                  }
            m->drop(data);
            }
      else if (type == ARTICULATION) {
            Articulation* atr = static_cast<Articulation*>(e);
            atr->setParent(this);
            atr->setTrack(track());
            score()->undoAddElement(atr);
            return atr;
            }
      return 0;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void BarLine::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 2;
      qreal lw = point(score()->styleS(ST_barWidth));
      qreal y1, y2;
      getY(&y1, &y2);
      grip[0].translate(QPointF(lw * .5, y1) + pagePos());
      grip[1].translate(QPointF(lw * .5, y2) + pagePos());
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void BarLine::startEdit(MuseScoreView*, const QPointF&)
{
      // keep a copy of original span values
      _origSpan         = _span;
      _origSpanFrom     = _spanFrom;
      _origSpanTo       = _spanTo;
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit()
      {
      if (ctrlDrag) {                      // if single bar line edit
            ctrlDrag = false;
            _customSpan       = true;           // mark bar line as custom spanning
            int newSpan       = _span;          // copy edited span values
            int newSpanFrom   = _spanFrom;
            int newSpanTo     = _spanTo;
            _span             = _origSpan;      // restore original span values
            _spanFrom         = _origSpanFrom;
            _spanTo           = _origSpanTo;
            score()->undoChangeSingleBarLineSpan(this, newSpan, newSpanFrom, newSpanTo);
            return;
            }

      // if same as staff settings, do nothing
      if (staff()->barLineSpan() == _span && staff()->barLineFrom() == _spanFrom && staff()->barLineTo() == _spanTo)
            return;

      int idx1 = staffIdx();

      if(_span != staff()->barLineSpan()) {
            // if now bar lines span more staves
            if (_span > staff()->barLineSpan()) {
                  int idx2 = idx1 + _span;
                  // set span 0 to all additional staves
                  for (int idx = idx1 + 1; idx < idx2; ++idx)
                        // mensurstrich special case:
                        // if line spans to top line of a stave AND current staff is
                        //    the last spanned staff BUT NOT the last score staff
                        //          keep its bar lines
                        // otherwise remove them
                        if(_spanTo > 0 || !(idx == idx2-1 && idx != score()->nstaves()-1) )
                              score()->undoChangeBarLineSpan(score()->staff(idx), 0, 0,
                                          (score()->staff(idx)->lines()-1)*2);
                  }
            // if now bar lines span fewer staves
            else {
                  int idx1 = staffIdx() + _span;
                  int idx2 = staffIdx() + staff()->barLineSpan();
                  // set standard span for each no-longer-spanned staff
                  for (int idx = idx1; idx < idx2; ++idx)
                        score()->undoChangeBarLineSpan(score()->staff(idx), 1, 0, (score()->staff(idx)->lines()-1)*2);
                  }
            }

      // update span for the staff the edited bar line belongs to
      score()->undoChangeBarLineSpan(staff(), _span, _spanFrom, _spanTo);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(const EditData& ed)
      {
      qreal lineDist = staff()->lineDistance() * spatium();
      qreal min, max, lastmax, y1, y2;
      getY(&y1, &y2);
      y1 -= yoff1;                  // current positions of barline ends, ignoring any in-process dragging
      y2 -= yoff2;
      if(ed.curGrip == 0) {
            // min offset for top grip is line -1
            // max offset is 1 line above bottom grip or 1 below last staff line, whichever comes first
            min = -y1 - lineDist;
            max = y2 - y1 - lineDist;                                   // 1 line above bottom grip
            lastmax = (staff()->lines() - _spanFrom/2) * lineDist;      // 1 line below last staff line
            if(lastmax < max)
                  max = lastmax;
            // update yoff1 and bring it within limits
            yoff1 += ed.delta.y();
            if(yoff1 < min)
                  yoff1 = min;
            if(yoff1 > max)
                  yoff1 = max;
            }
      else {
            // min for bottom grip is 1 line below top grip
            // no max
            min = y1 - y2 + lineDist;
            // update yoff2 and bring it within limit
            yoff2 += ed.delta.y();
            if(yoff2 < min)
                  yoff2 = min;
            }
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff / staff line
//---------------------------------------------------------

void BarLine::endEditDrag()
      {
      qreal y1, y2;
      getY(&y1, &y2);
      qreal ay0 = pagePos().y();
      qreal ay2 = ay0 + y2;                     // absolute (page-relative) bar line bottom coord

      int staffIdx1 = staffIdx();
      int staffIdx2;

      System* syst;
      if (parent()->type() == SYSTEM) {
            syst = static_cast<System*>(parent());
            }
      else {
            syst = static_cast<Segment*>(parent())->measure()->system();
            }
      qreal systTopY = syst->pagePos().y();

      // determine new span value
      int numOfStaves = syst->staves()->size();
      if (staffIdx1 + 1 >= numOfStaves)
            // if initial staff is last staff, ending staff must be the same
            staffIdx2 = staffIdx1;

      else {
            // if there are other staves after it, look for staff nearest to bar line bottom coord
            qreal staff1TopY = syst->staff(staffIdx1)->y() + systTopY;

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < numOfStaves; ++staffIdx2) {
                  // compute 1st staff height, absolute top Y of 2nd staff and height of blank between the staves
                  Staff * staff1      = score()->staff(staffIdx2-1);
                  qreal staff1Hght    = (staff1->lines()-1) * staff1->lineDistance() * spatium();
                  qreal staff2TopY    = systTopY   + syst->staff(staffIdx2)->y();
                  qreal blnkBtwnStaff = staff2TopY - staff1TopY - staff1Hght;
                  // if bar line bottom coord is above than mid-way of blank between staves...
                  if (ay2 < (staff1TopY + staff1Hght + blnkBtwnStaff * .5))
                        break;                  // ...staff 1 is ending staff
                  // if bar line is below, advance to next staff
                  staff1TopY = staff2TopY;
                  }
            staffIdx2 -= 1;
            }
      int newSpan = staffIdx2 - staffIdx1 + 1;

      // determine new spanFrom value
      int newSpanFrom = _spanFrom;
      if(yoff1 != 0.0) {
            // round bar line top coord to nearest line of 1st staff (in half line dist units)
            newSpanFrom = ((int)floor(y1 / (staff()->lineDistance() * spatium()) + 0.5 )) * 2;
            // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
            if(newSpanFrom <  MIN_BARLINE_SPAN_FROMTO)
                  newSpanFrom = MIN_BARLINE_SPAN_FROMTO;
            if(newSpanFrom > staff()->lines()*2)
                  newSpanFrom = staff()->lines()*2;
            }

      // determine new spanTo value
      int newSpanTo = _spanTo;
      if(yoff2 != 0.0) {
            // round bar line bottom coord to nearest line of 2nd staff (in half line dist units)
            Staff * staff2   = score()->staff(staffIdx2);
            qreal staff2TopY = systTopY + syst->staff(staffIdx2)->y();
            newSpanTo = ((int)floor( (ay2 - staff2TopY) / (staff2->lineDistance() * spatium()) + 0.5 )) * 2;
            // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
            if(newSpanTo <  MIN_BARLINE_SPAN_FROMTO)
                  newSpanTo = MIN_BARLINE_SPAN_FROMTO;
            if(newSpanTo > staff()->lines()*2)
                  newSpanTo = staff2->lines()*2;
            }

      // if any value changed, update
      if(newSpan != _span || newSpanFrom != _spanFrom || newSpanTo != _spanTo) {
            _span       = newSpan;
            _spanFrom   = newSpanFrom;
            _spanTo     = newSpanTo;
            }

      yoff1 = yoff2 = 0.0;
      }

//---------------------------------------------------------
//   layoutWidth
//---------------------------------------------------------

qreal BarLine::layoutWidth(Score* score, BarLineType type, qreal mag)
      {
      qreal _spatium = score->spatium();
      qreal dw = score->styleS(ST_barWidth).val() * _spatium;

      qreal dotwidth = symbols[score->symIdx()][dotSym].width(mag);
      switch(type) {
            case DOUBLE_BAR:
                  dw  = (score->styleS(ST_doubleBarWidth) * 2
                     + score->styleS(ST_doubleBarDistance)).val() * _spatium;
                  break;
            case START_REPEAT:
                  dw += dotwidth + (score->styleS(ST_endBarWidth)
                     + 2 * score->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case END_REPEAT:
                  dw += dotwidth + (score->styleS(ST_endBarWidth)
                     + 2 * score->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case END_BAR:
                  dw += (score->styleS(ST_endBarWidth)
                     + score->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case  END_START_REPEAT:
                  dw += 2 * dotwidth + (score->styleS(ST_barWidth)
                     + score->styleS(ST_endBarWidth)
                     + 4 * score->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case BROKEN_BAR:
            case NORMAL_BAR:
            case DOTTED_BAR:
                  break;
            default:
                  qDebug("illegal bar line type");
                  break;
            }
      return dw;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BarLine::layout()
      {
      qreal y1, y2;
      getY(&y1, &y2);

      // if bar line does not belong to a system, has a staff and staff is set to hide bar lines, set null bbox
      if (parent() && parent()->type() != Element::SYSTEM && staff() && !staff()->staffType()->showBarlines())
            setbbox(QRectF());

      // bar lines not hidden
      else {
            qreal dw = layoutWidth(score(), barLineType(), magS());
            QRectF r(0.0, y1, dw, y2-y1);

            if (score()->styleB(ST_repeatBarTips)) {
                  qreal mags = magS();
                  int si = score()->symIdx();
                  switch (barLineType()) {
                        case START_REPEAT:
                              r |= symbols[si][brackettipsRightUp].bbox(mags).translated(0, y1);
                              r |= symbols[si][brackettipsRightDown].bbox(mags).translated(0, y2);
                              break;
                        case END_REPEAT:
                              r |= symbols[si][brackettipsLeftUp].bbox(mags).translated(0, y1);
                              r |= symbols[si][brackettipsLeftDown].bbox(mags).translated(0, y2);
                              break;

                        case END_START_REPEAT:
                              {
                              qreal lw   = point(score()->styleS(ST_barWidth));
                              qreal lw2  = point(score()->styleS(ST_endBarWidth));
                              qreal d1   = point(score()->styleS(ST_endBarDistance));
                              const Sym& dotsym = symbols[score()->symIdx()][dotSym];
                              qreal dotw = dotsym.width(mags);
                              qreal x   =  dotw + 2 * d1 + lw + lw2 * .5;                     // thick bar

                              r |= symbols[si][brackettipsRightUp].bbox(mags).translated(x, y1);
                              r |= symbols[si][brackettipsRightDown].bbox(mags).translated(x, y2);
                              r |= symbols[si][brackettipsLeftUp].bbox(mags).translated(x, y1);
                              r |= symbols[si][brackettipsLeftDown].bbox(mags).translated(x, y2);
                              }
                              break;

                        default:
                              break;
                        }
                  }
            setbbox(r);
            }

      // in any case, lay out attached elements
      foreach(Element* e, _el) {
            e->layout();
            if (e->type() == ARTICULATION) {
                  Articulation* a       = static_cast<Articulation*>(e);
                  MScore::Direction dir = a->direction();
                  qreal distance        = 0.5 * spatium();
                  qreal x               = width() * .5;
                  if (dir == MScore::DOWN) {
                        qreal botY = y2 + distance;
                        a->setPos(QPointF(x, botY));
                        }
                  else {
                        qreal topY = y1 - distance;
                        a->setPos(QPointF(x, topY));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath BarLine::shape() const
      {
      QPainterPath p;
      qreal d = spatium() * .3;
      p.addRect(bbox().adjusted(-d, .0, d, .0));
      return p;
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int BarLine::tick() const
      {
      return (parent() && parent()->type() == SEGMENT)
         ? static_cast<Segment*>(parent())->tick() : 0;
      }

//---------------------------------------------------------
//   barLineTypeName
//---------------------------------------------------------

QString BarLine::barLineTypeName() const
      {
      return QString(barLineNames[barLineType()]);
      }

//---------------------------------------------------------
//   setBarLineType
//---------------------------------------------------------

void BarLine::setBarLineType(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(barLineNames)/sizeof(*barLineNames); ++i) {
            if (barLineNames[i] == s) {
                  _barLineType = BarLineType(i);
                  return;
                  }
            }
      _barLineType = NORMAL_BAR;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BarLine::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      // if no width (staff has bar lines turned off) and not all requested, do nothing
      if (width() == 0.0 && !all)
            return;
      func(data, this);
      foreach(Element* e, _el)
            e->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BarLine::add(Element* e)
      {
      if (parent() && parent()->type() != SEGMENT) {
            delete e;
            return;
            }
	e->setParent(this);
      switch(e->type()) {
            case ARTICULATION:
                  _el.push_back(e);
                  setGenerated(false);
                  if (parent() && parent()->parent())
                        static_cast<Measure*>(parent()->parent())->setEndBarLineGenerated(false);
                  break;
            default:
                  qDebug("BarLine::add() not impl. %s", e->name());
                  delete e;
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BarLine::remove(Element* e)
      {
      switch(e->type()) {
            case ARTICULATION:
                  if (!_el.remove(e))
                        qDebug("BarLine::remove(): cannot find %s", e->name());
                  break;
            default:
                  qDebug("BarLine::remove() not impl. %s", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   updateCustomSpan
//---------------------------------------------------------

void BarLine::updateCustomSpan()
      {
      // if barline belongs to a staff and any of the staff span params is different from barline's...
      if (staff())
            if (staff()->barLineSpan() != _span || staff()->barLineFrom() != _spanFrom || staff()->barLineTo() != _spanTo) {
                  _customSpan = true;           // ...span is custom
                  return;
                  }
      // if no staff or same span params as staff, span is not custom
      _customSpan = false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_SUBTYPE:
                  return int(_barLineType);
            case P_BARLINE_SPAN:
                  return span();
            case P_BARLINE_SPAN_FROM:
                  return spanFrom();
            case P_BARLINE_SPAN_TO:
                  return spanTo();
            default:
                  break;
            }
      return Element::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(P_ID id, const QVariant& v)
      {
      switch(id) {
            case P_SUBTYPE:
                  _barLineType = BarLineType(v.toInt());
                  _customSubtype = parent() && (static_cast<Segment*>(parent())->measure())->endBarLineType() != v.toInt();
                  break;
            case P_BARLINE_SPAN:
                  setSpan(v.toInt());
                  break;
            case P_BARLINE_SPAN_FROM:
                  setSpanFrom(v.toInt());
                  break;
            case P_BARLINE_SPAN_TO:
                  setSpanTo(v.toInt());
                  break;
            default:
                  return Element::setProperty(id, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant BarLine::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SUBTYPE:
                  // default subtype is the subtype of the measure, if any
                  if (parent() && parent()->type() == Element::SEGMENT && static_cast<Segment*>(parent())->measure() )
                      return static_cast<Segment*>(parent())->measure()->endBarLineType();
                  return NORMAL_BAR;
            case P_BARLINE_SPAN:
                  // if there is a staff, default span is staff span
                  if (staff())
                        return staff()->barLineSpan();
                  // if no staff, default span is 1
                  return 1;
            case P_BARLINE_SPAN_FROM:
                  // if there is a staff, default From span is staff From span
                  if (staff())
                        return staff()->barLineFrom();
                  // if no staff, default From is from top
                  return 0;
            case P_BARLINE_SPAN_TO:
                  // if there is a staff, default To span is staff To span
                  if (staff())
                        return staff()->barLineTo();
                  // if no staff, assume a standard 5-line setup
                  return DEFAULT_BARLINE_TO;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

}

