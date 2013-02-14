//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

TCursor SimpleText::_cursor;

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

SimpleText::~SimpleText()
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SimpleText::draw(QPainter* p) const
      {
      drawFrame(p);
      p->setFont(textStyle().fontPx(spatium()));
      p->setBrush(Qt::NoBrush);
      p->setPen(textColor());
      foreach(const TLine& t, _layout)
            p->drawText(t.pos, t.text);

      if (_editMode) {
            p->setBrush(QColor("steelblue"));
            p->drawRect(cursorRect());
            }
      }

//---------------------------------------------------------
//   cursorRect
//---------------------------------------------------------

QRectF SimpleText::cursorRect() const
      {
      QFontMetricsF fm(_textStyle.fontPx(spatium()));
      int line   = _cursor.line;
      qreal lh   = lineHeight();
      QPointF pt = _layout[line].pos;
      qreal xo   = fm.width(_layout[line].text.left(_cursor.column));

      qreal x = pt.x() + xo - 1;
      qreal y = pt.y() - fm.ascent();
      qreal w = 2;
      qreal h = fm.ascent() + fm.descent() + 1;

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
            color = Qt::blue;
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
//   firstLine
//---------------------------------------------------------

const QString& SimpleText::firstLine() const
      {
      const TLine* t = &_layout[0];
      return t->text;
      }

//---------------------------------------------------------
//   layout
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
                  foreach(QString s, sl) {
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
                  foreach(QString s, sl)
                        _layout.append(TLine(s));
                  }
            }
      int n = _layout.size();
      if (!n) {
            setPos(QPointF());
            setbbox(QRectF());
            return;
            }

      QPointF o(_textStyle.offset(spatium()));

      QRectF bb;
      qreal lh = lineHeight();
      qreal ly = .0;
      for (int i = 0; i < n; ++i) {
            TLine* t = &_layout[i];

            QRectF r(fm.tightBoundingRect(t->text));

            t->pos.ry() = ly;
            if (align() & ALIGN_BOTTOM)
                  t->pos.ry() += -r.bottom();
            else if (align() & ALIGN_VCENTER)
                  t->pos.ry() +=  -(r.top() + r.bottom()) * .5;
            else if (align() & ALIGN_BASELINE)
                  ;
            else  // ALIGN_TOP
                  t->pos.ry() += -r.top();

            if (align() & ALIGN_RIGHT)
                  t->pos.rx() = -r.right();
            else if (align() & ALIGN_HCENTER)
                  t->pos.rx() = -(r.left() + r.right()) * .5;
            else  // ALIGN_LEFT
                  t->pos.rx() = -r.left();
            bb |= r.translated(t->pos);
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
//   setText
//---------------------------------------------------------

void SimpleText::setText(const QString& s)
      {
      _text = s;
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString SimpleText::getText() const
      {
      return _text;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SimpleText::startEdit(MuseScoreView*, const QPointF& pt)
      {
      _cursor.line   = 0;
      _cursor.column = 0;
      _editMode      = true;
      setCursor(pt);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SimpleText::endEdit()
      {
      _editMode = false;
      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      _text.clear();
      foreach(const TLine& tl, _layout) {
            if (!_text.isEmpty())
                  _text += "\n";
            _text += tl.text;
            }
      }

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

QString& SimpleText::curLine()
      {
      return _layout[_cursor.line].text;
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
                  {
                  QString left(curLine().left(_cursor.column));
                  QString right(curLine().mid(_cursor.column));
                  curLine() = left;
                  ++_cursor.line;
                  _cursor.column = 0;
                  _layout.insert(_cursor.line, TLine(right));
                  s.clear();
                  }
                  break;

            case Qt::Key_Backspace:
                  if (!deletePreviousChar())
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Delete:
                  if (!deleteChar())
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

            default:
                  break;
            }

      if (!s.isEmpty()) {
            curLine().insert(_cursor.column, s);
            ++_cursor.column;
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
//   deletePreviousChar
//---------------------------------------------------------

bool SimpleText::deletePreviousChar()
      {
      if (_cursor.column == 0) {
            if (_cursor.line == 0)
                  return false;
            QString right = curLine();
            _layout.removeAt(_cursor.line);
            --_cursor.line;
            _cursor.column = curLine().size();
            curLine().append(right);
            }
      else {
            QChar c(curLine().at(_cursor.column-1));
            if (c.isLowSurrogate()) {
                  curLine().remove(_cursor.column-1, 1);
                  --_cursor.column;
                  }
            curLine().remove(_cursor.column-1, 1);
            --_cursor.column;
            }
      return true;
      }

//---------------------------------------------------------
//   deleteChar
//---------------------------------------------------------

bool SimpleText::deleteChar()
      {
      if (_cursor.column >= curLine().size())
            return false;
      if (curLine().at(_cursor.column-1).isHighSurrogate())
            curLine().remove(_cursor.column, 1);
      curLine().remove(_cursor.column, 1);
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
                  if (_cursor.column == 0)
                        return false;
                  if (curLine().at(_cursor.column-1).isLowSurrogate())
                        --_cursor.column;
                  --_cursor.column;
                  break;

            case QTextCursor::Right:
                  if (_cursor.column >= curLine().size())
                        return false;
                  if (curLine().at(_cursor.column).isHighSurrogate())
                        ++_cursor.column;
                  ++_cursor.column;
                  break;

            case QTextCursor::Up:
                  if (_cursor.line == 0)
                        return false;
                  --_cursor.line;
                  if (_cursor.column > curLine().size())
                        _cursor.column = curLine().size();
                  break;

            case QTextCursor::Down:
                  if (_cursor.line >= _layout.size()-1)
                        return false;
                  ++_cursor.line;
                  if (_cursor.column > curLine().size())
                        _cursor.column = curLine().size();
                  break;

            case QTextCursor::Start:
                  _cursor.line = 0;
                  _cursor.column = 0;
                  break;

            case QTextCursor::End:
                  _cursor.line = _layout.size() - 1;
                  _cursor.column = curLine().size();
                  break;
            default:
                  qDebug("SimpleText::movePosition: not implemented");
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   moveCursorToEnd
//---------------------------------------------------------

void SimpleText::moveCursorToEnd()
      {
      printf("moveCursorToEnd\n");
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void SimpleText::moveCursor(int col)
      {
      printf("moveCursor\n");
      }

//---------------------------------------------------------
//   addChar
//---------------------------------------------------------

void SimpleText::addChar(int code)
      {
      QString ss;
      if (code & 0xffff0000) {
            ss = QChar(QChar::highSurrogate(code));
            ss += QChar(QChar::lowSurrogate(code));
            }
      else
            ss = QChar(code);
      curLine().insert(_cursor.column, ss);
      _cursor.column += ss.size();
      score()->setLayoutAll(true);
      score()->end();
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
            if (l.pos.y() > pt.y()) {
                  _cursor.line = row == 0 ? 0 : row-1;
                  break;
                  }
            }
      qreal x = _layout.at(_cursor.line).pos.x();
      const QString& s(curLine());
      QFontMetricsF fm(_textStyle.fontPx(spatium()));
      for (int col = 0; col < s.size(); ++col) {
            if (x + fm.width(s.left(col))  > pt.x()) {
                  _cursor.column = col;
                  break;
                  }
            }
      score()->setUpdateAll(true);
      }

