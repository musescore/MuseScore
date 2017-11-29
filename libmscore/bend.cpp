//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measure.h"
#include "bend.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "xml.h"
#include "tie.h"
#include "system.h"
#include "segment.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   styledProperties
//---------------------------------------------------------

static constexpr std::array<StyledProperty,5> styledProperties {{
      { StyleIdx::bendFontFace,      P_ID::FONT_FACE },
      { StyleIdx::bendFontSize,      P_ID::FONT_SIZE },
      { StyleIdx::bendFontBold,      P_ID::FONT_BOLD },
      { StyleIdx::bendFontItalic,    P_ID::FONT_ITALIC },
      { StyleIdx::bendFontUnderline, P_ID::FONT_UNDERLINE }
      }};

//---------------------------------------------------------
//   label
//---------------------------------------------------------

static const char* label[] = {
      "", "1/4", "1/2", "3/4", "full",
      "1 1/4", "1 1/2", "1 3/4", "2",
      "2 1/4", "2 1/2", "2 3/4", "3"
      };

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Bend::font(qreal sp) const
      {
      QFont f(fontFace);
      f.setBold(fontBold);
      f.setItalic(fontItalic);
      f.setUnderline(fontUnderline);
      qreal m = fontSize;
      m *= sp / SPATIUM20;

      f.setPointSizeF(m);
      return f;
      }

