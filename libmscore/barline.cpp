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
#include "xml.h"
#include "marker.h"

namespace Ms {

//---------------------------------------------------------
//   static members init
//---------------------------------------------------------

qreal BarLine::yoff1 = 0.0;
qreal BarLine::yoff2 = 0.0;

bool BarLine::ctrlDrag = false;
bool BarLine::shiftDrag = false;
int  BarLine::_origSpan, BarLine::_origSpanFrom, BarLine::_origSpanTo;

//---------------------------------------------------------
//   barLineNames
//    must be synchronized with enum BarLineType
//---------------------------------------------------------

static const char* barLineNames[] = {
      QT_TRANSLATE_NOOP("barline", "normal"),
      QT_TRANSLATE_NOOP("barline", "double"),
      QT_TRANSLATE_NOOP("barline", "start-repeat"),
      QT_TRANSLATE_NOOP("barline", "end-repeat"),
      QT_TRANSLATE_NOOP("barline", "dashed"),
      QT_TRANSLATE_NOOP("barline", "end"),
      QT_TRANSLATE_NOOP("barline", "end-start-repeat"),
      QT_TRANSLATE_NOOP("barline", "dotted")
      };

const barLineTableItem barLineTable[] = {
        { BarLineType::NORMAL,           QT_TRANSLATE_NOOP("Palette", "Normal") },
        { BarLineType::BROKEN,           QT_TRANSLATE_NOOP("Palette", "Dashed style") },
        { BarLineType::DOTTED,           QT_TRANSLATE_NOOP("Palette", "Dotted style") },
        { BarLineType::END,              QT_TRANSLATE_NOOP("Palette", "End Bar style") },
        { BarLineType::DOUBLE,           QT_TRANSLATE_NOOP("Palette", "Double Bar style") },
        { BarLineType::START_REPEAT,     QT_TRANSLATE_NOOP("Palette", "Start Repeat") },
        { BarLineType::END_REPEAT,       QT_TRANSLATE_NOOP("Palette", "End Repeat") },
        { BarLineType::END_START_REPEAT, QT_TRANSLATE_NOOP("Palette", "End-Start Repeat") },
      };

unsigned int barLineTableSize()
      {
      return sizeof(barLineTable)/sizeof(*barLineTable);
      }

//---------------------------------------------------------
//   userTypeName
//---------------------------------------------------------

QString BarLine::userTypeName(BarLineType t)
      {
      return qApp->translate("barline", barLineNames[int(t)]);
      }

QString BarLine::userTypeName2(BarLineType t)
      {
      for (unsigned i = 0; i < sizeof(barLineTable)/sizeof(*barLineTable); ++i) {
           if(barLineTable[i].type == t)
                 return qApp->translate("Palette", barLineTable[i].name);
           }
      return QString();
      }


//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      _barLineType = BarLineType::NORMAL;
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
      if (parent()->type() != Element::Type::SEGMENT)
            system = static_cast<System*>(parent());
      else
            system = static_cast<Segment*>(parent())->measure()->system();

