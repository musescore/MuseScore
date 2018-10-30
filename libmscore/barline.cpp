//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
#include "part.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "articulation.h"
#include "stafftype.h"
#include "xml.h"
#include "marker.h"
#include "stafflines.h"
#include "spanner.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   undoChangeBarLineType
//---------------------------------------------------------

static void undoChangeBarLineType(BarLine* bl, BarLineType barType)
      {
      Measure* m = bl->measure();

      switch (barType) {
            case BarLineType::END:
            case BarLineType::NORMAL:
            case BarLineType::DOUBLE:
            case BarLineType::BROKEN:
            case BarLineType::DOTTED: {
                  Segment* segment        = bl->segment();
                  SegmentType segmentType = segment->segmentType();
                  if (segmentType == SegmentType::EndBarLine) {
                        m->undoChangeProperty(Pid::REPEAT_END, false);
                        for (Element* e : segment->elist()) {
                              if (e) {
                                    e->score()->undo(new ChangeProperty(e, Pid::BARLINE_TYPE, QVariant::fromValue(barType), PropertyFlags::NOSTYLE));
                                    e->score()->undo(new ChangeProperty(e, Pid::GENERATED, false, PropertyFlags::NOSTYLE));
                                    }
                              }
                        }
                  else if (segmentType == SegmentType::BeginBarLine) {
                        Segment* segment1 = m->undoGetSegmentR(SegmentType::BeginBarLine, 0);
                        for (Element* e : segment1->elist()) {
                              if (e) {
                                    e->score()->undo(new ChangeProperty(e, Pid::BARLINE_TYPE, QVariant::fromValue(barType), PropertyFlags::NOSTYLE));
                                    e->score()->undo(new ChangeProperty(e, Pid::GENERATED, false, PropertyFlags::NOSTYLE));
                                    }
                              else {
                                    auto score = bl->score();
                                    BarLine* newBl = new BarLine(score);
                                    newBl->setBarLineType(barType);
                                    newBl->setParent(segment1);
                                    newBl->setTrack(0);
                                    newBl->setSpanStaff(score->nstaves());
                                    newBl->score()->undo(new AddElement(newBl));
                                    }
                              }
                        }
                  else if (segmentType == SegmentType::StartRepeatBarLine)
                        m->undoChangeProperty(Pid::REPEAT_START, false);
                  }
                  break;
            case BarLineType::START_REPEAT:
                  m->undoChangeProperty(Pid::REPEAT_START, true);
                  break;
            case BarLineType::END_REPEAT:
                  m->undoChangeProperty(Pid::REPEAT_END, true);
                  break;
            }
      }

//---------------------------------------------------------
//   BarLineEditData
//---------------------------------------------------------

class BarLineEditData : public ElementEditData {
   public:
      qreal yoff1;
      qreal yoff2;
      };

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
      setHeight(4 * spatium()); // for use in palettes
      }

BarLine::BarLine(const BarLine& bl)
   : Element(bl)
      {
      _spanStaff   = bl._spanStaff;
      _spanFrom    = bl._spanFrom;
      _spanTo      = bl._spanTo;
      _barLineType = bl._barLineType;
      y1           = bl.y1;
      y2           = bl.y2;

      for (Element* e : bl._el)
            _el.push_back(e->clone());
      }

