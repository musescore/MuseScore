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

//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static int defaultSubtype = 0;

Property<BarLine> BarLine::propertyList[] = {
      { P_SUBTYPE, &BarLine::pSubtype, &defaultSubtype },
      };

static const int PROPERTIES = sizeof(BarLine::propertyList)/sizeof(*BarLine::propertyList);

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
      _span = 1;
      yoff  = 0.0;
      setHeight(4.0 * spatium()); // for use in palettes
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF BarLine::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system = measure()->system();
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
      if (parent() && parent()->type() == SEGMENT) {
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            if (staffIdx2 >= score()->nstaves()) {
                  qDebug("BarLine: bad _span %d\n", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  }
            Segment* segment = static_cast<Segment*>(parent());
            Measure* measure = segment->measure();
            System* system   = measure->system();
            StaffLines* l1   = measure->staffLines(staffIdx1);
            StaffLines* l2   = measure->staffLines(staffIdx2);

            qreal yp = system ? system->staff(staffIdx())->y() : 0.0;
            *y1 = l1->y1() - yp;
            *y2 = l2->y2() - yp;
            }
      else {
            // for use in palette
            *y1 = 0.0;
            *y2 = 4.0 * spatium();
            }

      *y2 += yoff;
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
      else {
            System* s = measure()->system();
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
      xml.tag("span", _span);
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
            else if (tag == "span")
                  _span = val.toInt();
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
         || (type == ARTICULATION && segment() && segment()->subtype() == SegEndBarLine)
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
            Measure* m = segment()->measure();
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
      *grips   = 1;
      qreal lw = point(score()->styleS(ST_barWidth));
      qreal y1, y2;
      getY(&y1, &y2);
      grip[0].translate(QPointF(lw * .5, y2) + pagePos());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit()
      {
      if (staff()->barLineSpan() == _span)
            return;

      int idx1 = staffIdx();

      if (_span > staff()->barLineSpan()) {
            // if span increased, set span to 0 for all newly spanned staves
            int idx2 = idx1 + _span;
            for (int idx = idx1 + 1; idx < idx2; ++idx)
                  score()->undoChangeBarLineSpan(score()->staff(idx), 0);
            }
      else {
            // if span decreased, set span to 1 (the default) for all staves no longer spanned
            int idx1 = staffIdx() + _span;
            int idx2 = staffIdx() + staff()->barLineSpan();
            for (int idx = idx1; idx < idx2; ++idx)
                  score()->undoChangeBarLineSpan(score()->staff(idx), 1);
            }
      // update span for the staff the edited bar line belongs to
      score()->undoChangeBarLineSpan(staff(), _span);
      // added "_score->setLayoutAll(true);" to ChangeBarLineSpan::flip()
      // otherwise no measure bar line update occurs
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(const EditData& ed)
      {
      yoff += ed.delta.y();
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void BarLine::endEditDrag()
      {
      qreal y1, h2;
      getY(&y1, &h2);
      yoff      = 0.0;
      qreal ay1 = pagePos().y();
      qreal ay2 = ay1 + h2;

      int staffIdx1 = staffIdx();
      int staffIdx2;
      Segment* segment = (Segment*)parent();
      Measure* measure = segment->measure();
      System* s = measure->system();
      int n = s->staves()->size();
      if (staffIdx1 + 1 >= n)
            staffIdx2 = staffIdx1;
      else {
            qreal ay = s->pagePos().y();
            qreal y  = s->staff(staffIdx1)->y() + ay;
            qreal h1 = staff()->lines() * spatium();

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  qreal h = s->staff(staffIdx2)->y() + ay - y;
                  if (ay2 < (y + (h + h1) * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }
      int newSpan = staffIdx2 - staffIdx1 + 1;
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
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BarLine::layout()
      {
      qreal y1, y2;
      getY(&y1, &y2);
      qreal _spatium = spatium();
      qreal dw = score()->styleS(ST_barWidth).val() * _spatium;

      qreal dotwidth = symbols[score()->symIdx()][dotSym].width(magS());
      switch(subtype()) {
            case DOUBLE_BAR:
                  dw  = (score()->styleS(ST_doubleBarWidth) * 2
                     + score()->styleS(ST_doubleBarDistance)).val() * _spatium;
                  break;
            case START_REPEAT:
                  dw += dotwidth + (score()->styleS(ST_endBarWidth)
                     + 2 * score()->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case END_REPEAT:
                  dw += dotwidth + (score()->styleS(ST_endBarWidth)
                     + 2 * score()->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case END_BAR:
                  dw += (score()->styleS(ST_endBarWidth)
                     + score()->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case  END_START_REPEAT:
                  dw += 2 * dotwidth + (score()->styleS(ST_barWidth)
                     + score()->styleS(ST_endBarWidth)
                     + 4 * score()->styleS(ST_endBarDistance)).val() * _spatium;
                  break;
            case BROKEN_BAR:
            case NORMAL_BAR:
                  break;
            default:
                  qDebug("illegal bar line type\n");
                  break;
            }
      QRectF r(0.0, y1, dw, y2-y1);

      if (score()->styleB(ST_repeatBarTips)) {
            // qreal mags = magS();
            switch (subtype()) {
                  case START_REPEAT:
                        //r |= symbols[brackettipsRightUp].bbox(mags).translated(0, y1);
                        //r |= symbols[brackettipsRightDown].bbox(mags).translated(0, y2);
                        break;
                  case END_REPEAT:
                        //r |= symbols[brackettipsLeftUp].bbox(mags).translated(0, y1);
                        //r |= symbols[brackettipsLeftDown].bbox(mags).translated(0, y2);
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
      return segment() ? segment()->tick() : 0;
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
	e->setParent(this);
      switch(e->type()) {
            case ARTICULATION:
                  _el.append(e);
                  setGenerated(false);
                  if (measure())
                        measure()->setEndBarLineGenerated(false);
                  break;
            default:
                  qDebug("BarLine::add() not impl. %s\n", e->name());
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
//   property
//---------------------------------------------------------

Property<BarLine>* BarLine::property(P_ID id) const
      {
      for (int i = 0; i < PROPERTIES; ++i) {
            if (propertyList[i].id == id)
                  return &propertyList[i];
            }
      return 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(P_ID propertyId) const
      {
      Property<BarLine>* p = property(propertyId);
      if (p)
            return getVariant(propertyId, ((*(BarLine*)this).*(p->data))());
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(P_ID propertyId, const QVariant& v)
      {
      Property<BarLine>* p = property(propertyId);
      if (p) {
            setVariant(propertyId, ((*this).*(p->data))(), v);
            setGenerated(false);
            return true;
            }
      score()->setLayoutAll(true);
      return Element::setProperty(propertyId, v);
      }

bool BarLine::setProperty(const QString& name, const QDomElement& e)
      {
      for (int i = 0; i < PROPERTIES; ++i) {
            if (propertyName(propertyList[i].id) == name) {
                  QVariant v = ::getProperty(propertyList[i].id, e);
                  setVariant(propertyList[i].id, ((*this).*(propertyList[i].data))(), v);
                  setGenerated(false);
                  return true;
                  }
            }
      return Element::setProperty(name, e);
      }

