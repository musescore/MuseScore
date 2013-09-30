//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

namespace Ms {

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
      // p->setPen(textColor());
      p->setPen(curColor());
      int rows = _layout.size();
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
            for (int row = 0; row < rows; ++row) {
                  const TLine& t = _layout.at(row);
                  p->drawText(t.pos, t.text);
                  if (row >= r1 && row <= r2) {
                        QBrush bg(QColor("steelblue"));
                        QFontMetricsF fm(_textStyle.fontPx(spatium()));
                        QRectF br;
                        if (row == r1 && r1 == r2) {
                              QString left  = t.text.left(c1);
                              QString mid   = t.text.mid(c1, c2 - c1);
                              QString right = t.text.mid(c2);

                              QPointF r (fm.width(left), 0.0);
                              br = fm.boundingRect(mid).translated(t.pos + r);
                              br.setWidth(fm.width(mid));
                              }
                        else if (row == r1) {
                              QString left  = t.text.left(c1);
                              QString right = t.text.mid(c1);

                              QPointF r (fm.width(left), 0.0);
                              br = fm.boundingRect(right).translated(t.pos + r);
                              br.setWidth(fm.width(right));
                              }
                        else if (row == r2) {
                              QString left  = t.text.left(c2);

                              br = fm.boundingRect(left).translated(t.pos);
                              br.setWidth(fm.width(left));
                              }
                        else  {
                              br = fm.boundingRect(t.text).translated(t.pos);
                              br.setWidth(fm.width(t.text));
                              }
                        drawSelection(p, br);
                        }
                  }
            }
      else {
            for (int row = 0; row < rows; ++row) {
                  const TLine& t = _layout.at(row);
                  p->drawText(t.pos, t.text);
                  }
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
      int line   = _cursor.line;
      QPointF pt = _layout[line].pos;
      qreal xo   = fm.width(_layout[line].text.left(_cursor.column));

      qreal x = pt.x() + xo - 1;
      qreal y = pt.y() - fm.ascent();
      qreal w = fm.width(QChar('w')) * .10;
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
                  foreach (QString s, sl)
                        _layout.append(TLine(s));
                  }
            }
      if (_layout.isEmpty())
            _layout.append(TLine());
      QPointF o(_textStyle.offset(spatium()));

      QRectF bb;
      qreal lh = lineHeight();
      qreal ly = .0;
      int n = _layout.size();
      QRectF cr(0, - fm.ascent(), 0, fm.height());

      for (int i = 0; i < n; ++i) {
            TLine* t = &_layout[i];
            QRectF r(fm.tightBoundingRect(t->text));

            t->pos.ry() = ly;
            if (align() & ALIGN_BOTTOM)
                  t->pos.ry() += -cr.bottom();
            else if (align() & ALIGN_VCENTER)
                  t->pos.ry() +=  -(cr.top() + cr.bottom()) * .5;
            else if (align() & ALIGN_BASELINE)
                  ;
            else  // ALIGN_TOP
                  t->pos.ry() += -cr.top();

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
      foreach(const TLine& tl, _layout) {
            if (!_text.isEmpty())
                  _text += "\n";
            _text += tl.text;
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
                  if(_cursor.hasSelection()) {
                        deleteSelectedText();
                        }
                  QString left(curLine().left(_cursor.column));
                  QString right(curLine().mid(_cursor.column));
                  curLine() = left;
                  ++_cursor.line;
                  _cursor.column = 0;
                  _layout.insert(_cursor.line, TLine(right));
                  s.clear();
                  _cursor.selectLine   = _cursor.line;
                  _cursor.selectColumn = _cursor.column;
                  }
                  break;

            case Qt::Key_Backspace:
                  if(_cursor.hasSelection()) {
                        deleteSelectedText();
                        }
                  else if (!deletePreviousChar())
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Delete:
                  if(_cursor.hasSelection()) {
                        deleteSelectedText();
                        }
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
      if (!s.isEmpty()) {
            if(_cursor.hasSelection()) {
                  deleteSelectedText();
                  }
            curLine().insert(_cursor.column, s);
            _cursor.column += s.size();
            _cursor.selectColumn = _cursor.column;
            }
      }

 //---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void SimpleText::selectAll()
      {
      _cursor.selectLine = 0;
      _cursor.selectColumn = 0;
      _cursor.line = _layout.size() - 1;
      _cursor.column = curLine().size();
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
      _cursor.selectLine   = _cursor.line;
      _cursor.selectColumn = _cursor.column;
      return true;
      }

//---------------------------------------------------------
//   deleteChar
//---------------------------------------------------------

bool SimpleText::deleteChar()
      {
      if (_cursor.column >= curLine().size()) {
            if (_cursor.line + 1 < _layout.size()) {
                  TLine l = _layout.at(_cursor.line + 1);
                  curLine() += l.text;
                  _layout.removeAt(_cursor.line + 1);
                  return true;
                  }
            return false;
            }
      if (_cursor.column > 0 && curLine().at(_cursor.column-1).isHighSurrogate())
            curLine().remove(_cursor.column, 1);
      curLine().remove(_cursor.column, 1);
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
                        _cursor.column = curLine().size();
                        }
                  else {
                        if (curLine().at(_cursor.column-1).isLowSurrogate())
                              --_cursor.column;
                        --_cursor.column;
                        }
                  break;

            case QTextCursor::Right:
                  if (_cursor.column >= curLine().size()) {
                        if (_cursor.line >= _layout.size()-1)
                              return false;
                        ++_cursor.line;
                        _cursor.column = 0;
                        }
                  else {
                        if (curLine().at(_cursor.column).isHighSurrogate())
                              ++_cursor.column;
                        ++_cursor.column;
                        }
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
                  _cursor.line   = 0;
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
            if (l.pos.y() > pt.y()) {
                  _cursor.line = row;
                  break;
                  }
            }
      qreal x = _layout.at(_cursor.line).pos.x();
      const QString& s(curLine());
      QFontMetricsF fm(_textStyle.fontPx(spatium()));
      int col = 0;
      for (; col < s.size(); col++) {
            if (s.at(col).isLowSurrogate())
                  continue;
            qreal w = fm.width(s.left(col));
            qreal prevWidth = fm.width(s.left(col-1));
            qreal nextWidth = fm.width(s.left(col+1));

            qreal off = (col == 0) ? 0 : (prevWidth + (w - prevWidth) / 2);
            qreal off2 = w + (nextWidth - w) / 2;
            if (x + off < pt.x() && pt.x() < x + off2) {
                  break;
                  }
            }
      _cursor.column = col;

      score()->setUpdateAll(true);
      if (mode == QTextCursor::MoveAnchor) {
            _cursor.selectLine = _cursor.line;
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
                        s += t.text.mid(c1, c2 - c1);
                  else if (row == r1)
                        s += t.text.mid(c1);
                  else if (row == r2)
                        s += t.text.left(c2);
                  else
                        s += t.text;
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
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        t.text.remove(c1, c2 - c1);
                  else if (row == r1)
                        t.text.remove(c1, t.text.size() - c1);
                  else if (row == r2)
                        t.text.remove(0, c2);
                  else {
                        toDelete.append(t);
                        }
                  }
            }
      if (r1 != r2) {
            TLine& tleft = _layout[r1];
            TLine& tright = _layout[r2];
            tleft.text.append(tright.text);
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