BarLine::~BarLine()
      {
      qDeleteAll(_el);
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
            int startIdx        = staffIdx();
            int endIdx          = startIdx + (spanStaff() ? 1 : 0);
            int staffIdx1       = startIdx;
            Staff* staff1       = score()->staff(staffIdx1);
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

void BarLine::getY() const
      {
      qreal _spatium = spatium();
      if (!parent()) {
            // for use in palette
            y1 = _spanFrom   * _spatium * .5;
            y2 = (8-_spanTo) * _spatium * .5;
            return;
            }
      int staffIdx1   = staffIdx();
      Staff* staff1   = score()->staff(staffIdx1);
      int staffIdx2   = staffIdx1;
      int nstaves     = score()->nstaves();
      bool spanStaves = false;

      Measure* measure = segment()->measure();
      if (_spanStaff) {
            for (int i2 = staffIdx1 + 1; i2 < nstaves; ++i2)  {
                  Staff* s = score()->staff(i2);
                  if (!s->invisible() && s->part()->show() && measure->visible(i2)) {
                        spanStaves = true;
                        staffIdx2  = i2;
                        break;
                        }
                  BarLine* nbl = toBarLine(segment()->element(i2 * VOICES));
                  if (!nbl || !nbl->spanStaff())
                        break;
                  }
            }

      System* system   = measure->system();
      if (!system)
            return;

      // test start and end staff visibility


      // base y on top visible staff in barline span
      // after skipping ones with hideSystemBarLine set
      // and accounting for staves that are shown but have invisible measures

      int tick       = segment()->measure()->tick();
      StaffType* st1 = staff1->staffType(tick);

      int from    = _spanFrom;
      int to      = _spanTo;
      int oneLine = st1->lines() == 1;
      if (oneLine && _spanFrom == 0) {
            from = BARLINE_SPAN_1LINESTAFF_FROM;
            if (!_spanStaff)
                  to = BARLINE_SPAN_1LINESTAFF_TO;
            }
      SysStaff* sysStaff1  = system->staff(staffIdx1);
      qreal yp = sysStaff1->y();
      qreal spatium1 = st1->spatium(score());
      qreal d  = st1->lineDistance().val() * spatium1;
      qreal yy = measure->staffLines(staffIdx1)->y1() - yp;
      qreal lw = score()->styleS(Sid::staffLineWidth).val() * spatium1 * .5;
      y1       = yy + from * d * .5 - lw;
      if (spanStaves)
            y2 = measure->staffLines(staffIdx2)->y1() - yp - to * d * .5;
      else
            y2 = yy + (st1->lines() * 2 - 2 + to) * d * .5 + lw;
      }

//---------------------------------------------------------
//   drawDots
//---------------------------------------------------------

void BarLine::drawDots(QPainter* painter, qreal x) const
      {
      qreal _spatium = spatium();

      qreal y1l;
      qreal y2l;
      if (parent() == 0) {    // for use in palette
            y1l = 2.0 * _spatium;
            y2l = 3.0 * _spatium;
            }
      else {
            Staff* staff  = score()->staff(staffIdx());
            StaffType* st = staff->staffType(tick());
            y1l           = st->doty1() * _spatium + 0.5 * score()->spatium() * mag();
            y2l           = st->doty2() * _spatium + 0.5 * score()->spatium() * mag();
            }
      drawSymbol(SymId::repeatDot, painter, QPointF(x, y1l));
      drawSymbol(SymId::repeatDot, painter, QPointF(x, y2l));
      }

//---------------------------------------------------------
//   drawTips
//---------------------------------------------------------

void BarLine::drawTips(QPainter* painter, bool reversed, qreal x) const
      {
      if (reversed) {
            if (isTop())
                  drawSymbol(SymId::reversedBracketTop, painter, QPointF(x - symWidth(SymId::reversedBracketTop), y1));
            if (isBottom())
                  drawSymbol(SymId::reversedBracketBottom, painter, QPointF(x - symWidth(SymId::reversedBracketBottom), y2));
            }
      else {
            if (isTop())
                  drawSymbol(SymId::bracketTop, painter, QPointF(x, y1));
            if (isBottom())
                  drawSymbol(SymId::bracketBottom, painter, QPointF(x, y2));
            }
      }

//---------------------------------------------------------
//   isTop
//---------------------------------------------------------

bool BarLine::isTop() const
      {
      int i = staffIdx();
      return i == 0 || !toBarLine(segment()->element((i-1) * VOICES))->spanStaff();
      }

//---------------------------------------------------------
//   isBottom
//---------------------------------------------------------

bool BarLine::isBottom() const
      {
      return !_spanStaff;      // TODO
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(QPainter* painter) const
      {
      switch (barLineType()) {
            case BarLineType::NORMAL: {
                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  }
                  break;

            case BarLineType::BROKEN: {
                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::DashLine, Qt::FlatCap));
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  }
                  break;

            case BarLineType::DOTTED: {
                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::DotLine, Qt::FlatCap));
                  painter->drawLine(QLineF(lw * .5, y1, lw * .5, y2));
                  }
                  break;

            case BarLineType::END: {
                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
                  qreal x  = lw * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));

                  qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
                  painter->setPen(QPen(curColor(), lw2, Qt::SolidLine, Qt::FlatCap));
                  x  += score()->styleP(Sid::endBarDistance) * mag();
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case BarLineType::DOUBLE: {
                  qreal lw2 = score()->styleP(Sid::doubleBarWidth)    * mag();
                  painter->setPen(QPen(curColor(), lw2, Qt::SolidLine, Qt::FlatCap));
                  qreal x = lw2 * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));
                  x += score()->styleP(Sid::doubleBarDistance) * mag();
                  painter->drawLine(QLineF(x, y1, x, y2));
                  }
                  break;

            case BarLineType::START_REPEAT: {
                  qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
                  painter->setPen(QPen(curColor(), lw2, Qt::SolidLine, Qt::FlatCap));
                  qreal x = lw2 * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));

                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
                  x  += score()->styleP(Sid::endBarDistance) * mag();
                  painter->drawLine(QLineF(x, y1, x, y2));

                  x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
                  x -= symBbox(SymId::repeatDot).width() * .5;
                  drawDots(painter, x);

                  if (score()->styleB(Sid::repeatBarTips))
                        drawTips(painter, false, 0.0);
                  }
                  break;

            case BarLineType::END_REPEAT: {
                  qreal lw = score()->styleP(Sid::barWidth) * mag();
                  painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));

                  qreal x = 0.0; // symBbox(SymId::repeatDot).width() * .5;
                  drawDots(painter, x);

                  x += score()->styleP(Sid::repeatBarlineDotSeparation) * mag();
                  x += symBbox(SymId::repeatDot).width() * .5;
                  painter->drawLine(QLineF(x, y1, x, y2));

                  x  += score()->styleP(Sid::endBarDistance) * mag();

                  qreal lw2 = score()->styleP(Sid::endBarWidth) * mag();
                  painter->setPen(QPen(curColor(), lw2, Qt::SolidLine, Qt::FlatCap));
                  painter->drawLine(QLineF(x, y1, x, y2));

                  if (score()->styleB(Sid::repeatBarTips))
                        drawTips(painter, true, x + lw2 * .5);
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
//   drawEditMode
//---------------------------------------------------------