//---------------------------------------------------------
//   makeSegments
//---------------------------------------------------------
void Bend::collectPoints()
      {
      if (!parent() || !parent()->isNote() || !staff())
            return;

      Note* parentNote = toNote(parent());
      std::vector<Note*> tie = parentNote->tiedNotes();
      std::vector<int> ticks(tie.size() + 1);

      for (size_t i = 0; i < tie.size(); ++i)
            ticks[i] = tie[i]->tick();
      ticks.back() = tie.back()->tick() + tie.back()->chord()->actualTicks();



      auto toBendTime = [&ticks](int tick)->int
            {
            int start = ticks.front();
            int total = ticks.back() - start;
            return (tick - start) * (60.0f / total) + .5f;
            };

      if (_points.empty()) //Bend just added with editor
            {
            size_t index = 0;
            if(parentNote != tie.front())
                  {
                  for(; parentNote != tie[index]; ++index)
                        _points.push_back(PitchValue(toBendTime(ticks[index]), 0, true));
                  parentNote->remove(this);
                  parentNote = tie.front();
                  setParent(parentNote);
                  parentNote->add(this);
                  }
            _points.push_back(PitchValue(toBendTime(ticks[index++]), 100, true));
            while(index < ticks.size())
                  _points.push_back(PitchValue(toBendTime(ticks[index++]), 0, true));
            }

      auto getBend = [](Note* note) -> Bend*
            {
            for (auto el : note->el())
                  if (el->isBend())
                        return toBend(el);
            return nullptr;
            };

      bool needRebuild = false;
      //GuitrarPro: prepare and delete all other tied bends
      for (auto note : tie) {
            if (note == parent())
                  continue;
            Bend* bend = getBend(note);
            if (bend) {
                  needRebuild = true;
                  auto points = bend->points();
                  for (int i = 1; i < points.size(); ++i)
                        _points.push_back(points[i]);

                  note->remove(bend);
                  delete bend;
                  }
            }

      if (parentNote != tie.front())
            {
            needRebuild = true;
            for(size_t index = 0; tie[index] != parentNote; ++index)
                  _points.push_front(PitchValue(0, 0, true));
            parentNote->remove(this);
            parentNote = tie.front();
            setParent(parentNote);
            parentNote->add(this);
            }

      //Need rebuild points:
      if (needRebuild || _points.size() < (int)tie.size() + 1 || _points.size() > (int)tie.size() + 2)
            {
            while(_points.size() < (int)tie.size() + 1)
                  _points.push_back(PitchValue(0, 0, true));
           if (_points.size() > (int)tie.size() + 2)
                 {
                 bool addZero = _points.back().pitch == 0;
                 while (_points.size() > (int)tie.size() + 2)
                        _points.takeLast();
                 if (addZero)
                       _points.back().pitch = 0;
                 }

            if (_points.size() == (int)tie.size() + 2)
                  {
                  int size = ticks.size();
                  int prev = ticks[size - 2];
                  int last = ticks[size - 1];
                  ticks[size - 1] = (prev + last) / 2;
                  ticks.push_back(last);
                  }
            for (int i = 0, ie = _points.size(); i < ie; ++i)
                  _points[i].time = toBendTime(ticks[i]);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------
void Bend::makeSegments()
      {
      if (!staff())
            return;
      auto tie = toNote(parent())->tiedNotes();
      int dif = _points.size() - (int)tie.size();

      bool tabStaff = staff()->isTabStaff(tick());

      tiedBend = tie.size() > 1;
      _segments.clear();

      if (dif == 2)
            tie.push_back(tie.back());

      _shape.clear();

      qreal _spatium = spatium() * 1.5;
      int nsegs = _points.size() - 1;
      if (tiedBend)
            {
            int n = std::min((int)tie.size(), (int)_points.size());
            for(int i = 0; i < n; ++i)
                  {

                  auto pt1 = _points[i];
                  auto pt2 = (_points.size() > i + 1) ? _points[i + 1] : _points.back();
                  if (pt1.pitch + pt2.pitch == 0)
                        continue;
                  int idx = (pt1.pitch + 12) / 25;
                  int nidx = (pt2.pitch + 12) / 25;
                  Tie* slur = nullptr;
                  if (tie[i]->tieFor())
                        {
                        slur= tie[i]->tieFor();
                        slur->getBendLines().clear();
                        }
                  else
                        slur = tie[i]->tieBack();

                  if (tabStaff)
                        {
                        auto nheight = tie[i]->height();
                        if (i == 0 && pt1.pitch != 0)
                              {
                              float x = tie[i]->headWidth() * .3f;
                              float y = -nheight * .6f ;
                              float x1 = x;
                              float y1 = -tie[i]->pos().y() - bendYOffset - bendHeight * idx;
                              slur->getBendLines().push_back(QLineF(x, y, x1, y1));
                              slur->getBendText() = label[idx];
                              }

                        float x = 0;
                        //if (slur->getBendLines().size())
                             // x = slur->getBendLines().back().x2();
                        float y = (idx == 0 ? tie.front()->height() * -.3 : - tie[i]->pos().y() - bendYOffset - bendHeight * idx);

                        float x2 = x + _spatium * 2.0f;
                        float y2 =(nidx == 0 ? tie.front()->height() * -.3 : - tie[i]->pos().y() - bendYOffset - bendHeight * nidx);
                        slur->getBendLines().push_back(QLineF(x, y, x2, y2));
                        if (y2 < y)
                              slur->getBendText() = label[nidx];
                        }
                  else
                        {
                        bool up = tie[i]->line() < 5;
                        if (i == 0 && pt1.pitch != 0)
                              {
                              float x = -_spatium * 2.0f;
                              float y = staff()->lineDistance(tick()) * spatium();
                              float x2 = -_spatium;
                              float y2 = y + (up ? - _spatium : _spatium);
                              slur->getBendLines().push_back(QLineF(x, y, x2, y2));
                              slur->getBendLines().push_back(QLineF(x2, y2, 0, 0));
                              }
                        if (i != 0)
                              {
                              bool add = true;
                              for (auto e : tie[i]->el())
                                    {
                                    if (e->isSymbol() && toSymbol(e)->sym() == SymId::noteheadParenthesisLeft)
                                          {
                                          add = false;
                                          break;
                                          }
                                    }
                              if (add)
                                    tie[i]->addParentheses();
                              }
                        if (i >= n - dif)
                              {
                              if (idx == nidx) continue;
                              float x = 0;
                              float y = 0;
                              float x2 = _spatium;
                              float endy = 0;
                              if (nidx == 0)
                                    endy = staff()->lineDistance(tick()) * spatium() - (up ? 4.0 : -4.0);
                              float y2 = endy + (up ? -_spatium : _spatium);
                              slur->getBendLines().push_back(QLineF(x, y, x2, y2));
                              slur->getBendLines().push_back(QLineF(x2, y2, _spatium * 2.0, endy));
                              }
                        else
                              {
                              float x = 0;
                              float y = 0;
                              float x2 = _spatium;
                              float y2 = (up ? -_spatium : _spatium);
                              slur->getBendLines().push_back(QLineF(x, y, x2, y2));
                              slur->getBendLines().push_back(QLineF(x2, y2, _spatium * 2, 0));
                              }

                        }
                  }
            }
      else {
            if (tabStaff) {
                  for (int i = 0; i < nsegs; ++i)
                        {
                        BendSegment bs;
                        int idx = (_points[i].pitch + 12) / 25;
                        int nidx = (_points[i + 1].pitch + 12) / 25;

                        if (i == 0 && idx != 0) {
                              bs.p1.rx() = tie.front()->headWidth() * .3f;
                              Note* n = tie.front();
                              bs.p1.ry() = - n->height() * .6;
                              bs.p2.rx() = bs.p1.x();
                              bs.p2.ry() = - tie.front()->pos().y() - bendYOffset - bendHeight * idx;
                              bs.label = QString(label[idx]);
                              bs.note = tie[i];
                              _segments.push_back(bs);
                              }
                        bs.p1.rx() = (_segments.empty() ? tie.front()->headWidth() : _segments.back().p2.x());
                        bs.p1.ry() = ((idx == 0 || tie.size() <= i) ? tie.front()->height() * -.3 : - tie[i]->pos().y() - bendYOffset - bendHeight * idx);
                        bs.p2.rx() = bs.p1.x() + _spatium * 2.0f;
                        bs.p2.ry() = ((nidx == 0 || tie.size() <= i) ? 0 : - tie[i]->pos().y() - bendYOffset - bendHeight * nidx);
                        if (bs.p1.y() > bs.p2.y())
                              bs.label = label[nidx];
                        else
                              bs.label = "";
                        bs.note = tie[i];
                        _segments.push_back(bs);
                        }
                  if (_segments.size())
                        {
                        qreal x = 0;
                        qreal y = 0;
                        for (auto& seg : _segments)
                              {
                              x = std::max(x, seg.p1.x());
                              x = std::max(x, seg.p2.x());
                              y = std::min(y, seg.p1.y());
                              y = std::min(y, seg.p1.y());
                              QPointF p1 = seg.p1, p2 = seg.p2;
                              if (seg.p1.y() == seg.p2.y())
                                    {
                                    p1.ry() -= 10;
                                    p2.ry() += 10;
                                    }
                              else if (seg.p1.x() == seg.p2.x())
                                    {
                                    p1.rx() -= 10;
                                    p2.rx() += 10;
                                    }
                              _shape.add(QRectF(p1, p2));
                              }
                        if (_segments.back().label.length())
                              {
                              QFont f = font(_spatium * MScore::pixelRatio);
                              QFontMetrics fm(f);
                              y -= fm.height();
                              x += fm.width(_segments.back().label);
                              }
                        else
                              {
                              x += spatium() * .4; // half width of arrow at bend end
                              }
                        setbbox(QRectF(0, 0, x, y));
                        }
                  }
            else  { //pitched staff
                  BendSegment bs;
                  int idx = (_points[0].pitch + 12) / 25;
                  int nidx = (_points[1].pitch + 12) / 25;
                  bool up = tie[0]->line() < 5;
                  qreal headWidth = tie[0]->headWidth();


                  if (idx != 0) {
                        bs.p1.rx() = -_spatium;
                        bs.p1.ry() = (up ? -11.0 : 11.0) + ( staff()->lineDistance(tick()) * spatium() * .5 * (idx / 4.0f));
                        bs.p2.rx() = -_spatium * .5;
                        bs.p2.ry() = (up ? -32.0 : 32.0);
                        bs.note = tie[0];
                        _segments.push_back(bs);
                        bs.p1 = bs.p2;
                        bs.p2.rx() = headWidth * .4;
                        bs.p2.ry() = (up ? -18.0f : 18.0f);
                        _segments.push_back(bs);
                        tie[0]->setLine(tie[0]->line() + idx / 4);
                        }
                  if (!tiedBend)
                        {
                        if (_points.size() == 2 && idx!= 0 && idx == nidx)
                              {
                              qreal x1 = 1000;
                              qreal x2 = -100;
                              qreal y1 = 1000;
                              qreal y2 = -100;
                              for (auto& s : _segments)
                                    {
                                    x1 = std::min(s.p1.x(), x1);
                                    x2 = std::max(s.p2.x(), x2);
                                    y1 = std::min(s.p1.y(), y1);
                                    y2 = std::max(s.p2.y(), y2);
                                    QPointF p1 = s.p1, p2 = s.p2;
                                    if (s.p1.y() == s.p2.y())
                                          {
                                          p1.ry() -= 10;
                                          p2.ry() += 10;
                                          }
                                    else if (s.p1.x() == s.p2.x())
                                          {
                                          p1.rx() -= 10;
                                          p2.rx() += 10;
                                          }
                                    _shape.add(QRectF(p1, p2));
                                    }
                              setbbox(QRectF(x1, y1, x2, y2));
                              return;
                              }
                        if (_points.size() == 2 || _points[1].pitch == _points.back().pitch || idx != 0)
                              nidx = (_points.back().pitch + 12) / 25;
                        else
                              nidx = (_points[1].pitch + 12) / 25;
                        bs.p1.rx() = headWidth * .6;
                        bs.p1.ry() = (up ? -18.0 : 18.0);
                        bs.p2.rx() = _spatium + bs.p1.x();
                        bs.p2.ry() = (up ? -32.0 : 32.0);
                        _segments.push_back(bs);
                        bs.p1 = bs.p2;
                        bs.p2.rx() = _spatium * 2.0 + headWidth * .6;
                        bs.p2.ry() = (-nidx / 8.0) * staff()->lineDistance(tick()) * spatium() + (up ? -11.0 : 11.0);
                        _segments.push_back(bs);
                        if (idx == 0 &&_points.size() > 2 && _points[1].pitch != _points.back().pitch)
                              {
                              nidx = (_points.back().pitch + 12) / 25;
                              bs.p1 = bs.p2;
                              bs.p1.rx() += headWidth * .1;
                              bs.p2.rx() = bs.p1.x() + _spatium;
                              bs.p2.ry() = (up ? -32.0 : 32.0);
                              _segments.push_back(bs);
                              bs.p1 = bs.p2;
                              bs.p2.rx() = bs.p1.x() + _spatium;
                              bs.p2.ry() = (-nidx / 8.0) * staff()->lineDistance(tick()) * spatium() + (up ? -11.0 : 11.0);
                              _segments.push_back(bs);
                              }
                        //setbbox(QRectF(_segments.front().p1.x() - 16.0, -24.0, _segments.back().p2.x() + 16.0, 24.0));
                        qreal x1 = 1000;
                        qreal x2 = -100;
                        qreal y1 = 1000;
                        qreal y2 = -100;
                        for (auto& s : _segments)
                              {
                              x1 = std::min(s.p1.x(), x1);
                              x2 = std::max(s.p2.x(), x2);
                              y1 = std::min(s.p1.y(), y1);
                              y2 = std::max(s.p2.y(), y2);
                              QPointF p1 = s.p1, p2 = s.p2;
                              if (s.p1.y() == s.p2.y())
                                    {
                                    p1.ry() -= 10;
                                    p2.ry() += 10;
                                    }
                              else if (s.p1.x() == s.p2.x())
                                    {
                                    p1.rx() -= 10;
                                    p2.rx() += 10;
                                    }
                              _shape.add(QRectF(p1, p2));
                              }
                        setbbox(QRectF(x1, y1, x2, y2));
                        }
                  }
            }
      }
//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bend::layout()
      {
      // during mtest, there may be no score. If so, exit.
      if (!score() || !staff())
            return;

      /*int tsize = toNote(parent())->tiedNotes().size();
      if (tsize > 1 && (_points.empty() || tsize < _points.size() + 1 || tsize > _points.size() + 2))*/
      _originPoints = _points;
      collectPoints();
      makeSegments();
      if (_originPoints.size())
            std::swap(_originPoints, _points);
      else
            _originPoints = _points;
      }
//---------------------------------------------------------
//   draw bend on panel: (private)
//---------------------------------------------------------

void Bend::panelDraw(QPainter* painter) const
      {
      QPen pen(curColor(), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);

      qreal _spatium = spatium();
      QFont f = font(_spatium * MScore::pixelRatio);
      painter->setFont(f);

      qreal aw = spatium() * .4;
      QPolygonF arrowUp;
      arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
      QFontMetrics fm(f);

      QPainterPath path;
      path.moveTo(-35, 20);
      path.cubicTo(15, 20, 20, -10, 20, -16);
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);

      painter->setBrush(curColor());
      painter->drawPolygon(arrowUp.translated(20, -16));

      painter->drawText(QRectF(20, -16, .0, .0),
                        Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString("bend"));
      }

QList<PitchValue> Bend::gridPoints() const
      {
      Note* n = toNote(parent());
      int gridSize = n->tiedNotes().size() + 2;
      QList<PitchValue> result;
      float timeStep = 60.0 / (gridSize - 1);
      float currentTime = 0;
      for(int i = 0; i < _originPoints.size() - 1; ++i)
            {
            result.push_back(_points.at(i));
            result.back().time = currentTime + .5;
            currentTime += timeStep;
            }
      result.push_back(_originPoints.back());
      result.back().time = 60.0;
      return result;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bend::draw(QPainter* painter) const
      {
      if (!staff())
            return panelDraw(painter);

      if (!parent()->isNote() || _segments.empty())
          return;

      bool up = toNote(parent())->line() < 5;
      qreal _spatium = spatium();

      QPen pen(curColor(), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      bool tabStaff = staff()->isTabStaff(tick());
      if (!tabStaff) // pitched staff
            {
            bool prebend = _segments.front().p1.x() < 0;
            int index = 0;
            auto seg = _segments[index++];
            if (prebend)
                  {
                  auto pt = seg.p1;
                  pt.rx() -= 10.0f;
                  pt.ry() += (up ? 11.0f : -12.0f);
                  drawSymbol(SymId::noteheadBlack, painter, pt, .7);
                  pt.rx() -= 10.0f;
                  drawSymbol(SymId::noteheadParenthesisLeft, painter, pt, .5);
                  pt.rx() += 34.0f;
                  drawSymbol(SymId::noteheadParenthesisRight, painter, pt, .5);

                  QPainterPath path;
                  path.moveTo(seg.p1);
                  path.lineTo(seg.p2);

                  seg = _segments[index++];

                  path.lineTo(seg.p2);
                  painter->drawPath(path);

                  if (index == _segments.size())
                        return;
                  seg = _segments[index++];
                  }
            if (tiedBend)
                  return;
            QPainterPath path;
            path.moveTo(seg.p1);
            path.lineTo(seg.p2);
            seg = _segments[index++];
            path.lineTo(seg.p2);
            painter->drawPath(path);

            auto pt = seg.p2;
            pt.rx() -= 10.0f;
            pt.ry() += (up ? 11.0f : -12.0f);
            drawSymbol(SymId::noteheadBlack, painter, pt, .7);

            if (index < _segments.size())
                  {
                  seg = _segments[index++];
                  QPainterPath path;
                  path.moveTo(seg.p1);
                  path.lineTo(seg.p2);
                  seg = _segments[index++];
                  path.lineTo(seg.p2);
                  painter->drawPath(path);

                  auto pt = seg.p2;
                  pt.rx() -= 10.0f;
                  pt.ry() += (up ? 11.0f : -12.0f);
                  drawSymbol(SymId::noteheadBlack, painter, pt, .7);
                  }
            }
      else //tab staff
            {
            qreal aw = _spatium * .5;
            QPolygonF arrowUp;
            arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
            QPolygonF arrowDown;
            arrowDown << QPointF(0, 0) << QPointF(aw*.5, -aw) << QPointF(-aw*.5, -aw);
            QFont f = font(_spatium * MScore::pixelRatio);
            painter->setFont(f);
            QFontMetrics fm(f);
            for (auto seg : _segments)
                  {
                  painter->setBrush(Qt::NoBrush);
                  if (seg.p1.x() == seg.p2.x())
                        {
                        painter->drawLine(seg.p1, seg.p2);
                        painter->setBrush(curColor());
                        painter->drawPolygon(arrowUp.translated(seg.p2));
                        qreal textWidth = fm.width(seg.label);
                        qreal textHeight = fm.height();
                        painter->drawText(QRectF(seg.p2.x() - textWidth / 2, seg.p2.y() - textHeight / 2, .0, .0), Qt::AlignVCenter|Qt::TextDontClip, seg.label);
                        }
                  else if (seg.p1.y() == seg.p2.y())
                        {
                        auto oldpen = painter->pen();
                        painter->setPen(QPen(curColor(), oldpen.width(), Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->drawLine(QLineF(seg.p1, seg.p2));
                        painter->setPen(oldpen);
                        }
                  else
                        {
                        qreal dx = seg.p2.x() - seg.p1.x();
                        qreal dy = seg.p1.y() - seg.p2.y();
                        QPainterPath path;
                        path.moveTo(seg.p1.x(), seg.p1.y());
                        path.cubicTo(seg.p1.x() + dx / 4, seg.p1.y(), seg.p2.x(), seg.p1.y() + dy / 5, seg.p2.x(), seg.p2.y());
                        painter->drawPath(path);
                        if (seg.p1.y() > seg.p2.y())
                              {
                              painter->setBrush(curColor());
                              painter->drawPolygon(arrowUp.translated(seg.p2));
                              qreal textWidth = fm.width(seg.label);
                              qreal textHeight = fm.height();
                              painter->drawText(QRectF(seg.p2.x() - textWidth / 2, seg.p2.y() - textHeight / 2, .0, .0), Qt::AlignVCenter|Qt::TextDontClip, seg.label);
                              }
                        else  {
                              painter->setBrush(curColor());
                              painter->drawPolygon(arrowDown.translated(seg.p2));
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Bend::write(XmlWriter& xml) const
      {
      xml.stag("Bend");
      for (const PitchValue& v : _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      for (auto k : styledProperties)
            writeProperty(xml, k.propertyIdx);
      writeProperty(xml, P_ID::PLAY);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Bend::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            bool found = false;
            for (auto k : styledProperties) {
                  if (readProperty(tag, e, k.propertyIdx)) {
                        setPropertyFlags(k.propertyIdx, PropertyFlags::UNSTYLED);
                        found = true;
                        break;
                        }
                  }
            if (found)
                  continue;
            if (tag == "point") {
                  PitchValue pv;
                  pv.time    = e.intAttribute("time");
                  pv.pitch   = e.intAttribute("pitch");
                  pv.vibrato = e.intAttribute("vibrato");
                  _points.append(pv);
                  e.readNext();
                  }
            else if (tag == "play") {
                  setPlayBend(e.readBool());
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Bend::getPropertyStyle(P_ID id) const
      {
      for (auto k : styledProperties) {
            if (k.propertyIdx == id)
                  return k.styleIdx;
            }
      return Element::getPropertyStyle(id);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Bend::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::FONT_FACE:
                  return fontFace;
            case P_ID::FONT_SIZE:
                  return fontSize;
            case P_ID::FONT_BOLD:
                  return fontBold;
            case P_ID::FONT_ITALIC:
                  return fontItalic;
            case P_ID::FONT_UNDERLINE:
                  return fontUnderline;
            case P_ID::PLAY:
                  return bool(playBend());
            default:
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bend::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::FONT_FACE:
                  fontFace = v.toString();
                  break;
            case P_ID::FONT_SIZE:
                  fontSize = v.toReal();
                  break;
            case P_ID::FONT_BOLD:
                  fontBold = v.toBool();
                  break;
            case P_ID::FONT_ITALIC:
                  fontItalic = v.toBool();
                  break;
            case P_ID::FONT_UNDERLINE:
                  fontUnderline = v.toBool();
                  break;
            case P_ID::PLAY:
                 setPlayBend(v.toBool());
                 break;
            default:
                  return Element::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Bend::propertyDefault(P_ID id) const
      {
      for (auto k : styledProperties) {
            if (k.propertyIdx == id)
                  return score()->styleV(k.styleIdx);
            }
      switch (id) {
            case P_ID::PLAY:
                  return true;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Bend::resetProperty(P_ID id)
      {
      setPropertyFlags(id, PropertyFlags::STYLED);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Bend::reset()
      {
      for (auto k : styledProperties)
            undoResetProperty(k.propertyIdx);
      Element::reset();
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags& Bend::propertyFlags(P_ID id)
      {
      int i = 0;
      for (auto k : styledProperties) {
            if (k.propertyIdx == id)
                  return propertyFlagsList[i];
            ++i;
            }
      return Element::propertyFlags(id);
      }

//---------------------------------------------------------
//   setPropertyFlags
//---------------------------------------------------------

void Bend::setPropertyFlags(P_ID id, PropertyFlags f)
      {
      int i = 0;
      for (auto k : styledProperties) {
            if (k.propertyIdx == id) {
                  propertyFlagsList[i] = f;
                  return;
                  }
            ++i;
            }
      Element::setPropertyFlags(id, f);
      }

Shape Bend::shape() const
      {
      return _shape;
      }
}
