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
int  BarLine::_origSpan;
int BarLine::_origSpanFrom;
int BarLine::_origSpanTo;

//---------------------------------------------------------
//   BarLineTable
//---------------------------------------------------------

const std::vector<BarLineTableItem> BarLine::barLineTable {
      { BarLineType::NORMAL,           QT_TRANSLATE_NOOP("Palette", "Normal barline"),   "normal" },
      { BarLineType::DOUBLE,           QT_TRANSLATE_NOOP("Palette", "Double barline"),   "double" },
      { BarLineType::START_REPEAT,     QT_TRANSLATE_NOOP("Palette", "Start repeat"),     "start-repeat" },
      { BarLineType::END_REPEAT,       QT_TRANSLATE_NOOP("Palette", "End repeat"),       "end-repeat" },
      { BarLineType::BROKEN,           QT_TRANSLATE_NOOP("Palette", "Dashed barline"),   "dashed" },
      { BarLineType::END,              QT_TRANSLATE_NOOP("Palette", "Final barline"),    "end" },
      { BarLineType::END_START_REPEAT, QT_TRANSLATE_NOOP("Palette", "End-start repeat"), "end-start-repeat" },
      { BarLineType::DOTTED,           QT_TRANSLATE_NOOP("Palette", "Dotted barline"),   "dotted" },
      };

//---------------------------------------------------------
//   barLineTableItem
//---------------------------------------------------------

const BarLineTableItem* BarLine::barLineTableItem(unsigned i)
      {
      if (i >= barLineTable.size())
            return 0;
      return &barLineTable[i];
      }

//---------------------------------------------------------
//   userTypeName
//---------------------------------------------------------

QString BarLine::userTypeName(BarLineType t)
      {
      for (const auto& i : barLineTable) {
           if (i.type == t)
                 return qApp->translate("Palette", i.userName);
           }
      return QString();
      }

//---------------------------------------------------------
//   barLineTypeName
//
//    Instance form returning the name string of the bar line type and
//    static form returning the name string for an arbitrary bar line type.
//---------------------------------------------------------

QString BarLine::barLineTypeName() const
      {
      return barLineTypeName(barLineType());
      }

QString BarLine::barLineTypeName(BarLineType t)
      {
      for (const auto& i : barLineTable) {
           if (i.type == t)
                 return i.name;
            }
      return QString("??");
      }

//---------------------------------------------------------
//   setBarLineType
//
//    Set the bar line type from the type name string.
//    Does not update _customSubtype or _generated flags: to be used when reading from a score file
//---------------------------------------------------------

void BarLine::setBarLineType(const QString& s)
      {
      _barLineType = barLineType(s);
      }

//---------------------------------------------------------
//   barLineType
//---------------------------------------------------------