void BarLine::drawEditMode(QPainter* p, EditData& ed)
      {
      Element::drawEditMode(p, ed);
      BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));
      y1 += bed->yoff1;
      y2 += bed->yoff2;
      QPointF pos(pagePos());
      p->translate(pos);
      BarLine::draw(p);
      p->translate(-pos);
      y1 -= bed->yoff1;
      y2 -= bed->yoff2;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(XmlWriter& xml) const
      {
      xml.stag(this);

      writeProperty(xml, Pid::BARLINE_TYPE);
      writeProperty(xml, Pid::BARLINE_SPAN);
      writeProperty(xml, Pid::BARLINE_SPAN_FROM);
      writeProperty(xml, Pid::BARLINE_SPAN_TO);

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
      resetProperty(Pid::BARLINE_SPAN);
      resetProperty(Pid::BARLINE_SPAN_FROM);
      resetProperty(Pid::BARLINE_SPAN_TO);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setBarLineType(e.readElementText());
            else if (tag == "span")
                  _spanStaff  = e.readBool();
            else if (tag == "spanFromOffset")
                  _spanFrom = e.readInt();
            else if (tag == "spanToOffset")
                  _spanTo = e.readInt();
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

bool BarLine::acceptDrop(EditData& data) const
      {
      ElementType type = data.element->type();
      if (type == ElementType::BAR_LINE) {
            return true;
            }
      else {
            return (type == ElementType::ARTICULATION
               && segment()
               && segment()->isEndBarLineType());
            }
      // Prevent unreachable code warning
      // return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(EditData& data)
      {
      Element* e = data.element;

      if (e->isBarLine()) {
            BarLine* bl    = toBarLine(e);
            BarLineType st = bl->barLineType();

            // if no change in subtype or no change in span, do nothing
            if (st == barLineType() && !bl->spanFrom() && !bl->spanTo()) {
                  delete e;
                  return 0;
                  }

            // check if the new property can apply to this single bar line
            BarLineType bt = BarLineType::START_REPEAT | BarLineType::END_REPEAT;
            bool oldRepeat = barLineType()     & bt;
            bool newRepeat = bl->barLineType() & bt;

            // if ctrl was used and repeats are not involved,
            // or if drop refers to span rather than subtype =>
            // single bar line drop

            if ((data.control() && !oldRepeat && !newRepeat) || (bl->spanFrom() || bl->spanTo()) ) {
                  // if drop refers to span, update this bar line span
                  if (bl->spanFrom() || bl->spanTo()) {
                        // if dropped spanFrom or spanTo are below the middle of standard staff (5 lines)
                        // adjust to the number of staff lines
                        // TODO:barlines
                        int spanFrom   = bl->spanFrom();
                        int spanTo     = bl->spanTo();
                        undoChangeProperty(Pid::BARLINE_SPAN, false);
                        undoChangeProperty(Pid::BARLINE_SPAN_FROM, spanFrom);
                        undoChangeProperty(Pid::BARLINE_SPAN_FROM, spanTo);
                        }
                  // if drop refers to subtype, update this bar line subtype
                  else
                        undoChangeBarLineType(this, st);
                  }
            else
                  undoChangeBarLineType(this, st);
            delete e;
            }
      else if (e->isArticulation()) {
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

void BarLine::updateGrips(EditData& ed) const
      {
      const BarLineEditData* bed = static_cast<const BarLineEditData*>(ed.getData(this));

      qreal lw = score()->styleP(Sid::barWidth) * staff()->mag(tick());
      getY();
      ed.grip[0].translate(QPointF(lw * .5, y1 + bed->yoff1) + pagePos());
      ed.grip[1].translate(QPointF(lw * .5, y2 + bed->yoff2) + pagePos());
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void BarLine::startEdit(EditData& ed)
      {
      ed.grips   = 2;
      ed.curGrip = Grip::END;

      BarLineEditData* bed = new BarLineEditData();
      bed->e     = this;
      bed->yoff1 = 0;
      bed->yoff2 = 0;
      ed.addData(bed);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void BarLine::endEdit(EditData&)
      {
#if 0 // TODO
      if (ctrlDrag) {                           // if single bar line edit
            char newSpanStaff = _spanStaff;          // copy edited span values
            char newSpanFrom  = _spanFrom;
            char newSpanTo    = _spanTo;
            _spanStaff        = _origSpanStaff;      // restore original span values
            _spanFrom         = _origSpanFrom;
            _spanTo           = _origSpanTo;
            // for mid-measure barline in root score, update parts
            if (midMeasure && score()->isMaster() && score()->excerpts().size() > 0) {
                  int currIdx = staffIdx();
                  Measure* m = segment()->measure();
                  // change linked barlines as necessary
                  int lastIdx = currIdx + qMax(_span, newSpanStaff);
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
                                    int spannedStaves = qMax(currIdx + newSpanStaff - idx, 0);
                                    int lNewSpan = qMin(spannedStaves, lscore->nstaves());
                                    Measure* lm = lscore->tick2measure(m->tick());
                                    Segment* lseg = lm->undoGetSegmentR(SegmentType::BarLine, rtick());
                                    BarLine* lbl = toBarLine(lseg->element(0));
                                    if (lbl) {
                                          // already a barline here
                                          if (lNewSpan > 0) {
                                                // keep barline, but update span if necessary
                                                if (lbl->span() != lNewSpan)
                                                      lbl->undoChangeProperty(Pid::BARLINE_SPAN, lNewSpan);
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
            undoChangeProperty(Pid::BARLINE_SPAN,      newSpanStaff);
            undoChangeProperty(Pid::BARLINE_SPAN_FROM, newSpanFrom);
            undoChangeProperty(Pid::BARLINE_SPAN_FROM, newSpanTo);
            return;
            }

      // if same as staff settings, do nothing
      if (staff()->barLineSpan() == _spanStaff && staff()->barLineFrom() == _spanFrom && staff()->barLineTo() == _spanTo)
            return;

//      int idx1 = staffIdx();
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
                              staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      0);
                              staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, 0);
                              staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   (staff->lines(tick())-1)*2);
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
                        int lines = staff->lines(tick());
                        int spanFrom = 0;
                        int spanTo   = 0;
                        staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      1);
                        staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, spanFrom);
                        staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   spanTo);
                        }
                  }
            }
      // update span for the staff the edited bar line belongs to
      staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      _spanStaff);
      staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, _spanFrom);
      staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   _spanTo);