      qreal yp = y();
      if (system) {
            // get first not hidden staff
            int staffIdx1 = staffIdx();
            Staff* staff1 = score()->staff(staffIdx1);
            SysStaff* sysStaff1 = system->staff(staffIdx1);
            while ( staff1 && sysStaff1 && !(sysStaff1->show() && staff1->show()) ) {
                  staffIdx1++;
                  staff1 = score()->staff(staffIdx1);
                  sysStaff1 = system->staff(staffIdx1);
                  }
            yp += system->staffYpage(staffIdx1);
            }
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
            if (e->type() == Element::Type::PAGE) {
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
            if (parent()->type() == Element::Type::SEGMENT) {
                  Segment* segment = static_cast<Segment*>(parent());
                  measure = segment->measure();
                  system  = measure->system();
                  }
            else {
                  system  = static_cast<System*>(parent());
                  measure = system->firstMeasure();
                  }
            if (measure) {
                  // test start and end staff visibility
                  int   nstaves = score()->nstaves();
                  int   span = _span;
                  Staff* staff1 = score()->staff(staffIdx1);
                  Staff* staff2 = score()->staff(staffIdx2);
                  SysStaff* sysStaff1 = system->staff(staffIdx1);
                  SysStaff* sysStaff2 = system->staff(staffIdx2);
                  while (span > 0) {
                        // if start staff not shown, reduce span and move one staff down
                        if ( !(sysStaff1->show() && staff1->show()) ) {
                              span--;
                              if (staffIdx1 >= nstaves-1)         // running out of staves?
                                    break;
                              sysStaff1 = system->staff(++staffIdx1);
                              staff1    = score()->staff(staffIdx1);
                        }
                        // if end staff not shown, reduce span and move one staff up
                        else if ( !(sysStaff2->show() && staff2->show()) ) {
                              span--;
                              if (staffIdx2 == 0)
                                    break;
                              sysStaff2 = system->staff(--staffIdx2);
                              staff2    = score()->staff(staffIdx2);
                        }
                        // if both staves shown, exit loop
                        else
                              break;
                  }
                  // if no longer any span, set 0 length and exit
                  if (span <= 0) {
                        *y1 = *y2 = 0;
                        return;
                  }
                  // both staffIdx1 and staffIdx2 are shown: compute corresponding line length
                  StaffLines* l1 = measure->staffLines(staffIdx1);
                  StaffLines* l2 = measure->staffLines(staffIdx2);

                  if (system)
                        yp += sysStaff1->y();
                  *y1 = l1->y1() - yp;
                  *y1 += (_spanFrom * staff1->lineDistance() * staff1->spatium()) / 2;
                  *y2 = l2->y1() - yp;
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
      qreal _spatium = spatium();

      if (parent() == 0) {    // for use in palette
            drawSymbol(SymId::repeatDot, painter, QPointF(x, 2.0 * _spatium));
            drawSymbol(SymId::repeatDot, painter, QPointF(x, 3.0 * _spatium));
            }
      else if (parent()->type() == Element::Type::SEGMENT) {
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
                  qreal doty1   = (st->doty1() + .5) * _spatium;
                  qreal doty2   = (st->doty2() + .5) * _spatium;

                  qreal staffy  = s->staff(staffIdx1 + i)->y() - dy;

                  drawSymbol(SymId::repeatDot, painter, QPointF(x, staffy + doty1));
                  drawSymbol(SymId::repeatDot, painter, QPointF(x, staffy + doty2));
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(QPainter* painter) const
      {
      // get line length and do nothing if 0 (or near enough)
      qreal y1, y2;
      getY(&y1, &y2);
      if (y2-y1 < 0.1)
            return;

      qreal _spatium = score()->spatium();
      qreal lw = score()->styleS(StyleIdx::barWidth).val() * _spatium;

      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      switch(barLineType()) {
            case BarLineType::BROKEN:
                  pen.setStyle(Qt::DashLine);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case BarLineType::DOTTED:
                  pen.setStyle(Qt::DotLine);
                  painter->setPen(pen);

            case BarLineType::NORMAL:
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  break;

            case BarLineType::END:
                  {
                  qreal lw2 = score()->styleS(StyleIdx::endBarWidth).val() * _spatium;
                  qreal d   = score()->styleS(StyleIdx::endBarDistance).val() * _spatium;

                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  qreal x = d + lw2 * .5 + lw;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case BarLineType::DOUBLE:
                  {
                  lw      = point(score()->styleS(StyleIdx::doubleBarWidth));
                  qreal d = point(score()->styleS(StyleIdx::doubleBarDistance));

                  pen.setWidthF(lw);
                  painter->setPen(pen);
                  qreal x = lw * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  x += d + lw;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case BarLineType::START_REPEAT:
                  {
                  qreal lw2 = point(score()->styleS(StyleIdx::endBarWidth));
                  qreal d1  = point(score()->styleS(StyleIdx::endBarDistance));

                  qreal x2   =  lw2 * .5;                               // thick line (lw2)
                  qreal x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  qreal x0   =  lw2 + d1 + lw + d1;                     // dot position

                  drawDots(painter, x0);

                  painter->drawLine(QLineF(x1, y1, x1, y2));

                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x2, y1, x2, y2));

                  if (score()->styleB(StyleIdx::repeatBarTips)) {
                        drawSymbol(SymId::bracketTop, painter, QPointF(0.0, y1));
                        drawSymbol(SymId::bracketBottom, painter, QPointF(0.0, y2));
                        }
                  }
                  break;

            case BarLineType::END_REPEAT:
                  {
                  qreal lw2  = point(score()->styleS(StyleIdx::endBarWidth));
                  qreal d1   = point(score()->styleS(StyleIdx::endBarDistance));
                  qreal dotw = symWidth(SymId::repeatDot);
                  qreal x1   =  dotw + d1 + lw * .5;
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;

                  drawDots(painter, 0.0);
                  painter->drawLine(QLineF(x1, y1, x1, y2));
                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  painter->drawLine(QLineF(x2, y1, x2, y2));

                  if (score()->styleB(StyleIdx::repeatBarTips)) {
                        qreal x = x2 + lw2 * .5;
                        qreal w1 = symBbox(SymId::reversedBracketTop).width();
                        drawSymbol(SymId::reversedBracketTop, painter, QPointF(x - w1, y1));
                        drawSymbol(SymId::reversedBracketBottom, painter, QPointF(x - w1, y2));
                        }
                  }
                  break;

            case BarLineType::END_START_REPEAT:
                  {
                  qreal lw2  = point(score()->styleS(StyleIdx::endBarWidth));
                  qreal d1   = point(score()->styleS(StyleIdx::endBarDistance));
                  qreal dotw = symWidth(SymId::repeatDot);

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

                  if (score()->styleB(StyleIdx::repeatBarTips)) {
                        qreal x = x2;
                        qreal w1 = symBbox(SymId::reversedBracketTop).width();
                        drawSymbol(SymId::bracketTop, painter, QPointF(x, y1));
                        drawSymbol(SymId::bracketBottom, painter, QPointF(x, y2));
                        drawSymbol(SymId::reversedBracketTop, painter, QPointF(x - w1, y1));
                        drawSymbol(SymId::reversedBracketBottom, painter, QPointF(x - w1, y2));
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
                        BarLineType ct = BarLineType::NORMAL;
                        switch (i) {
                              default:
                              case  0: ct = BarLineType::NORMAL; break;
                              case  1: ct = BarLineType::DOUBLE; break;
                              case  2: ct = BarLineType::START_REPEAT; break;
                              case  3: ct = BarLineType::END_REPEAT; break;
                              case  4: ct = BarLineType::BROKEN; break;
                              case  5: ct = BarLineType::END; break;
                              case  6: ct = BarLineType::END_START_REPEAT; break;
                              case  7: ct = BarLineType::DOTTED; break;
                              }
                        _barLineType = ct;     // set type directly, without triggering setBarLineType() checks
                        }
                  if (parent() && parent()->type() == Element::Type::SEGMENT) {
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

bool BarLine::acceptDrop(const DropData& data) const
      {
      Element::Type type = data.element->type();
      if (type == Element::Type::BAR_LINE) {
            if (parent() && parent()->type() == Element::Type::SEGMENT)
                  return true;
            if (parent() && parent()->type() == Element::Type::SYSTEM) {
                  BarLine* b = static_cast<BarLine*>(data.element);
                  return (b->barLineType() == BarLineType::BROKEN || b->barLineType() == BarLineType::DOTTED
                     || b->barLineType() == BarLineType::NORMAL || b->barLineType() == BarLineType::DOUBLE
                     || b->spanFrom() != 0 || b->spanTo() != DEFAULT_BARLINE_TO);
                  }
            }
      else {
            return (type == Element::Type::ARTICULATION
               && parent()
               && parent()->type() == Element::Type::SEGMENT
               && static_cast<Segment*>(parent())->segmentType() == Segment::Type::EndBarLine);
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(const DropData& data)
      {
      Element* e = data.element;
      Element::Type type = e->type();
      if (type == Element::Type::BAR_LINE) {
            BarLine* bl = static_cast<BarLine*>(e);
            BarLineType st = bl->barLineType();
            // if no change in subtype or no change in span, do nothing
            if (st == barLineType() && bl->spanFrom() == 0 && bl->spanTo() == DEFAULT_BARLINE_TO) {
                  delete e;
                  return 0;
                  }
            // system left-side bar line
            if (parent()->type() == Element::Type::SYSTEM) {
                  BarLine* b = static_cast<System*>(parent())->barLine();
                  score()->undoChangeProperty(b, P_ID::SUBTYPE, int(bl->barLineType()));
                  delete e;
                  return 0;
                  }

            //parent is a segment
            Measure* m = static_cast<Segment*>(parent())->measure();

            // check if the new property can apply to this single bar line
            bool oldRepeat = (barLineType() == BarLineType::START_REPEAT || barLineType() == BarLineType::END_REPEAT
                        || barLineType() == BarLineType::END_START_REPEAT);
            bool newRepeat = (bl->barLineType() == BarLineType::START_REPEAT || bl->barLineType() == BarLineType::END_REPEAT
                        || bl->barLineType() == BarLineType::END_START_REPEAT);
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
                        score()->undoChangeProperty(this, P_ID::SUBTYPE, int(bl->barLineType()));
                        }
                  delete e;
                  return 0;
                  }

            // drop applies to all bar lines of the measure
            if (st == BarLineType::START_REPEAT) {
                  m = m->nextMeasure();
                  if (m == 0) {
                        delete e;
                        return 0;
                        }
                  }
            m->drop(data);
            }
      else if (type == Element::Type::ARTICULATION) {
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

void BarLine::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      *grips   = 2;
      *defaultGrip = 1;
      qreal lw = point(score()->styleS(StyleIdx::barWidth));
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
      shiftDrag = false;

      // if no change, do nothing
      if (_span == _origSpan &&_spanFrom == _origSpanFrom && _spanTo == _origSpanTo) {
            ctrlDrag = false;
            return;
            }
      // if bar line has custom span, assume any span edit is local to this bar line
      if (_customSpan == true)
            ctrlDrag = true;
      // if bar line belongs to a system (system-initial bar line), edit is local
      if (parent() && parent()->type() == Element::Type::SYSTEM)
            ctrlDrag = true;

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
      if (yoff1 == 0.0 && yoff2 == 0.0)         // if no drag, do nothing
            return;

      qreal y1, y2;
      getY(&y1, &y2);
      qreal ay0 = pagePos().y();
      qreal ay2 = ay0 + y2;                     // absolute (page-relative) bar line bottom coord
      int staffIdx1 = staffIdx();
      int staffIdx2;
      System* syst;
      if (parent()->type() == Element::Type::SYSTEM) {
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

      // determine new spanFrom and spanTo values
      int newSpanFrom, newSpanTo;
      Staff * staff2    = score()->staff(staffIdx2);
      int Staff1lines   = staff()->lines();
      int Staff2lines   = staff2->lines();

      if (shiftDrag) {                    // if precision dragging
            newSpanFrom = _spanFrom;
            if(yoff1 != 0.0) {
                  // round bar line top coord to nearest line of 1st staff (in half line dist units)
                  newSpanFrom = ((int)floor(y1 / (staff()->lineDistance() * spatium()) + 0.5 )) * 2;
                  // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
                  if(newSpanFrom <  MIN_BARLINE_SPAN_FROMTO)
                        newSpanFrom = MIN_BARLINE_SPAN_FROMTO;
                  if(newSpanFrom > Staff1lines*2)
                        newSpanFrom = Staff1lines*2;
                  }

            newSpanTo = _spanTo;
            if(yoff2 != 0.0) {
                  // round bar line bottom coord to nearest line of 2nd staff (in half line dist units)
                  qreal staff2TopY = systTopY + syst->staff(staffIdx2)->y();
                  newSpanTo = ((int)floor( (ay2 - staff2TopY) / (staff2->lineDistance() * spatium()) + 0.5 )) * 2;
                  // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
                  if(newSpanTo <  MIN_BARLINE_SPAN_FROMTO)
                        newSpanTo = MIN_BARLINE_SPAN_FROMTO;
                  if(newSpanTo > Staff2lines*2)
                        newSpanTo = Staff2lines*2;
                  }
//            shiftDrag = false;          // NO: a last call to this function is made when exiting editing:
      }                                   // it would find shiftDrag = false and reset extrema to coarse resolution

      else {                              // if coarse dragging
            newSpanFrom = 0;
            newSpanTo   = (Staff2lines - 1) * 2;
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
      qreal dw = score->styleS(StyleIdx::barWidth).val() * _spatium;

      qreal dotwidth = score->scoreFont()->width(SymId::repeatDot, mag);
      switch(type) {
            case BarLineType::DOUBLE:
                  dw  = (score->styleS(StyleIdx::doubleBarWidth) * 2
                     + score->styleS(StyleIdx::doubleBarDistance)).val() * _spatium;
                  break;
            case BarLineType::START_REPEAT:
                  dw += dotwidth + (score->styleS(StyleIdx::endBarWidth)
                     + 2 * score->styleS(StyleIdx::endBarDistance)).val() * _spatium;
                  break;
            case BarLineType::END_REPEAT:
                  dw += dotwidth + (score->styleS(StyleIdx::endBarWidth)
                     + 2 * score->styleS(StyleIdx::endBarDistance)).val() * _spatium;
                  break;
            case BarLineType::END:
                  dw += (score->styleS(StyleIdx::endBarWidth)
                     + score->styleS(StyleIdx::endBarDistance)).val() * _spatium;
                  break;
            case  BarLineType::END_START_REPEAT:
                  dw += 2 * dotwidth + (score->styleS(StyleIdx::barWidth)
                     + score->styleS(StyleIdx::endBarWidth)
                     + 4 * score->styleS(StyleIdx::endBarDistance)).val() * _spatium;
                  break;
            case BarLineType::BROKEN:
            case BarLineType::NORMAL:
            case BarLineType::DOTTED:
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
      if (parent() && parent()->type() != Element::Type::SYSTEM && staff() && !staff()->staffType()->showBarlines())
            setbbox(QRectF());

      // bar lines not hidden
      else {
            qreal dw = layoutWidth(score(), barLineType(), magS());
            QRectF r(0.0, y1, dw, y2-y1);

            if (score()->styleB(StyleIdx::repeatBarTips)) {
                  switch (barLineType()) {
                        case BarLineType::START_REPEAT:
                              r |= symBbox(SymId::bracketTop).translated(0, y1);
                              r |= symBbox(SymId::bracketBottom).translated(0, y2);
                              break;
                        case BarLineType::END_REPEAT:
                              {
                              qreal w1 = symBbox(SymId::reversedBracketTop).width();
                              r |= symBbox(SymId::reversedBracketTop).translated(dw - w1, y1);
                              r |= symBbox(SymId::reversedBracketBottom).translated(dw - w1, y2);
                              break;
                              }

                        case BarLineType::END_START_REPEAT:
                              {
                              qreal lw   = point(score()->styleS(StyleIdx::barWidth));
                              qreal lw2  = point(score()->styleS(StyleIdx::endBarWidth));
                              qreal d1   = point(score()->styleS(StyleIdx::endBarDistance));
                              qreal dotw = symWidth(SymId::repeatDot);
                              qreal x   =  dotw + 2 * d1 + lw + lw2 * .5;                     // thick bar
                              qreal w1 = symBbox(SymId::reversedBracketTop).width();
                              r |= symBbox(SymId::bracketTop).translated(x, y1);
                              r |= symBbox(SymId::bracketBottom).translated(x, y2);
                              r |= symBbox(SymId::reversedBracketTop).translated(x - w1 , y1);
                              r |= symBbox(SymId::reversedBracketBottom).translated(x - w1, y2);
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
            if (e->type() == Element::Type::ARTICULATION) {
                  Articulation* a       = static_cast<Articulation*>(e);
                  MScore::Direction dir = a->direction();
                  qreal distance        = 0.5 * spatium();
                  qreal x               = width() * .5;
                  if (dir == MScore::Direction::DOWN) {
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
      return (parent() && parent()->type() == Element::Type::SEGMENT)
         ? static_cast<Segment*>(parent())->tick() : 0;
      }

//---------------------------------------------------------
//   barLineTypeName
//---------------------------------------------------------

QString BarLine::barLineTypeName() const
      {
      return QString(barLineNames[int(barLineType())]);
      }

//---------------------------------------------------------
//   setBarLineType
//
//    Set the bar line type from the type name string.
//    Does not update _customSubtype or _generated flags: to be used when reading from a score file
//---------------------------------------------------------

void BarLine::setBarLineType(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(barLineNames)/sizeof(*barLineNames); ++i) {
            if (barLineNames[i] == s) {
                  _barLineType = BarLineType(i);
                  return;
                  }
            }
      _barLineType = BarLineType::NORMAL;
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
      if (parent() && parent()->type() != Element::Type::SEGMENT) {
            delete e;
            return;
            }
      e->setParent(this);
      switch(e->type()) {
            case Element::Type::ARTICULATION:
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
            case Element::Type::ARTICULATION:
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
//   updateCustomType
//
//    Turns off _customSubtype flag if bar line type is the same of the context it is in
//    (usually the endBarLineType of the measure); turns it on otherwise.
//---------------------------------------------------------

void BarLine::updateCustomType()
      {
      BarLineType refType = BarLineType::NORMAL;
      if (parent()) {
            if (parent()->type() == Element::Type::SEGMENT) {
                  Segment* seg = static_cast<Segment*>(parent());
                  switch (seg->segmentType()) {
                        case Segment::Type::StartRepeatBarLine:
                              // if a start-repeat segment, ref. type is START_REPEAT
                              // if measure has relevant repeat flag or none if measure hasn't
                              refType = (seg->measure()->repeatFlags() & Repeat::START) != 0
                                          ? BarLineType::START_REPEAT : BarLineType(-1);
                              break;
                        case Segment::Type::BarLine:
                              // if a non-end-measure bar line, type is always custom
                              refType = BarLineType(-1);           // use an invalid type
                              break;
                        case Segment::Type::EndBarLine:
                              // if end-measure bar line, reference type is the measure endBarLinetype
                              refType = seg->measure()->endBarLineType();
                              break;
                        default:                      // keep lint happy!
                              break;
                        }
                  }
            // if parent is not a segment, it can only be a system and NORMAL can be used as ref. type
            }
      _customSubtype = (_barLineType != refType);
      updateGenerated(!_customSubtype);         // if _customSubType, _genereated is surely false
      }

//---------------------------------------------------------
//   updateGenerated
//
//    Sets the _generated status flag by checking all the bar line properties are at default values.
//
//    canBeTrue: optional parameter; if set to false, the _generated flag is unconditionally set to false
//          without checking the individual properties; to be used when a non-default condition is already known
//          to speed up the function.
//---------------------------------------------------------

void BarLine::updateGenerated(bool canBeTrue)
      {
      if (!canBeTrue)
            setGenerated(false);
      else {
            bool generatedType = !_customSubtype;     // if customSubType, assume not generated
            if (parent()) {
                  if (parent()->type() == Element::Type::SEGMENT) {
                        // if bar line belongs to an EndBarLine segment,
                        // combine with measure endBarLineGenerated flag
                        if (static_cast<Segment*>(parent())->segmentType() == Segment::Type::EndBarLine)
                              generatedType &= static_cast<Segment*>(parent())->measure()->endBarLineGenerated();
                        // if any other segment (namely, StartBarLine and BarLine), bar line is not generated
                        else
                              generatedType = false;
                  }
                  // if bar line does not belongs to a segment, it belongs to a system and is generated only if NORMAL
                  else
                        generatedType = (_barLineType == BarLineType::NORMAL);
            }
            // set to generated only if all properties are non-customized
            setGenerated(
                  color()           == MScore::defaultColor
                  && _visible       == true
                  && generatedType  == true
                  && _customSpan    == false
                  );
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::SUBTYPE:
                  return int(_barLineType);
            case P_ID::BARLINE_SPAN:
                  return span();
            case P_ID::BARLINE_SPAN_FROM:
                  return spanFrom();
            case P_ID::BARLINE_SPAN_TO:
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
            case P_ID::SUBTYPE:
                  setBarLineType(BarLineType(v.toInt()));
                  break;
            case P_ID::BARLINE_SPAN:
                  setSpan(v.toInt());
                  break;
            case P_ID::BARLINE_SPAN_FROM:
                  setSpanFrom(v.toInt());
                  break;
            case P_ID::BARLINE_SPAN_TO:
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
            case P_ID::SUBTYPE:
                  // default subtype is the subtype of the measure, if any
                  if (parent() && parent()->type() == Element::Type::SEGMENT && static_cast<Segment*>(parent())->measure() )
                      return int(static_cast<Segment*>(parent())->measure()->endBarLineType());
                  return int(BarLineType::NORMAL);
            case P_ID::BARLINE_SPAN:
                  // if there is a staff, default span is staff span
                  if (staff())
                        return staff()->barLineSpan();
                  // if no staff, default span is 1
                  return 1;
            case P_ID::BARLINE_SPAN_FROM:
                  // if there is a staff, default From span is staff From span
                  if (staff())
                        return staff()->barLineFrom();
                  // if no staff, default From is from top
                  return 0;
            case P_ID::BARLINE_SPAN_TO:
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

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* BarLine::nextElement()
      {
      if (parent()->type() == Element::Type::SEGMENT)
            return static_cast<Segment*>(parent())->firstInNextSegments(score()->inputState().prevTrack() / VOICES);

      return parent()->nextElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* BarLine::prevElement()
      {
      if (parent()->type() == Element::Type::SEGMENT)
            return static_cast<Segment*>(parent())->lastInPrevSegments(score()->inputState().prevTrack() / VOICES);

      return parent()->prevElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString BarLine::accessibleInfo()
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(BarLine::userTypeName2(this->barLineType()));
      }

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString BarLine::accessibleExtraInfo()
      {
      if (parent()->type() == Element::Type::SEGMENT) {
            Segment* seg = static_cast<Segment*>(parent());
            QString rez = "";

            foreach(Element* e, *el())
                  rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());

            foreach (Element* e, seg->annotations()) {
                  if (e->track() == track())
                        rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                  }
            Measure* m = seg->measure();

            if (m) {
                  //jumps
                  foreach (Element* e, *m->el()) {
                        if(e->type() == Element::Type::JUMP)
                              rez= QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                        if(e->type() == Element::Type::MARKER) {
                              Marker* m = static_cast<Marker*>(e);
                              if (m->markerType() == Marker::Type::FINE)
                                    rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                              }

                        }
                  //markers
                  Measure* nextM = m->nextMeasureMM();
                  if (nextM) {
                        foreach (Element* e, *nextM->el()) {
                              if(e->type() == Element::Type::MARKER)
                                    if(static_cast<Marker*>(e)->markerType() == Marker::Type::FINE)
                                          continue; //added above^
                                    rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                              }
                        }
                  }

            int tick = seg->tick();

            std::vector< ::Interval<Spanner*> > spanners = score()->spannerMap().findOverlapping(tick, tick);
            for (std::vector< ::Interval<Spanner*> >::iterator i = spanners.begin(); i < spanners.end(); i++) {
                  ::Interval<Spanner*> interval = *i;
                  Spanner* s = interval.value;
                  if (s->type() == Element::Type::VOLTA) {
                        if(s->tick() == tick)
                              rez = tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                        if(s->tick2() == tick)
                              rez = tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                        }
                  }
            return rez;
            }

      return Element::accessibleExtraInfo();
      }

}

