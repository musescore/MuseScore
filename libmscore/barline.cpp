//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: barline.cpp 5604 2012-05-04 15:29:13Z wschweer $
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

#define DEFAULT_BARLINE_TO    4

//---------------------------------------------------------
//   static members init
//---------------------------------------------------------

qreal BarLine::yoff1 = 0.0;
qreal BarLine::yoff2 = 0.0;

bool BarLine::ctrlDrag = false;

//---------------------------------------------------------
//   barLineNames
//    must be synchronized with enum BarLineType
//---------------------------------------------------------

static const char* barLineNames[] = {
      "normal", "double", "start-repeat", "end-repeat", "dashed", "end",
      "end-start-repeat"
      };

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setSubtype(NORMAL_BAR);
      _span     = 1;
      _spanFrom = 0;
      _spanTo   = DEFAULT_BARLINE_TO;
      setHeight(DEFAULT_BARLINE_TO * spatium()); // for use in palettes
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF BarLine::pagePos() const
      {
      if (parent() == 0)
            return pos();
      if (parent()->type() != SEGMENT)
            return pos() + parent()->pagePos();

      System* system = static_cast<Segment*>(parent())->measure()->system();

      qreal yp = y();
      if (system)
            yp += system->staffY(staffIdx());
      return QPointF(pageX(), yp);
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
                  qDebug("BarLine: bad _span %d\n", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  }
            Measure* measure;
            System* system;
            if (parent()->type() == SEGMENT) {
                  Segment* segment = static_cast<Segment*>(parent());
                  measure = segment->measure();
                  system = measure->system();
                  }
            else {
                  system = static_cast<System*>(parent());
                  measure = system->firstMeasure();
                  }

            StaffLines* l1   = measure->staffLines(staffIdx1);
            StaffLines* l2   = measure->staffLines(staffIdx2);

            qreal yp = system ? system->staff(staffIdx())->y() : 0.0;
            *y1 = l1->y1() - yp;
            *y1 += _spanFrom * score()->staff(staffIdx1)->lineDistance() * _spatium;
            *y2 = l2->y1() - yp;
            *y2 += _spanTo   * score()->staff(staffIdx2)->lineDistance() * _spatium;
            }
      else {
            // for use in palette
            *y1 = 0.0;
            *y2 = DEFAULT_BARLINE_TO * _spatium;
            }

      *y1 += yoff1;
      *y2 += yoff2;
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
            int _staffIdx = staffIdx();
            qreal dy  = s->staff(_staffIdx)->y();
            for (int i = 0; i < _span; ++i) {
                  Staff* staff  = score()->staff(_staffIdx + i);
                  StaffType* st = staff->staffType();
                  qreal doty1   = st->doty1() * _spatium;
                  qreal doty2   = st->doty2() * _spatium;

                  qreal staffy  = s->staff(_staffIdx + i)->y() - dy;

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
      qreal lw = point(score()->styleS(ST_barWidth));
      qreal y1, y2;
      getY(&y1, &y2);

      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);
      qreal mags = magS();

      switch(subtype()) {
            case BROKEN_BAR:
                  pen.setStyle(Qt::DashLine);
                  painter->setPen(pen);

            case NORMAL_BAR:
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case END_BAR:
                  {
                  qreal lw2 = point(score()->styleS(ST_endBarWidth));
                  qreal d   = point(score()->styleS(ST_endBarDistance));

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
                        symbols[score()->symIdx()][brackettipsRightUp].draw(painter, mags, QPointF(0.0, y1));
                        symbols[score()->symIdx()][brackettipsRightDown].draw(painter, mags, QPointF(0.0, y2));
                        }
                  }
                  break;

            case END_REPEAT:
                  {
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
      xml.tag("subtype", subtypeName());
      // palette bar lines have no staff!
      if (staff() && (_span != staff()->barLineSpan()
                  || _spanFrom != staff()->barLineFrom() || _spanTo != staff()->barLineTo()) )
            xml.tag(QString("span from=\"%1\" to=\"%2\"").arg(_spanFrom).arg(_spanTo), _span);
      foreach(const Element* e, _el)
            e->write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(const QDomElement& de)
      {
      // if bar line belongs to a staff, span values default to staff values
      if(staff()) {
            _span       = staff()->barLineSpan();
            _spanFrom   = staff()->barLineFrom();
            _spanTo     = staff()->barLineTo();
      }
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "subtype") {
                  bool ok;
                  int i = val.toInt(&ok);
                  if (!ok)
                        setSubtype(val);
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
                              }
                        setSubtype(ct);
                        }
                  }
            else if (tag == "span") {
//                  QString strLine;
                  // WARNING: following statements assume staff and staff bar line spans are correctly set
                  _span  = val.toInt();
//                  strLine.setNum(staff() ? staff()->barLineFrom() : 0);
                  _spanFrom  = e.attribute("from", QString::number(_spanFrom)).toInt();
//                  strLine.setNum(staff() ? staff()->barLineTo() : DEFAULT_BARLINE_TO);
                  _spanTo    = e.attribute("to", QString::number(_spanTo)).toInt();
                  }
            else if (tag == "Articulation") {
                  Articulation* a = new Articulation(score());
                  a->read(e);
                  add(a);
                  }
            else if (!Element::readProperties(e))
                  domError(e);
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
      return type == BAR_LINE
         || (type == ARTICULATION
                && parent()
                && parent()->type() == SEGMENT
                && static_cast<Segment*>(parent())->subtype() == Segment::SegEndBarLine
            )
         ;
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
            BarLineType st = bl->subtype();
            if (st == subtype()) {
                  delete e;
                  return 0;
                  }
            if (parent()->type() == SYSTEM) {
                  BarLine* b = static_cast<System*>(parent())->barLine();
                  score()->undoChangeProperty(b, P_SUBTYPE, int(bl->subtype()));
                  delete e;
                  return 0;
                  }
            Measure* m = static_cast<Segment*>(parent())->measure();
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
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit()
      {
      if (staff()->barLineSpan() == _span && staff()->barLineFrom() == _spanFrom && staff()->barLineTo() == _spanTo)
            return;

      int idx1 = staffIdx();

      if(_span != staff()->barLineSpan()) {
            // if now bar lines span more staves
            if (_span > staff()->barLineSpan()) {
                  int idx2 = idx1 + _span;
                  // set span 0 to all additional staves
                  for (int idx = idx1 + 1; idx < idx2; ++idx)
                        score()->undoChangeBarLineSpan(score()->staff(idx), 0, 0, score()->staff(idx)->lines()-1);
                  }
            // if now bar lines span fewer staves
            else {
                  int idx1 = staffIdx() + _span;
                  int idx2 = staffIdx() + staff()->barLineSpan();
                  // set standard span for each un-spanned staff
                  for (int idx = idx1; idx < idx2; ++idx)
                        score()->undoChangeBarLineSpan(score()->staff(idx), 1, 0, score()->staff(idx)->lines()-1);
                  }
            }
      // update span for the staff the edited bar line belongs to
      score()->undoChangeBarLineSpan(staff(), _span, _spanFrom, _spanTo);
      // added "_score->setLayoutAll(true);" to ChangeBarLineSpan::flip()
      // otherwise no measure bar line update occurs
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
            lastmax = (staff()->lines() - _spanFrom) * lineDist;        // 1 spatium below last staff line
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
//      qreal ay1 = ay0 + y1;
      qreal ay2 = ay0 + y2;                     // absolute (page-relative) bar line bottom coord

      int staffIdx1 = staffIdx();
      int staffIdx2;
      Segment* segm     = (Segment*)parent();
      Measure* meas     = segm->measure();
      System*  syst     = meas->system();
      qreal    systTopY = syst->pagePos().y();

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
            // round bar line top coor to nearest line of 1st staff
            newSpanFrom = (int)floor(y1 / (staff()->lineDistance() * spatium()) + 0.5 );
            // min = 1 spatium above 1st staff line | max = 1 spatium below last staff line
            if(newSpanFrom <  -1)
                  newSpanFrom = -1;
            if(newSpanFrom > staff()->lines())
                  newSpanFrom = staff()->lines();
            }

      // determine new spanTo value
      int newSpanTo = _spanTo;
      if(yoff2 != 0.0) {
            // round bar line bottom coor to nearest line of 2nd staff
            Staff * staff2   = score()->staff(staffIdx2);
            qreal staff2TopY = systTopY + syst->staff(staffIdx2)->y();
            newSpanTo = (int)floor( (ay2 - staff2TopY) / (staff2->lineDistance() * spatium()) + 0.5 );
            // min = 1 spatium above 1st staff line | max = 1 spatium below last staff line
            if(newSpanTo <  -1)
                  newSpanTo = -1;
            if(newSpanTo > staff()->lines())
                  newSpanTo = staff2->lines();
            }

      if (newSpan != _span) {
/*    ONLY TAKE NOTE OF NEW BAR LINE SPAN: LET BarLine::endEdit() DO THE JOB!
            if (newSpan > _span) {
                  int diff = newSpan - _span;
                  staffIdx1 += _span;
                  staffIdx2 = staffIdx1 + diff;
                  Segment* s = score()->firstMeasure()->first(SegEndBarLine);
                  for (; s; s = s->next1(SegEndBarLine)) {
                        for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
                              Element* e = s->element(staffIdx * VOICES);
                              if (e) {
                                    score()->undoRemoveElement(e);
                                    }
                              }
                        }
                  }
*/            _span = newSpan;
//            score()->undoChangeBarLineSpan(staff(), _span);
            }

      yoff1 = yoff2 = 0.0;
      // if any value changed, update with an undoable operation
      if(newSpan != _span || newSpanFrom != _spanFrom || newSpanTo != _spanTo) {
            _span       = newSpan;
            _spanFrom   = newSpanFrom;
            _spanTo     = newSpanTo;
/* to be used for individual bar line edit
            setGenerated(false);
            if(_span == staff()->barLineSpan() && _spanFrom == staff()->barLineFrom() && _spanTo == staff()->barLineTo())
                  setGenerated(true);
*/
            // this should actually be in EndEdit() but, if span changes, bar lines are removed and recreated.
            // While re-creating, staff span values are used as default for new bar lines!
            // so, we need to update the staff span values
//            score()->undoChangeBarLineSpan(staff(), _span, _spanFrom, _spanTo);
            }
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
                  break;
            default:
                  qDebug("illegal bar line type\n");
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
      qreal _spatium = spatium();

      qreal dw = layoutWidth(score(), subtype(), magS());
      QRectF r(0.0, y1, dw, y2-y1);

      if (score()->styleB(ST_repeatBarTips)) {
            qreal mags = magS();
            int si = score()->symIdx();
            switch (subtype()) {
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
      foreach(Element* e, _el) {
            e->layout();
            if (e->type() == ARTICULATION) {
                  Articulation* a       = static_cast<Articulation*>(e);
                  ArticulationAnchor aa = a->anchor();
                  qreal distance        = 0.5 * _spatium;
                  qreal x               = width() - (a->width() * .5);
                  if (aa == A_TOP_STAFF) {
                        qreal topY = y1 - distance;
                        a->setPos(QPointF(x, topY));
                        }
                  else if (aa == A_BOTTOM_STAFF) {
                        qreal botY = y2 + distance;
                        a->setPos(QPointF(x, botY));
                        }
                  }
            }
      setbbox(r);
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
//   subtypeName
//---------------------------------------------------------

QString BarLine::subtypeName() const
      {
      return QString(barLineNames[subtype()]);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void BarLine::setSubtype(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(barLineNames)/sizeof(*barLineNames); ++i) {
            if (barLineNames[i] == s) {
                  _subtype = BarLineType(i);
                  return;
                  }
            }
      _subtype = NORMAL_BAR;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BarLine::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
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
                  _el.append(e);
                  setGenerated(false);
                  if (parent() && parent()->parent())
                        static_cast<Measure*>(parent()->parent())->setEndBarLineGenerated(false);
                  break;
            default:
                  qDebug("BarLine::add() not impl. %s\n", e->name());
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
                        qDebug("BarLine::remove(): cannot find %s\n", e->name());
                  break;
            default:
                  qDebug("BarLine::remove() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(P_ID id) const
      {
      if (id == P_SUBTYPE)
            return _subtype;
      return Element::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(P_ID id, const QVariant& v)
      {
      if (id == P_SUBTYPE)
            _subtype = BarLineType(v.toInt());
      else
            return Element::setProperty(id, v);
      score()->setLayoutAll(true);
      return true;
      }