#endif
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void BarLine::editDrag(EditData& ed)
      {
      BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));

      qreal lineDist = staff()->lineDistance(tick()) * spatium();
      getY();
      if (ed.curGrip == Grip::START) {
            // min offset for top grip is line -1 (-2 for 1-line staves)
            // max offset is 1 line above bottom grip or 1 below last staff line, whichever comes first
            int lines = staff()->lines(tick());
            qreal min = (-y1 - lines == 1) ? lineDist * 2 : lineDist;
            qreal max = y2 - y1 - lineDist;                                   // 1 line above bottom grip
            qreal lastmax = (lines - _spanFrom/2) * lineDist;      // 1 line below last staff line
            if (lastmax < max)
                  max = lastmax;
            // update yoff1 and bring it within limits
            bed->yoff1 += ed.delta.y();
            if (bed->yoff1 < min)
                  bed->yoff1 = min;
            if (bed->yoff1 > max)
                  bed->yoff1 = max;
            }
      else {
            // min for bottom grip is 1 line below top grip
            // no max
            qreal min = y1 - y2 + lineDist;
            // update yoff2 and bring it within limit
            bed->yoff2 += ed.delta.y();
            if (bed->yoff2 < min)
                  bed->yoff2 = min;
            }
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff / staff line
//---------------------------------------------------------