BarLineType BarLine::barLineType(const QString& s)
      {
      for (const auto& i : barLineTable) {
            if (i.name == s)
                  return i.type;
            }
      return BarLineType::NORMAL;   // silent default
      }

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setHeight(DEFAULT_BARLINE_TO/2 * spatium()); // for use in palettes
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal BarLine::mag() const
      {
      return staff() ? staff()->mag() : 1.0;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF BarLine::pagePos() const
      {
      if (segment() == 0)
            return pos();
      System* system = segment()->measure()->system();

      qreal yp = y();
      if (system) {
            // get first not hidden staff
            int startIdx = staffIdx();
            int endIdx = startIdx + span();
            int staffIdx1 = startIdx;
            Staff* staff1 = score()->staff(staffIdx1);
            SysStaff* sysStaff1 = system->staff(staffIdx1);
            while (staff1 && sysStaff1 && !(sysStaff1->show() && staff1->show())) {
                  if (++staffIdx1 >= endIdx) {
                        // no visible staves spanned; just use first
                        staffIdx1 = startIdx;
                        break;
                        }
                  staff1 = score()->staff(staffIdx1);
                  sysStaff1 = system->staff(staffIdx1);
                  }
            yp += system->staffYpage(staffIdx1);
            }
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

void BarLine::getY(qreal* y1, qreal* y2) const
      {
      qreal _spatium = spatium();
      int   span = _span;
      if (parent()) {
            int staffIdx1    = staffIdx();
            int staffIdx2    = staffIdx1 + _span - 1;
            if (staffIdx2 >= score()->nstaves()) {
                  qDebug("BarLine: bad _span %d", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  }
            Measure* measure = 0;
            System* system   = 0;
            SysStaff* sysStaff0 = 0;      // top staff for barline in system
            bool systemBarLine;
            if (parent()->type() == Element::Type::SEGMENT) {
                  measure = segment()->measure();
                  system  = measure->system();
                  if (system)
                        sysStaff0 = system->staff(staffIdx1);
                  systemBarLine = false;
                  }
            else {
                  sysStaff0 = segment()->measure()->system()->staff(staffIdx1);
                  measure = segment()->measure()->system()->firstMeasure();
                  for (int i = staffIdx1; i < staffIdx2; ++i) {
                        if (!score()->staff(i)->hideSystemBarLine()) {
                              span -= (i - staffIdx1);
                              staffIdx1 = i;
                              break;
                              }
                        }
                  systemBarLine = true;
                  }
            if (measure) {
                  // test start and end staff visibility
                  int nstaves = score()->nstaves();
                  Staff* staff1 = score()->staff(staffIdx1);
                  Staff* staff2 = score()->staff(staffIdx2);
                  SysStaff* sysStaff1  = 0;
                  SysStaff* sysStaff1a = 0;     // first staff that is shown, even if it has invisible measures
                  if (system) {
                        sysStaff1 = system->staff(staffIdx1);
                        SysStaff* sysStaff2 = system->staff(staffIdx2);
                        Measure* nm = measure->nextMeasure();
                        if (nm && nm->system() != measure->system())
                              nm = nullptr;
                        while (span > 0) {
                              bool show1 = sysStaff1->show() && staff1->show();
                              // if start staff not shown, reduce span and move one staff down
                              if (!(show1 && (measure->visible(staffIdx1) || (nm && nm->visible(staffIdx1))))) {
                                    span--;
                                    if (show1 && !sysStaff1a)
                                          sysStaff1a = sysStaff1;       // use for its y offset
                                    if (staffIdx1 >= nstaves-1)         // running out of staves?
                                          break;
                                    sysStaff1 = system->staff(++staffIdx1);
                                    staff1    = score()->staff(staffIdx1);
                                    }
                              // if end staff not shown, reduce span and move one staff up
                              else if (!(sysStaff2->show() && staff2->show() && (measure->visible(staffIdx2) || (nm && nm->visible(staffIdx2))))) {
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
                        }
                  // if no longer any span, set 0 length and exit
                  if (span <= 0) {
                        *y1 = *y2 = 0;
                        return;
                        }
                  // both staffIdx1 and staffIdx2 are shown: compute corresponding line length
                  StaffLines* l1 = measure->staffLines(staffIdx1);
                  StaffLines* l2 = measure->staffLines(staffIdx2);

                  qreal yp = 0.0;
                  if (systemBarLine) {
                        // system initial barline, parent is system
                        // base y on top staff for barline
                        // system barline span already accounts for staff visibility
                        yp = sysStaff0->y();
                        }
                  else if (system) {
                        // ordinary barline within system, parent is measure
                        // base y on top visible staff in barline span
                        // after skipping ones with hideSystemBarLine set
                        // and accounting for staves that are shown but have invisible measures
                        yp = sysStaff1a ? sysStaff1a->y() : sysStaff1->y();
                        }
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
      else {
            System* system = segment()->measure()->system();
            int staffIdx1  = staffIdx();
            // find first visible staff
            Staff* staff1 = score()->staff(staffIdx1);
            SysStaff* sysStaff1 = system->staff(staffIdx1);
            while ( staff1 && sysStaff1 && !(sysStaff1->show() && staff1->show()) ) {
                  staffIdx1++;
                  staff1 = score()->staff(staffIdx1);
                  sysStaff1 = system->staff(staffIdx1);
                  }
            int staffIdx2    = staffIdx1 + _span - 1;
            int sp = _span;
            if (staffIdx2 >= score()->nstaves()) {
                  qDebug("BarLine: bad _span %d", _span);
                  staffIdx2 = score()->nstaves() - 1;
                  sp = staffIdx2 - staffIdx1 + 1;
                  }
            qreal dy  = sysStaff1->y();
            for (int i = 0; i < sp; ++i) {
                  Staff* staff  = score()->staff(staffIdx1 + i);
                  SysStaff* sysStaff = system->staff(staffIdx1 + i);
                  if (sysStaff->show()) {
                        StaffType* st = staff->staffType();
                        qreal doty1   = (st->doty1() + .5) * _spatium;
                        qreal doty2   = (st->doty2() + .5) * _spatium;

                        qreal staffy  = sysStaff->y() - dy;

                        drawSymbol(SymId::repeatDot, painter, QPointF(x, staffy + doty1));
                        drawSymbol(SymId::repeatDot, painter, QPointF(x, staffy + doty2));
                        }
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

      qreal _mag = (score()->styleB(StyleIdx::scaleBarlines) && staff()) ? staff()->mag() : 1.0;

      qreal lw = score()->styleP(StyleIdx::barWidth) * _mag;

      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      switch (barLineType()) {
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
                  qreal lw2 = score()->styleP(StyleIdx::endBarWidth) * _mag;
                  qreal d   = score()->styleP(StyleIdx::endBarDistance) * _mag;

                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  pen.setWidthF(lw2);
                  painter->setPen(pen);
                  qreal x = d + lw2 * .5 + lw;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case BarLineType::DOUBLE:
                  {
                  lw      = score()->styleP(StyleIdx::doubleBarWidth) * _mag;
                  qreal d = score()->styleP(StyleIdx::doubleBarDistance) * _mag;

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
                  qreal lw2 = score()->styleP(StyleIdx::endBarWidth) * _mag;
                  qreal d1  = score()->styleP(StyleIdx::endBarDistance) * _mag;
                  qreal d2  = score()->styleP(StyleIdx::repeatBarlineDotSeparation) * _mag;

                  qreal x2   =  lw2 * .5;                               // thick line (lw2)
                  qreal x1   =  lw2 + d1 + lw * .5;                     // thin line (lw)
                  qreal x0   =  lw2 + d2 + lw + d1;                     // dot position

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
                  qreal lw2  = score()->styleP(StyleIdx::endBarWidth) * _mag;
                  qreal d1  = score()->styleP(StyleIdx::repeatBarlineDotSeparation) * _mag;
                  qreal d2   = score()->styleP(StyleIdx::endBarDistance) * _mag;
                  qreal dotw = symWidth(SymId::repeatDot);
                  qreal x1   =  dotw + d1 + lw * .5;
                  qreal x2   =  dotw + d2 + lw + d1 + lw2 * .5;

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
                  qreal lw2  = score()->styleP(StyleIdx::endBarWidth) * _mag;
                  qreal d1   = score()->styleP(StyleIdx::endBarDistance) * _mag;
                  qreal d2  = score()->styleP(StyleIdx::repeatBarlineDotSeparation) * _mag;
                  qreal dotw = symWidth(SymId::repeatDot);

                  qreal x1   =  dotw + d2 + lw * .5;                                // thin bar
                  qreal x2   =  dotw + d1 + lw + d1 + lw2 * .5;                     // thick bar
                  qreal x3   =  dotw + d1 + lw + d1 + lw2 + d1 + lw * .5;           // thin bar
                  qreal x4   =  dotw + d2 + lw + d1 + lw2 + d1 + lw + d1;           // dot position

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
      Segment* s = segment();
      if (s && !score()->printing()) {
            Measure* m = s->measure();
            if (s && s->isEndBarLineType() && m->isIrregular() && score()->markIrregularMeasures() && !m->isMMRest()) {
                  painter->setPen(MScore::layoutBreakColor);
                  QFont f("FreeSerif");
                  f.setPointSizeF(12 * spatium() * MScore::pixelRatio / SPATIUM20);
                  f.setBold(true);
                  QString str = m->len() > m->timesig() ? "+" : "-";
                  QRectF r = QFontMetricsF(f, MScore::paintDevice()).boundingRect(str);
                  painter->setFont(f);
                  painter->drawText(-r.width(), 0.0, str);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(Xml& xml) const
      {
      xml.stag("BarLine");
      xml.tag("subtype", barLineTypeName());

      // if any span value is different from staff's, output all values
      // (palette bar lines have no staff: output all values)

      if (!staff() || customSpan())
            xml.tag(QString("span from=\"%1\" to=\"%2\"").arg(_spanFrom).arg(_spanTo), _span);
      else
            xml.tag("span", _span);
      for (const Element* e : _el)
            e->write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(XmlReader& e)
      {
      resetProperty(P_ID::BARLINE_SPAN);
      resetProperty(P_ID::BARLINE_SPAN_FROM);
      resetProperty(P_ID::BARLINE_SPAN_TO);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setBarLineType(e.readElementText());
            else if (tag == "customSubtype")                      // obsolete
                  e.readInt();
            else if (tag == "span") {
                  _spanFrom   = e.intAttribute("from", _spanFrom);
                  _spanTo     = e.intAttribute("to", _spanTo);
                  _span       = e.readInt();
                  _customSpan = !staff() || custom(P_ID::BARLINE_SPAN)
                                 || custom(P_ID::BARLINE_SPAN_FROM) || custom(P_ID::BARLINE_SPAN_TO);
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
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(const DropData& data) const
      {
      Element::Type type = data.element->type();
      if (type == Element::Type::BAR_LINE)
            return true;
      else {
            return (type == Element::Type::ARTICULATION
               && segment()
               && segment()->isEndBarLineType());
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
            BarLine* bl    = toBarLine(e);
            BarLineType st = bl->barLineType();

            // if no change in subtype or no change in span, do nothing
            if (st == barLineType() && bl->spanFrom() == 0 && bl->spanTo() == DEFAULT_BARLINE_TO) {
                  delete e;
                  return 0;
                  }
            // check if the new property can apply to this single bar line
            BarLineType bt = BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT;
            bool oldRepeat = barLineType() & bt;
            bool newRepeat = bl->barLineType() & bt;

            // if ctrl was used and repeats are not involved,
            // or if drop refers to span rather than subtype =>
            // single bar line drop

            if ((data.control() && !oldRepeat && !newRepeat) || (bl->spanFrom() != 0 || bl->spanTo() != DEFAULT_BARLINE_TO) ) {
                  // if drop refers to span, update this bar line span
                  if (bl->spanFrom() != 0 || bl->spanTo() != DEFAULT_BARLINE_TO) {
                        // if dropped spanFrom or spanTo are below the middle of standard staff (5 lines)
                        // adjust to the number of syaff lines
                        int bottomSpan = (staff()->lines()-1) * 2;
                        int spanFrom   = bl->spanFrom() > 4 ? bottomSpan - (8 - bl->spanFrom()) : bl->spanFrom();
                        int spanTo     = bl->spanTo() > 4 ? bottomSpan - (8 - bl->spanTo()) : bl->spanTo();
                        score()->undoChangeSingleBarLineSpan(this, 1, spanFrom, spanTo);
                        }
                  // if drop refers to subtype, update this bar line subtype
                  else
                        undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(bl->barLineType()));
                  delete e;
                  return 0;
                  }

            //---------------------------------------------
            //    Update repeat flags for current measure
            //    and next measure if this is a EndBarLine.
            //---------------------------------------------

            if (segment()->isEndBarLineType()) {
                  Measure* m  = segment()->measure();
                  if (st == BarLineType::START_REPEAT) {
                        if (m->nextMeasureMM())
                              score()->undoChangeBarLine(m->nextMeasureMM(), st);
                        }
                  else
                        score()->undoChangeBarLine(m, st);
                  }
            else if (segment()->isBeginBarLineType()) {
                  undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(st));
                  undoChangeProperty(P_ID::GENERATED, false);
                  }

            delete e;
            return 0;
            }

      else if (type == Element::Type::ARTICULATION) {
            Articulation* atr = toArticulation(e);
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

void BarLine::updateGrips(Grip* defaultGrip, QVector<QRectF>& grip) const
      {
      *defaultGrip = Grip::END;
      qreal lw = score()->styleP(StyleIdx::barWidth) * staff()->mag();
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
      _origSpan     = _span;
      _origSpanFrom = _spanFrom;
      _origSpanTo   = _spanTo;
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
      if (_customSpan)
            ctrlDrag = true;
      // for mid-measure barlines, edit is local
      bool midMeasure = false;
      if (segment()->isBarLine()) {
            ctrlDrag = true;
            midMeasure = true;
            }

      if (ctrlDrag) {                           // if single bar line edit
            int newSpan       = _span;          // copy edited span values
            int newSpanFrom   = _spanFrom;
            int newSpanTo     = _spanTo;
            _span             = _origSpan;      // restore original span values
            _spanFrom         = _origSpanFrom;
            _spanTo           = _origSpanTo;
            _customSpan       = true;
            // for mid-measure barline in root score, update parts
            if (midMeasure && score()->isMaster() && score()->excerpts().size() > 0) {
                  int currIdx = staffIdx();
                  Measure* m = segment()->measure();
                  // change linked barlines as necessary
                  int lastIdx = currIdx + qMax(_span, newSpan);
                  for (int idx = currIdx; idx < lastIdx; ++idx) {
                        Staff* staff = score()->staff(idx);
                        LinkedStaves* ls = staff->linkedStaves();
                        if (ls) {
                              for (Staff* lstaff : ls->staves()) {
                                    Score* lscore = lstaff->score();
                                    // don't change barlines in root score
                                    if (lscore == staff->score())
                                          continue;
                                    // change barline only in top staff of part
                                    if (lstaff != lscore->staff(0))
                                          continue;
                                    int spannedStaves = qMax(currIdx + newSpan - idx, 0);
                                    int lNewSpan = qMin(spannedStaves, lscore->nstaves());
                                    Measure* lm = lscore->tick2measure(m->tick());
                                    Segment* lseg = lm->undoGetSegment(Segment::Type::BarLine, tick());
                                    BarLine* lbl = toBarLine(lseg->element(0));
                                    if (lbl) {
                                          // already a barline here
                                          if (lNewSpan > 0) {
                                                // keep barline, but update span if necessary
                                                if (lbl->span() != lNewSpan)
                                                      lbl->undoChangeProperty(P_ID::BARLINE_SPAN, lNewSpan);
                                                }
                                          else {
                                                // remove barline
                                                lbl->unlink();
                                                lbl->score()->undoRemoveElement(lbl);
                                                }
                                          }
                                    else {
                                          // new barline needed
                                          lbl = toBarLine(linkedClone());
                                          lbl->setSpan(lNewSpan);
                                          lbl->setTrack(lstaff->idx() * VOICES);
                                          lbl->setScore(lscore);
                                          lbl->setParent(lseg);
                                          lscore->undoAddElement(lbl);
                                          }
                                    }
                              }
                        }
                  }
            score()->undoChangeSingleBarLineSpan(this, newSpan, newSpanFrom, newSpanTo);
            return;
            }

      // if same as staff settings, do nothing
      if (staff()->barLineSpan() == _span && staff()->barLineFrom() == _spanFrom && staff()->barLineTo() == _spanTo)
            return;

      int idx1 = staffIdx();

      if (_span != staff()->barLineSpan()) {
            // if now bar lines span more staves
            if (_span > staff()->barLineSpan()) {
                  int idx2 = idx1 + _span;
                  // set span 0 to all additional staves
                  for (int idx = idx1 + 1; idx < idx2; ++idx) {
                        // Mensurstrich special case:
                        // if line spans to top line of a stave AND current staff is
                        //    the last spanned staff BUT NOT the last score staff
                        //          keep its bar lines
                        // otherwise remove them
                        if (_spanTo > 0 || !(idx == idx2-1 && idx != score()->nstaves()-1)) {
                              Staff* staff = score()->staff(idx);
                              staff->undoChangeProperty(P_ID::BARLINE_SPAN,      0);
                              staff->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, 0);
                              staff->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   (staff->lines()-1)*2);
                              }
                        }
                  }
            // if now bar lines span fewer staves
            else {
                  int idx1 = staffIdx() + _span;
                  int idx2 = staffIdx() + staff()->barLineSpan();
                  // set standard span for each no-longer-spanned staff
                  for (int idx = idx1; idx < idx2; ++idx) {
                        Staff* staff = score()->staff(idx);
                        int lines = staff->lines();
                        int spanFrom = lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
                        int spanTo = lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (lines - 1) * 2;
                        staff->undoChangeProperty(P_ID::BARLINE_SPAN,      1);
                        staff->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, spanFrom);
                        staff->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   spanTo);
                        }
                  }
            }

      // update span for the staff the edited bar line belongs to
      staff()->undoChangeProperty(P_ID::BARLINE_SPAN,      _span);
      staff()->undoChangeProperty(P_ID::BARLINE_SPAN_FROM, _spanFrom);
      staff()->undoChangeProperty(P_ID::BARLINE_SPAN_TO,   _spanTo);
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
      if (ed.curGrip == Grip::START) {
            // min offset for top grip is line -1 (-2 for 1-line staves)
            // max offset is 1 line above bottom grip or 1 below last staff line, whichever comes first
            min = -y1 - (staff()->lines() == 1 ? lineDist * 2 : lineDist);
            max = y2 - y1 - lineDist;                                   // 1 line above bottom grip
            lastmax = (staff()->lines() - _spanFrom/2) * lineDist;      // 1 line below last staff line
            if (lastmax < max)
                  max = lastmax;
            // update yoff1 and bring it within limits
            yoff1 += ed.delta.y();
            if (yoff1 < min)
                  yoff1 = min;
            if (yoff1 > max)
                  yoff1 = max;
            }
      else {
            // min for bottom grip is 1 line below top grip
            // no max
            min = y1 - y2 + lineDist;
            // update yoff2 and bring it within limit
            yoff2 += ed.delta.y();
            if (yoff2 < min)
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
      System* syst   = segment()->measure()->system();
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
      int staff1lines   = staff()->lines();
      int staff2lines   = staff2->lines();

      if (shiftDrag) {                    // if precision dragging
            newSpanFrom = _spanFrom;
            if (yoff1 != 0.0) {
                  // round bar line top coord to nearest line of 1st staff (in half line dist units)
                  newSpanFrom = ((int)floor(y1 / (staff()->lineDistance() * spatium()) + 0.5 )) * 2;
                  // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
                  // except for 1-line staves
                  int minFrom = staff1lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
                  if (newSpanFrom <  minFrom)
                        newSpanFrom = minFrom;
                  if (newSpanFrom > staff1lines * 2)
                        newSpanFrom = staff1lines * 2;
                  }

            newSpanTo = _spanTo;
            if (yoff2 != 0.0) {
                  // round bar line bottom coord to nearest line of 2nd staff (in half line dist units)
                  qreal staff2TopY = systTopY + syst->staff(staffIdx2)->y();
                  newSpanTo = ((int)floor( (ay2 - staff2TopY) / (staff2->lineDistance() * spatium()) + 0.5 )) * 2;
                  // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
                  int maxTo = staff2lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : staff2lines * 2;
                  if (newSpanTo <  MIN_BARLINE_SPAN_FROMTO)
                        newSpanTo = MIN_BARLINE_SPAN_FROMTO;
                  if (newSpanTo > maxTo)
                        newSpanTo = maxTo;
                  }
            }

      else {                              // if coarse dragging
            newSpanFrom = staff1lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM: 0;
            newSpanTo   = staff2lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staff2lines - 1) * 2;
            }

      // if any value changed, update
      if (newSpan != _span || newSpanFrom != _spanFrom || newSpanTo != _spanTo) {
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
      if (!score->styleB(StyleIdx::scaleBarlines))
            mag = 0.0;
      qreal dw = score->styleP(StyleIdx::barWidth) * mag;

      qreal dotwidth = score->scoreFont()->width(SymId::repeatDot, mag);
      switch (type) {
            case BarLineType::DOUBLE:
                  dw  = (score->styleP(StyleIdx::doubleBarWidth) * 2
                     + score->styleP(StyleIdx::doubleBarDistance)) * mag;
                  break;
            case BarLineType::START_REPEAT:
                  dw += dotwidth + (score->styleP(StyleIdx::endBarWidth)
                     +  (score->styleP(StyleIdx::repeatBarlineDotSeparation) + score->styleP(StyleIdx::endBarDistance))) * mag;
                  break;
            case BarLineType::END_REPEAT:
                  dw += dotwidth + (score->styleP(StyleIdx::endBarWidth)
                     + (score->styleP(StyleIdx::repeatBarlineDotSeparation) + score->styleP(StyleIdx::endBarDistance))) * mag;
                  break;
            case BarLineType::END:
                  dw += (score->styleP(StyleIdx::endBarWidth)
                     + score->styleP(StyleIdx::endBarDistance)) * mag;
                  break;
            case  BarLineType::END_START_REPEAT:
                  dw += 2 * dotwidth + (score->styleP(StyleIdx::barWidth)
                     + score->styleP(StyleIdx::endBarWidth)
                     + 2 * (score->styleP(StyleIdx::repeatBarlineDotSeparation) + score->styleP(StyleIdx::endBarDistance))) * mag;
                  break;
            case BarLineType::BROKEN:
            case BarLineType::NORMAL:
            case BarLineType::DOTTED:
                  break;
            default:
                  qDebug("illegal barline type %d", int(type));
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

      // if bar line has a staff and staff is set to hide bar lines, set null bbox
      if (staff() && !staff()->staffType()->showBarlines())
            setbbox(QRectF());
      else {
            // bar lines not hidden
            qreal dw = layoutWidth(score(), barLineType(), mag());
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
                              qreal lw   = score()->styleP(StyleIdx::barWidth);
                              qreal lw2  = score()->styleP(StyleIdx::endBarWidth);
                              qreal d1   = score()->styleP(StyleIdx::endBarDistance);
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
      for (Element* e : _el) {
            e->layout();
            if (e->isArticulation()) {
                  Articulation* a       = toArticulation(e);
                  Direction dir = a->direction();
                  qreal distance        = 0.5 * spatium();
                  qreal x               = width() * .5;
                  if (dir == Direction::DOWN) {
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

QPainterPath BarLine::outline() const
      {
      QPainterPath p;
      qreal d = spatium() * .3;
      p.addRect(bbox().adjusted(-d, .0, d, .0));
      return p;
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
      for (Element* e : _el)
            e->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BarLine::add(Element* e)
      {
      e->setParent(this);
      switch (e->type()) {
            case Element::Type::ARTICULATION:
                  _el.push_back(e);
                  setGenerated(false);
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
//   getProperty
//---------------------------------------------------------

QVariant BarLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::BARLINE_TYPE:
                  return QVariant::fromValue(_barLineType);
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
      switch (id) {
            case P_ID::BARLINE_TYPE:
                  setBarLineType(v.value<BarLineType>());
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
      setGenerated(false);
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant BarLine::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::BARLINE_TYPE:
                  if (segment() && segment()->measure() && !segment()->measure()->nextMeasure())
                        return QVariant::fromValue(BarLineType::END);
                  return QVariant::fromValue(BarLineType::NORMAL);

            case P_ID::BARLINE_SPAN:
                  if (staff())
                        return staff()->barLineSpan();
                  return 1;

            case P_ID::BARLINE_SPAN_FROM:
                  if (staff())
                        return staff()->barLineFrom();
                  return 0;

            case P_ID::BARLINE_SPAN_TO:
                  if (staff())
                        return staff()->barLineTo();
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
      return segment()->firstInNextSegments(score()->inputState().prevTrack() / VOICES);
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* BarLine::prevElement()
      {
      return segment()->lastInPrevSegments(score()->inputState().prevTrack() / VOICES);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString BarLine::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(BarLine::userTypeName(barLineType()));
      }

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString BarLine::accessibleExtraInfo() const
      {
      Segment* seg = segment();
      QString rez;

      for (const Element* e : *el()) {
            if (!score()->selectionFilter().canSelect(e))
                  continue;
            rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
            }

      for (const Element* e : seg->annotations()) {
            if (!score()->selectionFilter().canSelect(e))
                  continue;
            if (e->track() == track())
                  rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
            }
      Measure* m = seg->measure();

      if (m) {    // always true?
            //jumps
            for (const Element* e : m->el()) {
                  if (!score()->selectionFilter().canSelect(e)) continue;
                  if (e->type() == Element::Type::JUMP)
                        rez= QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                  if (e->type() == Element::Type::MARKER) {
                        const Marker* m = toMarker(e);
                        if (m->markerType() == Marker::Type::FINE)
                              rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                        }

                  }
            //markers
            Measure* nextM = m->nextMeasureMM();
            if (nextM) {
                  for (const Element* e : nextM->el()) {
                        if (!score()->selectionFilter().canSelect(e))
                              continue;
                        if (e->isMarker()) {
                              if (toMarker(e)->markerType() == Marker::Type::FINE)
                                    continue; //added above^
                              rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                              }
                        }
                  }
            }

      int tick = seg->tick();

      auto spanners = score()->spannerMap().findOverlapping(tick, tick);
      for (auto interval : spanners) {
            Spanner* s = interval.value;
            if (!score()->selectionFilter().canSelect(s))
                  continue;
            if (s->type() == Element::Type::VOLTA) {
                  if (s->tick() == tick)
                        rez = tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                  if (s->tick2() == tick)
                        rez = tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                  }
            }
      return rez;
      }

}

