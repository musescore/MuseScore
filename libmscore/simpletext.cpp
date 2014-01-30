//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "simpletext.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "box.h"
#include "textframe.h"
#include "sym.h"

namespace Ms {

TCursor SimpleText::_cursor;

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TFragment TFragment::split(int column)
      {
      int idx = 0;
      int col = 0;
      TFragment f;
      f.cf = cf;

      for (const QChar& c : text) {
            if (col == column) {
                  if (idx) {
                        if (idx < text.size()) {
                              f.text = text.mid(idx);
                              text   = text.left(idx);
                              if (cf == CharFormat::SYMBOL) {
                                    QList<SymId> l1;
                                    for (int k = 0; k < ids.size(); ++k) {
                                          if (k < idx)
                                                l1.append(ids[k]);
                                          else
                                                f.ids.append(ids[k]);
                                          }
                                    ids = l1;
                                    }
                              }
                        }
                  return f;
                  }
            ++idx;
            if (c.isHighSurrogate())
                  continue;
            ++col;
            }
      return f;
      }

//---------------------------------------------------------
//   operator ==
//---------------------------------------------------------

bool TFragment::operator ==(const TFragment& f) const
      {
      return cf == f.cf && (cf == CharFormat::STYLED ? text == f.text : ids == f.ids);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TFragment::draw(QPainter* p, const SimpleText* t) const
      {
      if (cf == CharFormat::STYLED)
            p->drawText(pos, text);
      else if (cf == CharFormat::SYMBOL) {
            p->save();
            bool fallback = false;
            for (SymId id : ids) {
                  if (!t->symIsValid(id)) {
                        fallback = true;
                        break;
                        }
                  }
            ScoreFont* f = fallback ? ScoreFont::fallbackFont() : t->score()->scoreFont();
            QString s;
            for (SymId id : ids)
                  s.append(f->toString(id));

            qreal px  = qreal(p->font().pixelSize());
            qreal mag = (px * PPI) / (20.0 * MScore::DPI);
            f->draw(s, p, mag, pos);
            p->restore();
            }
      }

//---------------------------------------------------------
//   TLine
//---------------------------------------------------------

TLine::TLine(const QString& s)
      {
      setText(s);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString TLine::text() const
      {
      QString s;
      for (const TFragment& f : _text) {
            if (f.cf == CharFormat::STYLED)
                  s += f.text;
            else {
                  for (SymId id : f.ids) {
                        s += "&";
                        s += Sym::id2name(id);
                        s += ";";
                        }
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TLine::setText(const QString& s)
      {
      QRegularExpression r("&([a-zA-Z]+\\w?);");
      int offset = 0;
      for (;;) {
            QRegularExpressionMatch m = r.match(s, offset);
            if (m.hasMatch()) {
                  int si = m.capturedStart(0);
                  int ei = m.capturedEnd(0);
                  if (si > offset) {
                        TFragment f;
                        f.text = s.mid(offset, si - offset);
                        f.cf = CharFormat::STYLED;
                        _text.append(f);
                        }
                  SymId id = Sym::name2id(m.captured(1));
                  if (!_text.isEmpty() && _text.back().cf == CharFormat::SYMBOL)
                        _text.back().ids.append(id);
                  else {
                        TFragment f;
                        f.cf = CharFormat::SYMBOL;
                        f.ids.append(id);
                        _text.append(f);
                        }
                  offset = ei;
                  }
            else {
                  TFragment f;
                  f.text = s.mid(offset);
                  if (!f.text.isEmpty()) {
                        f.cf = CharFormat::STYLED;
                        _text.append(f);
                        }
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TLine::draw(QPainter* p, const SimpleText* t) const
      {
      for (const TFragment& f : _text)
            f.draw(p, t);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TLine::layout(double y, SimpleText* t)
      {
      _bbox = QRectF();
      qreal x = 0.0;
      QFontMetricsF fm(t->textStyle().fontPx(t->spatium()));

      for (TFragment& f : _text) {
            f.pos.setX(x);
            f.pos.setY(y);
            qreal w;
            QRectF r;
            switch(f.cf) {
                  case CharFormat::STYLED:
                        r = fm.tightBoundingRect(f.text).translated(f.pos);
                        w = fm.width(f.text);
                        break;
                  case CharFormat::SYMBOL:
                        {
                        ScoreFont* sf = t->score()->scoreFont();
                        f.text.clear();
                        for (SymId id : f.ids)
                              f.text.append(sf->toString(id));
                        r = t->symBbox(f.text).translated(f.pos);
                        w = t->symWidth(f.text);
                        }
                        break;
                  }
            _bbox |= r;
            x += w;
            }
      }

//---------------------------------------------------------
//   xpos
//---------------------------------------------------------

qreal TLine::xpos(int column, const SimpleText* t) const
      {
      QFontMetricsF fm(t->textStyle().fontPx(t->spatium()));

      int col = 0;
      for (const TFragment& f : _text) {
            if (column == col)
                  return f.pos.x();
            int idx = 0;
            for (const QChar& c : f.text) {
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  if (column == col) {
                        if (f.cf == CharFormat::STYLED)
                              return f.pos.x() + fm.width(f.text.left(idx));
                        else if (f.cf == CharFormat::SYMBOL)
                              return f.pos.x() + t->symWidth(f.text.left(idx));
                        }
                  }
            }
      return 0.0;
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF TLine::boundingRect(int col1, int col2, const SimpleText* t) const
      {
      qreal x1 = xpos(col1, t);
      qreal x2 = xpos(col2, t);
      return QRectF(x1, _bbox.y(), x2-x1, _bbox.height());
      }

//---------------------------------------------------------
//   moveX
//---------------------------------------------------------

void TLine::moveX(qreal dx)
      {
      for (TFragment& f : _text)
            f.pos.rx() += dx;
      _bbox = _bbox.translated(dx, 0.0);
      }

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int TLine::columns() const
      {
      int col = 0;
      for (const TFragment& f : _text) {
            for (const QChar& c : f.text) {
                  if (!c.isHighSurrogate())
                        ++col;
                  }
            }
      return col;
      }

//---------------------------------------------------------
//   column
//    Return nearest column for position x. X is in
//    SimpleText coordinate system
//---------------------------------------------------------

int TLine::column(qreal x, SimpleText* t) const
      {
      int col = 0;
      QFontMetricsF fm(t->textStyle().fontPx(t->spatium()));
      for (const TFragment& f : _text) {
            int idx = 0;
            if (x <= f.pos.x())
                  return col;
            qreal px = 0.0;
            for (const QChar& c : f.text) {
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  qreal xo;
                  if (f.cf == CharFormat::STYLED)
                        xo = fm.width(f.text.left(idx));
                  else if (f.cf == CharFormat::SYMBOL)
                        xo = t->symWidth(f.text.left(idx));
                  if (x <= f.pos.x() + px + (xo-px)*.5)
                        return col;
                  ++col;
                  px = xo;
                  }
            }
      return col;
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void TLine::insert(int column, const QString& s)
      {
      int col = 0;
      for (auto n = _text.begin(); n != _text.end(); ++n) {
            TFragment& f = *n;
            int rcol = 0;
            for (const QChar& c : f.text) {
                  if (col == column) {
                        if (f.cf == CharFormat::STYLED)
                              f.text.insert(rcol, s);
                        else if (f.cf == CharFormat::SYMBOL) {
                              TFragment f2 = f.split(rcol);
                              n = _text.insert(n+1, TFragment(s));
                              _text.insert(n+1, f2);
                              }
                        return;
                        }
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            }
      if (!_text.isEmpty() && _text.back().cf == CharFormat::STYLED)
            _text.back().text.append(s);
      else
            _text.append(TFragment(s));
      }

void TLine::insert(int column, SymId id)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (i->cf == CharFormat::SYMBOL)
                              i->ids.insert(rcol, id);
                        else if (i->cf == CharFormat::STYLED) {
                              TFragment f2 = i->split(rcol);
                              i = _text.insert(i+1, TFragment(id));
                              _text.insert(i+1, f2);
                              }
                        return;
                        }
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            }
      if (_text.back().cf == CharFormat::SYMBOL)
            _text.back().ids.append(id);
      else
            _text.append(TFragment(id));
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TLine::remove(int column)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int idx  = 0;
            int rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (c.isSurrogate())
                              i->text.remove(rcol, 1);
                        i->text.remove(rcol, 1);
                        if (i->cf == CharFormat::SYMBOL)
                              i->ids.removeAt(idx);
                        if (i->text.isEmpty() && (_text.size() > 1))
                              _text.erase(i);
                        return;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            }
      qDebug("TLine::remove: column %d not found", column);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TLine::remove(int start, int n)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end();) {
            int idx  = 0;
            int rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == start) {
                        if (c.isSurrogate())
                              i->text.remove(rcol, 1);
                        i->text.remove(rcol, 1);
                        if (i->cf == CharFormat::SYMBOL)
                              i->ids.removeAt(idx);
                        if (i->text.isEmpty() && (_text.size() > 1))
                              i = _text.erase(i);
                        --n;
                        if (n == 0)
                              return;
                        continue;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            ++i;
            }
      qDebug("TLine::remove: column %d not found", start);
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TLine TLine::split(int column)
      {
      TLine tl;

      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int idx = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (idx) {
                              if (idx < i->text.size()) {
                                    tl._text.append(TFragment(i->text.mid(idx)));
                                    i->text = i->text.left(idx);
                                    if (i->cf == CharFormat::SYMBOL) {
                                          QList<SymId> l1, l2;
                                          for (int k = 0; k < i->ids.size(); ++k) {
                                                if (k < idx)
                                                      l1.append(i->ids[k]);
                                                else
                                                      l2.append(i->ids[k]);
                                                }
                                          i->ids = l1;
                                          tl._text.back().cf = CharFormat::SYMBOL;
                                          tl._text.back().ids = l2;
                                          }
                                    ++i;
                                    }
                              }
                        for (; i != _text.end(); i = _text.erase(i))
                              tl._text.append(*i);
                        if (tl._text.isEmpty())
                              tl.setText("");
                        return tl;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  }
            }
      tl._text.append(TFragment(""));
      return tl;
      }

//---------------------------------------------------------
//   SimpleText
//---------------------------------------------------------

SimpleText::SimpleText(Score* s)
   : Element(s)
      {
      if (s)
            _textStyle = s->textStyle(TEXT_STYLE_DEFAULT);
      _layoutToParentWidth = false;
      _editMode            = false;
      }

SimpleText::SimpleText(const SimpleText& st)
   : Element(st)
      {
      _text                = st._text;
      _layout              = st._layout;
      _textStyle           = st._textStyle;
      _layoutToParentWidth = st._layoutToParentWidth;
      frame                = st.frame;
      _editMode            = false;
      }

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void SimpleText::drawSelection(QPainter* p, const QRectF& r) const
      {
      QBrush bg(QColor("steelblue"));
      p->setCompositionMode(QPainter::CompositionMode_HardLight);
      p->setBrush(bg);
      p->setPen(Qt::NoPen);
      p->drawRect(r);
      p->setCompositionMode(QPainter::CompositionMode_SourceOver);
      p->setPen(textColor());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SimpleText::draw(QPainter* p) const
      {
      p->setFont(textStyle().fontPx(spatium()));
      p->setBrush(Qt::NoBrush);
      p->setPen(curColor());
      if (_editMode && _cursor.hasSelection()) {
            int r1 = _cursor.selectLine;
            int r2 = _cursor.line;
            int c1 = _cursor.selectColumn;
            int c2 = _cursor.column;

            if (r1 > r2) {
                  qSwap(r1, r2);
                  qSwap(c1, c2);
                  }
            else if (r1 == r2) {
                  if (c1 > c2)
                        qSwap(c1, c2);
                  }
            int row = 0;
            for (const TLine& t : _layout) {
                  t.draw(p, this);
                  if (row >= r1 && row <= r2) {
                        QRectF br;
                        if (row == r1 && r1 == r2)
                              br = t.boundingRect(c1, c2, this);
                        else if (row == r1)
                              br = t.boundingRect(c1, t.columns(), this);
                        else if (row == r2)
                              br = t.boundingRect(0, c2, this);
                        else
                              br = t.boundingRect();
                        drawSelection(p, br);
                        }
                  ++row;
                  }
            }
      else {
            for (const TLine& t : _layout)
                  t.draw(p, this);
            }

      if (_editMode) {
            p->setBrush(curColor());
            QPen pen(curColor());
            pen.setJoinStyle(Qt::MiterJoin);
            p->setPen(pen);
            p->drawRect(cursorRect());
            }
      }

//---------------------------------------------------------
//   cursorRect
//---------------------------------------------------------

QRectF SimpleText::cursorRect() const
      {
      QFontMetricsF fm(_textStyle.fontPx(spatium()));
      const TLine& tline = curLine();

      qreal h = fm.ascent() * 1.2;  // lineSpacing();
      qreal x = tline.xpos(_cursor.column, this);
      qreal y = tline.y();
      x      -= 1.0;   //??
      y      -= fm.ascent();
      qreal w = fm.width(QChar('w')) * .10;
      return QRectF(x, y, w, h);
      }

//---------------------------------------------------------
//   drawFrame
//---------------------------------------------------------

void SimpleText::drawFrame(QPainter* painter) const
      {
      if (!textStyle().hasFrame())
            return;

      QColor color(frameColor());
      if (!visible())
            color = Qt::gray;
      else if (selected())
            color = MScore::selectColor[0];
      if (frameWidth().val() != 0.0) {
            QPen pen(color, frameWidth().val() * spatium());
            painter->setPen(pen);
            }
      else
            painter->setPen(Qt::NoPen);
      QColor bg(backgroundColor());
      painter->setBrush(bg.alpha() ? QBrush(bg) : Qt::NoBrush);
      if (circle())
            painter->drawArc(frame, 0, 5760);
      else {
            int r2 = frameRound() * lrint((frame.width() / frame.height()));
            if (r2 > 99)
                  r2 = 99;
            painter->drawRoundRect(frame, frameRound(), r2);
            }
      }

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

QColor SimpleText::textColor() const
      {
      if (!score()->printing()) {
            QColor color;
            if (selected())
                  return MScore::selectColor[0];
            if (!visible())
                  return Qt::gray;
            }
      return textStyle().foregroundColor();
      }

//---------------------------------------------------------
//   layout
//    convert _text to _layout
//---------------------------------------------------------

void SimpleText::layout()
      {
      QFontMetricsF fm(_textStyle.fontPx(spatium()));

      if (!_editMode) {
            QStringList sl = _text.split('\n');
            _layout.clear();
            if (parent() && layoutToParentWidth()) {
                  Element* e = parent();
                  qreal w = e->width();
                  if (e->type() == HBOX || e->type() == VBOX || e->type() == TBOX) {
                        Box* b = static_cast<Box*>(e);
                        w -= ((b->leftMargin() + b->rightMargin()) * MScore::DPMM);
                        }
                  foreach (const QString& s, sl) {
                        if (fm.width(s) < w)
                              _layout.append(TLine(s));
                        else {
                              int n = s.size();
                              int sidx = 0;
                              int eidx = n-1;
                              while (eidx > sidx) {
                                    while (fm.width(s.mid(sidx, eidx-sidx+1)) > w) {
                                          --eidx;
                                          while (eidx > sidx) {
                                                if (s[eidx].isSpace())
                                                      break;
                                                --eidx;
                                                }
                                          }
                                    if (eidx == sidx)
                                         eidx = n-1;
                                    _layout.append(TLine(s.mid(sidx, eidx-sidx+1)));
                                    sidx = eidx;
                                    eidx = n-1;
                                    }
                              }
                        }
                  }
            else {
                  foreach (const QString& s, sl)
                        _layout.append(TLine(s));
                  }
            }
      if (_layout.isEmpty())
            _layout.append(TLine());
      QPointF o(_textStyle.offset(spatium()));

      QRectF bb;
      qreal lh = lineHeight();
      qreal ly = .0;
      QRectF cr(0, - fm.ascent(), 0, fm.height());

      for (TLine& t : _layout) {
            qreal y = ly;
            if (align() & ALIGN_BOTTOM)
                  y += -cr.bottom();
            else if (align() & ALIGN_VCENTER)
                  y +=  -(cr.top() + cr.bottom()) * .5;
            else if (align() & ALIGN_BASELINE)
                  ;
            else  // ALIGN_TOP
                  y += -cr.top();
            t.layout(y, this);

            QRectF r(t.boundingRect());

            qreal rx;
            if (align() & ALIGN_RIGHT)
                  rx = -r.right();
            else if (align() & ALIGN_HCENTER)
                  rx = -(r.left() + r.right()) * .5;
            else  // ALIGN_LEFT
                  rx = -r.left();
            t.moveX(rx);

            bb |= t.boundingRect();

            ly += lh;
            }
      setPos(o);
      if (_editMode)
            bb |= cursorRect();
      setbbox(bb);
      }

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void SimpleText::layoutFrame()
      {
      frame = bbox();
      if (circle()) {
            if (frame.width() > frame.height()) {
                  frame.setY(frame.y() + (frame.width() - frame.height()) * -.5);
                  frame.setHeight(frame.width());
                  }
            else {
                  frame.setX(frame.x() + (frame.height() - frame.width()) * -.5);
                  frame.setWidth(frame.height());
                  }
            }
      qreal _spatium = spatium();
      qreal w = (paddingWidth() + frameWidth() * .5f).val() * _spatium;
      frame.adjust(-w, -w, w, w);
      w = frameWidth().val() * _spatium;
      setbbox(frame.adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal SimpleText::lineSpacing() const
      {
      return QFontMetricsF(textStyle().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal SimpleText::lineHeight() const
      {
      return QFontMetricsF(textStyle().font(spatium())).height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal SimpleText::baseLine() const
      {
      return QFontMetricsF(textStyle().font(spatium())).ascent();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SimpleText::startEdit(MuseScoreView*, const QPointF& pt)
      {
      _cursor.line   = 0;
      _cursor.column = 0;
      _cursor.selectLine = 0;
      _cursor.selectColumn = 0;
      if (_layout.isEmpty())
            layout();
      setCursor(pt);
      undoPushProperty(P_TEXT);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SimpleText::endEdit()
      {
      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      _text.clear();
      for (const TLine& tl : _layout) {
            if (!_text.isEmpty())
                  _text += "\n";
            _text += tl.text();
            }
      if (links()) {
            foreach(Element* e, *links()) {
                  if (e == this)
                        continue;
                  e->undoChangeProperty(P_TEXT, _text);
                  }
            }
      }

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

const TLine& SimpleText::curLine() const
      {
      return _layout[_cursor.line];
      }

TLine& SimpleText::curLine()
      {
      return _layout[_cursor.line];
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool SimpleText::edit(MuseScoreView*, int, int key,
   Qt::KeyboardModifiers modifiers, const QString& _s)
      {
      QRectF refresh(canvasBoundingRect());
      QString s = _s;
      QTextCursor::MoveMode mm = (modifiers & Qt::ShiftModifier)
         ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

      switch (key) {
            case Qt::Key_Return:
                  if (_cursor.hasSelection())
                        deleteSelectedText();

                  _layout.insert(_cursor.line + 1, curLine().split(_cursor.column));
                  ++_cursor.line;
                  _cursor.column = 0;
                  s.clear();
                  _cursor.selectLine   = _cursor.line;
                  _cursor.selectColumn = _cursor.column;
                  break;

            case Qt::Key_Backspace:
                  if (_cursor.hasSelection())
                        deleteSelectedText();
                  else if (!deletePreviousChar())
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Delete:
                  if (_cursor.hasSelection())
                        deleteSelectedText();
                  else if (!deleteChar())
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Left:
                  if (!movePosition(QTextCursor::Left, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Right:
                  if (!movePosition(QTextCursor::Right, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Up:
                  movePosition(QTextCursor::Up, mm);
                  s.clear();
                  break;

            case Qt::Key_Down:
                  movePosition(QTextCursor::Down, mm);
                  s.clear();
                  break;

            case Qt::Key_Home:
                  movePosition(QTextCursor::Start, mm);
                  s.clear();
                  break;

            case Qt::Key_End:
                  movePosition(QTextCursor::End, mm);
                  s.clear();
                  break;

            case Qt::Key_Space:
                  s = " ";
                  break;

            case Qt::Key_Minus:
                  s = "-";
                  break;

            case Qt::Key_F:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d191");
                  break;
            case Qt::Key_M:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d190");
                  break;
            case Qt::Key_P:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d18f");
                  break;
            case Qt::Key_Z:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d18e");
                  break;
            case Qt::Key_S:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d18d");
                  break;
            case Qt::Key_R:
                  if (modifiers & Qt::ControlModifier)
                        s = QString::fromUtf8(u8"\U0001d18c");
            case Qt::Key_A:
                  if (modifiers & Qt::ControlModifier)
                        selectAll();
                  break;
            default:
                  break;
            }
      if (!s.isEmpty()) {
            if (!s[0].isPrint())
                  s = "";
            else {
                  insertText(s);
                  }
            }
      layout();
      if (parent() && parent()->type() == TBOX) {
            TBox* tbox = static_cast<TBox*>(parent());
            tbox->layout();
            System* system = tbox->system();
            system->setHeight(tbox->height());
            score()->doLayoutPages();
            score()->setUpdateAll(true);
            }
      else {
            static const qreal w = 2.0; // 8.0 / view->matrix().m11();
            refresh |= canvasBoundingRect();
            score()->addRefresh(refresh.adjusted(-w, -w, w, w));
            }
      return true;
      }

//---------------------------------------------------------
//   insertText
//---------------------------------------------------------

void SimpleText::insertText(const QString& s)
      {
      if (s.isEmpty())
            return;
      if (_cursor.hasSelection())
            deleteSelectedText();
      curLine().insert(_cursor.column, s);
      _cursor.column += s.size();
      _cursor.selectColumn = _cursor.column;
      }

//---------------------------------------------------------
//   insertSym
//---------------------------------------------------------

void SimpleText::insertSym(SymId id)
      {
      if (_cursor.hasSelection())
            deleteSelectedText();
      curLine().insert(_cursor.column, id);
      _cursor.column += 1;
      _cursor.selectColumn = _cursor.column;
      layout();
      }

//---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void SimpleText::selectAll()
      {
      _cursor.selectLine = 0;
      _cursor.selectColumn = 0;
      _cursor.line = _layout.size() - 1;
      _cursor.column = curLine().columns();
      }

//---------------------------------------------------------
//   deletePreviousChar
//---------------------------------------------------------

bool SimpleText::deletePreviousChar()
      {
      if (_cursor.column == 0) {
            if (_cursor.line == 0)
                  return false;
            const TLine& l1 = _layout.at(_cursor.line);
            TLine& l2       = _layout[_cursor.line - 1];
            _cursor.column  = l2.columns();
            for (const TFragment& f : l1.fragments())
                  l2.fragments().append(f);
            _layout.removeAt(_cursor.line);
            --_cursor.line;
            }
      else {
            --_cursor.column;
            curLine().remove(_cursor.column);
            }
      _cursor.selectLine   = _cursor.line;
      _cursor.selectColumn = _cursor.column;
      return true;
      }

//---------------------------------------------------------
//   deleteChar
//---------------------------------------------------------

bool SimpleText::deleteChar()
      {
      if (_cursor.column >= curLine().columns()) {
            if (_cursor.line + 1 < _layout.size()) {
                  TLine& l1       = _layout[_cursor.line];
                  const TLine& l2 = _layout[_cursor.line + 1];
                  for (const TFragment& f : l2.fragments())
                        l1.fragments().append(f);
                  _layout.removeAt(_cursor.line + 1);
                  return true;
                  }
            return false;
            }
      curLine().remove(_cursor.column);
      _cursor.selectLine   = _cursor.line;
      _cursor.selectColumn = _cursor.column;
      return true;
      }

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

bool SimpleText::movePosition(QTextCursor::MoveOperation op,
   QTextCursor::MoveMode mode)
      {
      switch(op) {
            case QTextCursor::Left:
                  if (_cursor.column == 0) {
                        if (_cursor.line == 0)
                              return false;
                        --_cursor.line;
                        _cursor.column = curLine().columns();
                        }
                  else
                        --_cursor.column;
                  break;

            case QTextCursor::Right:
                  if (_cursor.column >= curLine().columns()) {
                        if (_cursor.line >= _layout.size()-1)
                              return false;
                        ++_cursor.line;
                        _cursor.column = 0;
                        }
                  else
                        ++_cursor.column;
                  break;

            case QTextCursor::Up:
                  if (_cursor.line == 0)
                        return false;
                  --_cursor.line;
                  if (_cursor.column > curLine().columns())
                        _cursor.column = curLine().columns();
                  break;

            case QTextCursor::Down:
                  if (_cursor.line >= _layout.size()-1)
                        return false;
                  ++_cursor.line;
                  if (_cursor.column > curLine().columns())
                        _cursor.column = curLine().columns();
                  break;

            case QTextCursor::Start:
                  _cursor.line   = 0;
                  _cursor.column = 0;
                  break;

            case QTextCursor::End:
                  _cursor.line = _layout.size() - 1;
                  _cursor.column = curLine().columns();
                  break;

            default:
                  qDebug("SimpleText::movePosition: not implemented");
                  return false;
            }
      if (mode == QTextCursor::MoveAnchor) {
            _cursor.selectLine   = _cursor.line;
            _cursor.selectColumn = _cursor.column;
            }
      return true;
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

bool SimpleText::setCursor(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;
      _cursor.line = 0;
      for (int row = 0; row < _layout.size(); ++row) {
            const TLine& l = _layout.at(row);
            if (l.fragments().front().pos.y() > pt.y()) {
                  _cursor.line = row;
                  break;
                  }
            }
      _cursor.column = curLine().column(pt.x(), this);
      score()->setUpdateAll(true);
      if (mode == QTextCursor::MoveAnchor) {
            _cursor.selectLine   = _cursor.line;
            _cursor.selectColumn = _cursor.column;
            }
      if (_cursor.hasSelection())
            QApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
      return true;
      }

//---------------------------------------------------------
//   selectedText
//    return current selection
//---------------------------------------------------------

QString SimpleText::selectedText() const
      {
      QString s;
      int r1 = _cursor.selectLine;
      int r2 = _cursor.line;
      int c1 = _cursor.selectColumn;
      int c2 = _cursor.column;

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _layout.size();
      for (int row = 0; row < rows; ++row) {
            const TLine& t = _layout.at(row);
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        s += t.text().mid(c1, c2 - c1);
                  else if (row == r1)
                        s += t.text().mid(c1);
                  else if (row == r2)
                        s += t.text().left(c2);
                  else
                        s += t.text();
                  }
            if(row != rows -1)
                  s += "\n";
            }
      return s;
      }

//---------------------------------------------------------
//   deleteSelectedText
//---------------------------------------------------------

void SimpleText::deleteSelectedText()
      {
      int r1 = _cursor.selectLine;
      int r2 = _cursor.line;
      int c1 = _cursor.selectColumn;
      int c2 = _cursor.column;

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _layout.size();
      QList<TLine> toDelete;
      for (int row = 0; row < rows; ++row) {
            TLine& t = _layout[row];
            if (row >= r1 && row <= r2) {             // TODO: does not work with symbols
                  if (row == r1 && r1 == r2)
                        t.remove(c1, c2 - c1);
                  else if (row == r1)
                        t.remove(c1, t.text().size() - c1);
                  else if (row == r2)
                        t.remove(0, c2);
                  else {
                        toDelete.append(t);
                        }
                  }
            }
      if (r1 != r2) {
            TLine& l1 = _layout[r1];
            const TLine& l2 = _layout[r2];
            for (const TFragment& f : l2.fragments())
                  l1.fragments().append(f);

            _layout.removeAt(r2);
            QMutableListIterator<TLine> i(_layout);
            while (i.hasNext()) {
                  if (toDelete.contains(i.next()))
                        i.remove();
                  }
            }
      _cursor.selectLine   = r1;
      _cursor.line   = r1;
      _cursor.selectColumn = c1;
      _cursor.column = c1;
      }

}