void BarLine::endEditDrag(EditData& ed)
      {
      getY();
      BarLineEditData* bed = static_cast<BarLineEditData*>(ed.getData(this));
      y1 += bed->yoff1;
      y2 += bed->yoff2;

      qreal ay0      = pagePos().y();
      qreal ay2      = ay0 + y2;                     // absolute (page-relative) bar line bottom coord
      int staffIdx1  = staffIdx();
      System* syst   = segment()->measure()->system();
      qreal systTopY = syst->pagePos().y();

      // determine new span value
      int staffIdx2;
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
                  qreal staff1Hght    = (staff1->lines(tick())-1) * staff1->lineDistance(tick()) * spatium();
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

      // determine new spanFrom and spanTo values
      int newSpanFrom, newSpanTo;
//      Staff * staff2    = score()->staff(staffIdx2);
//      int staff1lines   = staff()->lines(tick());
//      int staff2lines   = staff2->lines(tick());

#if 0       // TODO
      if (shiftDrag) {                    // if precision dragging
            newSpanFrom = _spanFrom;
            if (yoff1 != 0.0) {
                  // round bar line top coord to nearest line of 1st staff (in half line dist units)
                  newSpanFrom = ((int)floor(y1 / (staff()->lineDistance(tick()) * spatium()) + 0.5 )) * 2;
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
                  newSpanTo = ((int)floor( (ay2 - staff2TopY) / (staff2->lineDistance(tick()) * spatium()) + 0.5 )) * 2;
                  // min = 1 line dist above 1st staff line | max = 1 line dist below last staff line
                  int maxTo = staff2lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : staff2lines * 2;
                  if (newSpanTo <  MIN_BARLINE_SPAN_FROMTO)
                        newSpanTo = MIN_BARLINE_SPAN_FROMTO;
                  if (newSpanTo > maxTo)
                        newSpanTo = maxTo;
                  }
            }
#endif
//      else {                              // if coarse dragging
         {                              // if coarse dragging
            newSpanFrom = 0;
            newSpanTo   = 0;
            }

      bool localDrag = ed.control() || segment()->isBarLine();
      if (localDrag) {
            Segment* s = segment();
            for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx) {
                  BarLine* b = toBarLine(s->element(staffIdx * VOICES));
                  if (!b) {
                        b = toBarLine(linkedClone());
                        b->setSpanStaff(true);
                        b->setTrack(staffIdx * VOICES);
                        b->setScore(score());
                        b->setParent(s);
                        score()->undoAddElement(b);
                        }
                  b->undoChangeProperty(Pid::BARLINE_SPAN, true);
                  }
            BarLine* b = toBarLine(s->element(staffIdx2 * VOICES));
            if (b)
                  b->undoChangeProperty(Pid::BARLINE_SPAN, false);
            }
      else {
            for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++staffIdx)
                  score()->staff(staffIdx)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, true);
            score()->staff(staffIdx2)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
            staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, newSpanFrom);
            staff()->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   newSpanTo);
            }
      }

//---------------------------------------------------------
//   layoutWidth
//---------------------------------------------------------

qreal BarLine::layoutWidth(Score* score, BarLineType type)
      {
      qreal dotwidth = score->scoreFont()->width(SymId::repeatDot, 1.0);

      qreal w {0.0};
      switch (type) {
            case BarLineType::DOUBLE:
                  w = score->styleP(Sid::doubleBarWidth) + score->styleP(Sid::doubleBarDistance);
                  break;
            case BarLineType::START_REPEAT:
            case BarLineType::END_REPEAT:
                  w = score->styleP(Sid::endBarWidth) * .5
                     + score->styleP(Sid::endBarDistance)
                     + score->styleP(Sid::repeatBarlineDotSeparation)
                     + dotwidth * .5;
                  break;
            case BarLineType::END:
                  w = (score->styleP(Sid::endBarWidth) + score->styleP(Sid::barWidth)) * .5
                     + score->styleP(Sid::endBarDistance);
                  break;
            case BarLineType::BROKEN:
            case BarLineType::NORMAL:
            case BarLineType::DOTTED:
                  w = score->styleP(Sid::barWidth);
                  break;
            }
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BarLine::layout()
      {
      setPos(QPointF());
      setMag(score()->styleB(Sid::scaleBarlines) && staff() ? staff()->mag(tick()) : 1.0);
      qreal _spatium = spatium();
      y1 = _spatium * .5 * _spanFrom;
      y2 = _spatium * .5 * (8.0 + _spanTo);

      // bar lines not hidden
      qreal w = layoutWidth(score(), barLineType()) * mag();
      QRectF r(0.0, y1, w, y2 - y1);

      if (score()->styleB(Sid::repeatBarTips)) {
            switch (barLineType()) {
                  case BarLineType::START_REPEAT:
                        r |= symBbox(SymId::bracketTop).translated(0, y1);
                        // r |= symBbox(SymId::bracketBottom).translated(0, y2);
                        break;
                  case BarLineType::END_REPEAT: {
                        qreal w1 = symBbox(SymId::reversedBracketTop).width();
                        r |= symBbox(SymId::reversedBracketTop).translated(-w1, y1);
                        // r |= symBbox(SymId::reversedBracketBottom).translated(0, y2);
                        }
                        break;
                  default:
                        break;
                  }
            }
      setbbox(r);

      for (Element* e : _el) {
            e->layout();
            if (e->isArticulation()) {
                  Articulation* a  = toArticulation(e);
                  Direction dir    = a->direction();
                  qreal distance   = 0.5 * spatium();
                  qreal x          = width() * .5;
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
//   layout2
//    called after system layout; set vertical dimensions
//---------------------------------------------------------

void BarLine::layout2()
      {
      getY();
      bbox().setTop(y1);
      bbox().setBottom(y2);

      if (score()->styleB(Sid::repeatBarTips)) {
            switch (barLineType()) {
                  case BarLineType::START_REPEAT:
                        bbox() |= symBbox(SymId::bracketTop).translated(0, y1);
                        bbox() |= symBbox(SymId::bracketBottom).translated(0, y2);
                        break;
                  case BarLineType::END_REPEAT:
                        {
                        qreal w1 = symBbox(SymId::reversedBracketTop).width();
                        bbox() |= symBbox(SymId::reversedBracketTop).translated(-w1, y1);
                        bbox() |= symBbox(SymId::reversedBracketBottom).translated(-w1, y2);
                        break;
                        }
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape BarLine::shape() const
      {
      Shape shape;
#ifndef NDEBUG
      shape.add(bbox(), name());
#else
      shape.add(bbox());
#endif
      return shape;
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
            case ElementType::ARTICULATION:
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
            case ElementType::ARTICULATION:
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

QVariant BarLine::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::BARLINE_TYPE:
                  return QVariant::fromValue(_barLineType);
            case Pid::BARLINE_SPAN:
                  return spanStaff();
            case Pid::BARLINE_SPAN_FROM:
                  return int(spanFrom());
            case Pid::BARLINE_SPAN_TO:
                  return int(spanTo());
            default:
                  break;
            }
      return Element::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BarLine::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::BARLINE_TYPE:
                  setBarLineType(v.value<BarLineType>());
                  break;
            case Pid::BARLINE_SPAN:
                  setSpanStaff(v.toBool());
                  break;
            case Pid::BARLINE_SPAN_FROM:
                  setSpanFrom(v.toInt());
                  break;
            case Pid::BARLINE_SPAN_TO:
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
//   undoChangeProperty
//---------------------------------------------------------

void BarLine::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (id == Pid::BARLINE_TYPE && segment()) {
            const BarLine* bl = this;
            BarLineType blType = v.value<BarLineType>();
            if (blType == BarLineType::START_REPEAT) { // change next measures endBarLine
                  if (bl->measure()->nextMeasure())
                        bl = bl->measure()->nextMeasure()->endBarLine();
                  else
                        bl = 0;
                  }
            if (bl)
                  undoChangeBarLineType(const_cast<BarLine*>(bl), v.value<BarLineType>());
            }
      else
            ScoreElement::undoChangeProperty(id, v, ps);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant BarLine::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::BARLINE_TYPE:
// dynamic default values are a bad idea: writing to xml the value maybe ommited resulting in
//    wrong values on read (as the default may be different on read)
//                  if (segment() && segment()->measure() && !segment()->measure()->nextMeasure())
//                        return QVariant::fromValue(BarLineType::END);
                  return QVariant::fromValue(BarLineType::NORMAL);

            case Pid::BARLINE_SPAN:
                  return staff() ? staff()->barLineSpan() : false;

            case Pid::BARLINE_SPAN_FROM:
                  return staff() ? staff()->barLineFrom() : 0;

            case Pid::BARLINE_SPAN_TO:
                  return staff() ? staff()->barLineTo() : 0;

            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* BarLine::nextSegmentElement()
      {
      return segment()->firstInNextSegments(score()->inputState().prevTrack() / VOICES);
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* BarLine::prevSegmentElement()
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
                  if (e->type() == ElementType::JUMP)
                        rez= QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
                  if (e->type() == ElementType::MARKER) {
                        const Marker* m1 = toMarker(e);
                        if (m1->markerType() == Marker::Type::FINE)
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
            if (s->type() == ElementType::VOLTA) {
                  if (s->tick() == tick)
                        rez = QObject::tr("%1 Start of %2").arg(rez).arg(s->screenReaderInfo());
                  if (s->tick2() == tick)
                        rez = QObject::tr("%1 End of %2").arg(rez).arg(s->screenReaderInfo());
                  }
            }
      return rez;
      }

}

