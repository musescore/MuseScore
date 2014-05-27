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

#include "text.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "box.h"
#include "textframe.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

static const qreal subScriptSize   = 0.6;
static const qreal subScriptOffset = 0.5;       // of x-height
static const qreal superScriptOffset = -0.5;       // of x-height

TextCursor Text::_cursor;

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool CharFormat::operator==(const CharFormat& cf) const
      {
      if (cf.type() != type())
            return false;
      if (cf.bold() != bold()
         || cf.italic() != italic()
         || cf.underline() != underline()
         || cf.valign() != valign()
         || cf.fontSize() != fontSize())
            return false;
      if (type() == CharFormatType::TEXT)
            return cf.fontFamily() == fontFamily();
      return true;
      }

//---------------------------------------------------------
//   clearSelection
//---------------------------------------------------------

void TextCursor::clearSelection()
      {
      _selectLine   = _line;
      _selectColumn = _column;
      }

//---------------------------------------------------------
//   initFromStyle
//---------------------------------------------------------

void TextCursor::initFromStyle(const TextStyle& s)
      {
      _format.setFontFamily(s.family());
      _format.setFontSize(s.size());
      _format.setBold(s.bold());
      _format.setItalic(s.italic());
      _format.setUnderline(s.underline());
      _format.setValign(VerticalAlignment::AlignNormal);
      }

//---------------------------------------------------------
//   TextFragment
//---------------------------------------------------------

TextFragment::TextFragment()
      {
      }

TextFragment::TextFragment(const QString& s)
      {
      format.setType(CharFormatType::TEXT);
      text = s;
      }

TextFragment::TextFragment(TextCursor* cursor, SymId id)
      {
      format = *cursor->format();
      format.setType(CharFormatType::SYMBOL);
      ids.append(id);
      }

TextFragment::TextFragment(TextCursor* cursor, const QString& s)
      {
      format = *cursor->format();
      text = s;
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TextFragment TextFragment::split(int column)
      {
      int idx = 0;
      int col = 0;
      TextFragment f;
      f.format = format;

      for (const QChar& c : text) {
            if (col == column) {
                  if (idx) {
                        if (idx < text.size()) {
                              f.text = text.mid(idx);
                              text   = text.left(idx);
                              if (format.type() == CharFormatType::SYMBOL) {
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
//   columns
//---------------------------------------------------------

int TextFragment::columns() const
      {
      int col = 0;
      for (const QChar& c : text) {
            if (c.isHighSurrogate())
                  continue;
            ++col;
            }
      return col;
      }

//---------------------------------------------------------
//   operator ==
//---------------------------------------------------------

bool TextFragment::operator ==(const TextFragment& f) const
      {
      return format == f.format && (format.type() == CharFormatType::TEXT ? text == f.text : ids == f.ids);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextFragment::draw(QPainter* p, const Text* t) const
      {
      p->setFont(font(t));
      p->drawText(pos, text);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextFragment::font(const Text* t) const
      {
      QFont font;

      qreal m = format.fontSize() * MScore::DPI / PPI;
      if (t->textStyle().sizeIsSpatiumDependent())
            m *= t->spatium() / ( SPATIUM20 * MScore::DPI);

      font.setUnderline(format.underline());
      if (format.type() == CharFormatType::TEXT) {
            font.setFamily(format.fontFamily());
            font.setBold(format.bold());
            font.setItalic(format.italic());
            }
      else {
            bool fallback = false;
            for (SymId id : ids) {
                  if (!t->symIsValid(id)) {
                        fallback = true;
                        break;
                        }
                  }
            ScoreFont* f = fallback ? ScoreFont::fallbackFont() : t->score()->scoreFont();
            text.clear();
            for (SymId id : ids)
                  text.append(f->toString(id));
            font.setFamily(f->family());
            font.setWeight(QFont::Normal);  // if not set we get system default
            font.setStyleStrategy(QFont::NoFontMerging);
            font.setHintingPreference(QFont::PreferVerticalHinting);
            // if (f->family() == "Bravura")       // HACK: why are bravura dynamics are so small?
            //       m *= 1.9;
            }
      if (format.valign() != VerticalAlignment::AlignNormal)
            m *= subScriptSize;
      font.setPixelSize(lrint(m));
      return font;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextBlock::draw(QPainter* p, const Text* t) const
      {
      p->translate(0.0, _y);
      for (const TextFragment& f : _text)
            f.draw(p, t);
      p->translate(0.0, -_y);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBlock::layout(Text* t)
      {
      _bbox = QRectF();
      qreal x = 0.0;
      _lineSpacing = 0.0;

      qreal layoutWidth = 0;
      Element* e = t->parent();
      if (e && t->layoutToParentWidth()) {
            layoutWidth = e->width();
            if (e->type() == Element::ElementType::HBOX || e->type() == Element::ElementType::VBOX || e->type() == Element::ElementType::TBOX) {
                  Box* b = static_cast<Box*>(e);
                  layoutWidth -= ((b->leftMargin() + b->rightMargin()) * MScore::DPMM);
                  }
            }
      if (_text.isEmpty()) {
            QFontMetricsF fm(t->textStyle().fontPx(t->spatium()));
            _bbox.setRect(0.0, -fm.ascent(), 1.0, fm.ascent());
            _lineSpacing = fm.lineSpacing();
            }
      else {
            for (TextFragment& f : _text) {
                  f.pos.setX(x);
                  QFontMetricsF fm(f.font(t));
                  if (f.format.valign() != VerticalAlignment::AlignNormal) {
                        qreal voffset = fm.xHeight() / subScriptSize;   // use original height
                        if (f.format.valign() != VerticalAlignment::AlignNormal) {
                              if (f.format.valign() == VerticalAlignment::AlignSubScript)
                                    voffset *= subScriptOffset;
                              else
                                    voffset *= superScriptOffset;
                              }
                        f.pos.setY(voffset);
                        }
                  else
                        f.pos.setY(0.0);
                  qreal w;
                  QRectF r;
                  r = fm.tightBoundingRect(f.text).translated(f.pos);
                  w = fm.width(f.text);
                  _bbox |= r;
                  x += w;
                  _lineSpacing = qMax(_lineSpacing, fm.lineSpacing());
                  }
            }
      qreal rx;
      if (t->textStyle().align() & ALIGN_RIGHT)
            rx = layoutWidth-_bbox.right();
      else if (t->textStyle().align() & ALIGN_HCENTER)
            rx = (layoutWidth - (_bbox.left() + _bbox.right())) * .5;
      else  // ALIGN_LEFT
            rx = -_bbox.left();
      for (TextFragment& f : _text)
            f.pos.rx() += rx;
      _bbox.translate(rx, 0.0);
      }

//---------------------------------------------------------
//   xpos
//---------------------------------------------------------

qreal TextBlock::xpos(int column, const Text* t) const
      {
      int col = 0;
      for (const TextFragment& f : _text) {
            if (column == col)
                  return f.pos.x();
            QFontMetricsF fm(f.font(t));
            int idx = 0;
            for (const QChar& c : f.text) {
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  if (column == col)
                        return f.pos.x() + fm.width(f.text.left(idx));
                  }
            }
      return _bbox.x();
      }

//---------------------------------------------------------
//   fragment
//---------------------------------------------------------

const TextFragment* TextBlock::fragment(int column) const
      {
      if (_text.isEmpty())
            return 0;
      int col = 0;
      auto f = _text.begin();
      for (; f != _text.end(); ++f) {
            for (const QChar& c : f->text) {
                  if (c.isHighSurrogate())
                        continue;
                  if (column == col)
                        return &*f;
                  ++col;
                  }
            }
      if (column == col)
            return &*(f-1);
      return 0;
      }

//---------------------------------------------------------
//   formatAt
//---------------------------------------------------------

const CharFormat* TextBlock::formatAt(int column) const
      {
      const TextFragment* f = fragment(column);
      if (f)
            return &(f->format);
      return 0;
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF TextBlock::boundingRect(int col1, int col2, const Text* t) const
      {
      qreal x1 = xpos(col1, t);
      qreal x2 = xpos(col2, t);
      return QRectF(x1, _bbox.y(), x2-x1, _bbox.height());
      }

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int TextBlock::columns() const
      {
      int col = 0;
      for (const TextFragment& f : _text) {
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
//    Text coordinate system
//---------------------------------------------------------

int TextBlock::column(qreal x, Text* t) const
      {
      int col = 0;
      for (const TextFragment& f : _text) {
            int idx = 0;
            if (x <= f.pos.x())
                  return col;
            qreal px = 0.0;
            for (const QChar& c : f.text) {
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  QFontMetricsF fm(f.font(t));
                  qreal xo;
                  if (f.format.type() == CharFormatType::TEXT)
                        xo = fm.width(f.text.left(idx));
                  else
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

void TextBlock::insert(TextCursor* cursor, const QString& s)
      {
      int rcol;
      auto i = fragment(cursor->column(), &rcol);
      if (i != _text.end()) {
            if (i->format.type() == CharFormatType::TEXT) {
                  if (!(i->format == *cursor->format())) {
                        if (rcol == 0)
                              _text.insert(i, TextFragment(cursor, s));
                        else {
                              TextFragment f2 = i->split(rcol);
                              i = _text.insert(i+1, TextFragment(cursor, s));
                              _text.insert(i+1, f2);
                              }
                        }
                  else
                        i->text.insert(rcol, s);
                  }
            else {
                  if (rcol == 0) {
                        if (i != _text.begin() && (i-1)->format == *cursor->format())
                              (i-1)->text.append(s);
                        else
                              _text.insert(i, TextFragment(cursor, s));
                        }
                  else {
                        TextFragment f2 = i->split(rcol);
                        i = _text.insert(i+1, TextFragment(cursor, s));
                        f2.format = *cursor->format();
                        f2.format.setType(CharFormatType::SYMBOL);
                        _text.insert(i+1, f2);
                        }
                  }
            }
      else {
            if (!_text.isEmpty() && _text.back().format == *cursor->format())
                  _text.back().text.append(s);
            else
                  _text.append(TextFragment(cursor, s));
            }
      }

void TextBlock::insert(TextCursor* cursor, SymId id)
      {
      int rcol;
      auto i = fragment(cursor->column(), &rcol);
      if (i != _text.end()) {
            if (i->format.type() == CharFormatType::SYMBOL) {
                  if (!(i->format == *cursor->format())) {
                        if (rcol == 0)
                              _text.insert(i, TextFragment(cursor, id));
                        else {
                              TextFragment f2 = i->split(rcol);
                              i = _text.insert(i+1, TextFragment(cursor, id));
                              _text.insert(i+1, f2);
                              }
                        }
                  else
                        i->ids.insert(rcol, id);
                  }
            else if (i->format.type() == CharFormatType::TEXT) {
                  if (rcol == 0) {
                        if (i != _text.begin() && (i-1)->format == *cursor->format())
                              (i-1)->ids.append(id);
                        else
                              _text.insert(i, TextFragment(cursor, id));
                        }
                  else {
                        TextFragment f2 = i->split(rcol);
                        i = _text.insert(i+1, TextFragment(cursor, id));
                        _text.insert(i+1, f2);
                        }
                  }
            }
      else {
            if (!_text.isEmpty() && _text.back().format.type() == CharFormatType::SYMBOL)
                  _text.back().ids.append(id);
            else
                  _text.append(TextFragment(cursor, id));
            }
      }

//---------------------------------------------------------
//   fragment
//---------------------------------------------------------

QList<TextFragment>::iterator TextBlock::fragment(int column, int* rcol)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            *rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == column)
                        return i;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++*rcol;
                  }
            }
      return _text.end();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TextBlock::remove(int column)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int idx  = 0;
            int rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (c.isSurrogate())
                              i->text.remove(rcol, 1);
                        if (i->format.type() == CharFormatType::SYMBOL) {
                              i->ids.removeAt(idx);
                              if (i->ids.isEmpty())
                                    _text.erase(i);
                              }
                        else {
                              i->text.remove(rcol, 1);
                              if (i->text.isEmpty())
                                    _text.erase(i);
                              }
                        simplify();
                        return;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            }
//      qDebug("TextBlock::remove: column %d not found", column);
      }

//---------------------------------------------------------
//   simplify
//---------------------------------------------------------

void TextBlock::simplify()
      {
      if (_text.size() < 2)
            return;
      auto i = _text.begin();
      TextFragment* f = &*i;
      ++i;
      for (; i != _text.end(); ++i) {
            while (i != _text.end() && (i->format == f->format)) {
                  if (f->format.type() == CharFormatType::SYMBOL)
                        f->ids.append(i->ids);
                  else
                        f->text.append(i->text);
                  i = _text.erase(i);
                  }
            if (i == _text.end())
                  break;
            f = &*i;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TextBlock::remove(int start, int n)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end();) {
            int idx  = 0;
            int rcol = 0;
            bool inc = true;
            for (const QChar& c : i->text) {
                  if (col == start) {
                        if (c.isSurrogate())
                              i->text.remove(rcol, 1);
                        i->text.remove(rcol, 1);
                        if (i->format.type() == CharFormatType::SYMBOL)
                              i->ids.removeAt(idx);
                        if (i->text.isEmpty() && (_text.size() > 1)) {
                              i = _text.erase(i);
                              inc = false;
                              }
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
            if (inc)
                  ++i;
            }
      }

//---------------------------------------------------------
//   changeFormat
//---------------------------------------------------------

void TextBlock::changeFormat(FormatId id, QVariant data, int start, int n)
      {
      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int columns = i->columns();
            if (start + n <= col)
                  break;
            if (start >= col + columns) {
                  col += i->columns();
                  continue;
                  }
            int endCol = col + columns;

            if ((start <= col) && (start < endCol) && ((start+n) < endCol)) {
                  // left
                  TextFragment f = i->split(start + n - col);
                  i->changeFormat(id, data);
                  i = _text.insert(i+1, f);
                  }
            else if (start > col && ((start+n) < endCol)) {
                  // middle
                  TextFragment lf = i->split(start+n - col);
                  TextFragment mf = i->split(start - col);
                  mf.changeFormat(id, data);
                  i = _text.insert(i+1, mf);
                  i = _text.insert(i+1, lf);
                  }
            else if (start > col) {
                  // right
                  TextFragment f = i->split(start - col);
                  f.changeFormat(id, data);
                  i = _text.insert(i+1, f);
                  }
            else {
                  // complete fragment
                  i->changeFormat(id, data);
                  }
            col = endCol;
            }
      }

//---------------------------------------------------------
//   setFormat
//---------------------------------------------------------

void CharFormat::setFormat(FormatId id, QVariant data)
      {
      switch (id) {
            case FormatId::Bold:
                  _bold = data.toBool();
                  break;
            case FormatId::Italic:
                  _italic = data.toBool();
                  break;
            case FormatId::Underline:
                  _underline = data.toBool();
                  break;
            case FormatId::Valign:
                  _valign = static_cast<VerticalAlignment>(data.toInt());
                  break;
            case FormatId::FontSize:
                  _fontSize = data.toDouble();
                  break;
            case FormatId::FontFamily:
                  _fontFamily = data.toString();
                  break;
            }
      }

//---------------------------------------------------------
//   changeFormat
//---------------------------------------------------------

void TextFragment::changeFormat(FormatId id, QVariant data)
      {
      format.setFormat(id, data);
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TextBlock TextBlock::split(int column)
      {
      TextBlock tl;

      int col = 0;
      for (auto i = _text.begin(); i != _text.end(); ++i) {
            int idx = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (idx) {
                              if (idx < i->text.size()) {
                                    TextFragment tf(i->text.mid(idx));
                                    tf.format = i->format;
                                    tl._text.append(tf);
                                    i->text = i->text.left(idx);
                                    if (i->format.type() == CharFormatType::SYMBOL) {
                                          QList<SymId> l1, l2;
                                          for (int k = 0; k < i->ids.size(); ++k) {
                                                if (k < idx)
                                                      l1.append(i->ids[k]);
                                                else
                                                      l2.append(i->ids[k]);
                                                }
                                          i->ids = l1;
                                          tl._text.back().format.setType(CharFormatType::SYMBOL);
                                          tl._text.back().ids = l2;
                                          }
                                    ++i;
                                    }
                              }
                        for (; i != _text.end(); i = _text.erase(i))
                              tl._text.append(*i);
                        return tl;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  }
            }
      tl._text.append(TextFragment(""));  //??
      return tl;
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString TextBlock::text(int col1, int len) const
      {
      QString s;
      int col = 0;
      for (auto f : _text) {
            for (const QChar& c : f.text) {
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  if (col >= col1 && (len < 0 || ((col1-col) < len)))
                        s += c;
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : Element(s)
      {
      _styleIndex = TEXT_STYLE_DEFAULT;
      if (s)
            _textStyle = s->textStyle(TEXT_STYLE_DEFAULT);
      _layoutToParentWidth = false;
      _editMode            = false;
      setFlag(ElementFlag::MOVABLE, true);
      }

Text::Text(const Text& st)
   : Element(st)
      {
      _text                = st._text;
      _layout              = st._layout;
      frame                = st.frame;
      _styleIndex          = st._styleIndex;
      _layoutToParentWidth = st._layoutToParentWidth;
      _editMode            = false;
      _textStyle           = st._textStyle;
      }

//---------------------------------------------------------
//   updateCursorFormat
//---------------------------------------------------------

void Text::updateCursorFormat(TextCursor* cursor)
      {
      TextBlock* block = &_layout[cursor->line()];
      int column = cursor->hasSelection() ? cursor->selectColumn() : cursor->column();
      const CharFormat* format = block->formatAt(column);
      if (format)
            cursor->setFormat(*format);
      else
            cursor->initFromStyle(textStyle());
      }

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void Text::drawSelection(QPainter* p, const QRectF& r) const
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

void Text::draw(QPainter* p) const
      {
      if (textStyle().hasFrame()) {
            if (textStyle().frameWidth().val() != 0.0) {
                  QColor color(textStyle().frameColor());
                  if (!visible())
                        color = Qt::gray;
                  else if (selected())
                        color = MScore::selectColor[0];
                  QPen pen(color, textStyle().frameWidth().val() * spatium());
                  p->setPen(pen);
                  }
            else
                  p->setPen(Qt::NoPen);
            QColor bg(textStyle().backgroundColor());
            p->setBrush(bg.alpha() ? QBrush(bg) : Qt::NoBrush);
            if (textStyle().circle())
                  p->drawArc(frame, 0, 5760);
            else {
                  int r2 = textStyle().frameRound() * lrint((frame.width() / frame.height()));
                  if (r2 > 99)
                        r2 = 99;
                  p->drawRoundRect(frame, textStyle().frameRound(), r2);
                  }
            }
      p->setBrush(Qt::NoBrush);
      QColor color;
      if (selected())
            color = MScore::selectColor[0];
      else if (!visible())
            color = Qt::gray;
      else
            color = textStyle().foregroundColor();
      p->setPen(color);
      if (_editMode && _cursor.hasSelection()) {
            int r1 = _cursor.selectLine();
            int r2 = _cursor.line();
            int c1 = _cursor.selectColumn();
            int c2 = _cursor.column();

            if (r1 > r2) {
                  qSwap(r1, r2);
                  qSwap(c1, c2);
                  }
            else if (r1 == r2) {
                  if (c1 > c2)
                        qSwap(c1, c2);
                  }
            int row = 0;
            for (const TextBlock& t : _layout) {
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
                        br.translate(0.0, t.y());
                        drawSelection(p, br);
                        }
                  ++row;
                  }
            }
      else {
            for (const TextBlock& t : _layout)
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

QRectF Text::cursorRect() const
      {
      const TextBlock& tline = curLine();
      const TextFragment* fragment = tline.fragment(_cursor.column());

      qreal ascent;
      if (fragment) {
            QFont font = fragment->font(this);
            if (font.family() == score()->scoreFont()->font().family()) {
                  QFontMetricsF fm = QFontMetricsF(_textStyle.fontPx(spatium()));
                  ascent = fm.ascent();
                  }
            else {
                  QFontMetricsF fm = QFontMetrics(font);
                  ascent = fm.ascent();
                  }
            }
      else {
            QFontMetricsF fm = QFontMetricsF(_textStyle.fontPx(spatium()));
            ascent = fm.ascent();
            }

      ascent *= 0.7;
      qreal h = ascent;       // lineSpacing();
      qreal x = tline.xpos(_cursor.column(), this);
      qreal y = tline.y();
      y      -= ascent;
      return QRectF(x, y, 0.0, h);
      }

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

QColor Text::textColor() const
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
//   insert
//---------------------------------------------------------

void Text::insert(TextCursor* cursor, QChar c)
      {
      if (cursor->hasSelection())
            deleteSelectedText();
      if (cursor->line() >= _layout.size())
            _layout.append(TextBlock());
      if (c == QChar::LineFeed) {
            _layout[cursor->line()].setEol(true);
            cursor->setLine(cursor->line() + 1);
            cursor->setColumn(0);
            if (_layout.size() < cursor->line())
                  _layout.append(TextBlock());
            }
      else {
            _layout[cursor->line()].insert(cursor, QString(c));
            cursor->setColumn(cursor->column() + 1);
            }
      cursor->clearSelection();
      }

void Text::insert(TextCursor* cursor, SymId id)
      {
      if (cursor->hasSelection())
            deleteSelectedText();
      if (cursor->line() >= _layout.size())
            _layout.append(TextBlock());
      _layout[cursor->line()].insert(cursor, id);
      cursor->setColumn(cursor->column() + 1);
      cursor->clearSelection();
      }

//---------------------------------------------------------
//   parseStringProperty
//---------------------------------------------------------

static QString parseStringProperty(const QString& s)
      {
      QString rs;
      for (const QChar& c : s) {
            if (c == '"')
                  break;
            rs += c;
            }
      return rs;
      }

//---------------------------------------------------------
//   parseNumProperty
//---------------------------------------------------------

static qreal parseNumProperty(const QString& s)
      {
      return parseStringProperty(s).toDouble();
      }

//---------------------------------------------------------
//   createLayout
//    create layout from text
//---------------------------------------------------------

void Text::createLayout()
      {
      _layout.clear();
      TextCursor cursor;
      cursor.initFromStyle(textStyle());

      int state = 0;
      QString token;
      QString sym;
      bool symState = false;
      for (const QChar& c : _text) {
            if (state == 0) {
                  if (c == '<') {
                        state = 1;
                        token.clear();
                        }
                  else if (c == '&') {
                        state = 2;
                        token.clear();
                        }
                  else {
                        if (symState)
                              sym += c;
                        else
                              insert(&cursor, c);
                        }
                  }
            else if (state == 1) {
                  if (c == '>') {
                        state = 0;
                        if (token == "b")
                              cursor.format()->setBold(true);
                        else if (token == "/b")
                              cursor.format()->setBold(false);
                        else if (token == "i")
                              cursor.format()->setItalic(true);
                        else if (token == "/i")
                              cursor.format()->setItalic(false);
                        else if (token == "u")
                              cursor.format()->setUnderline(true);
                        else if (token == "/u")
                              cursor.format()->setUnderline(false);
                        else if (token == "sub")
                              cursor.format()->setValign(VerticalAlignment::AlignSubScript);
                        else if (token == "/sub")
                              cursor.format()->setValign(VerticalAlignment::AlignNormal);
                        else if (token == "sup")
                              cursor.format()->setValign(VerticalAlignment::AlignSuperScript);
                        else if (token == "/sup")
                              cursor.format()->setValign(VerticalAlignment::AlignNormal);
                        else if (token == "sym") {
                              symState = true;
                              sym.clear();
                              }
                        else if (token == "/sym") {
                              symState = false;
                              insert(&cursor, Sym::name2id(sym));
                              }
                        else if (token.startsWith("font ")) {
                              token = token.mid(5);
                              if (token.startsWith("size=\""))
                                    cursor.format()->setFontSize(parseNumProperty(token.mid(6)));
                              else if (token.startsWith("face=\""))
                                    cursor.format()->setFontFamily(parseStringProperty(token.mid(6)));
                              else
                                    qDebug("cannot parse html property <%s>", qPrintable(token));
                              }
                        }
                  else
                        token += c;
                  }
            else if (state == 2) {
                  if (c == ';') {
                        state = 0;
                        if (token == "lt")
                              insert(&cursor, '<');
                        else if (token == "gt")
                              insert(&cursor, '>');
                        else if (token == "amp")
                              insert(&cursor, '&');
                        else if (token == "quot")
                              insert(&cursor, '"');
                        else
                              insert(&cursor, Sym::name2id(token));
                        }
                  else
                        token += c;
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
      setPos(_textStyle.offset(spatium()));
      layout1();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Text::layout1()
      {
      if (!_editMode)
            createLayout();

      if (_layout.isEmpty())
            _layout.append(TextBlock());

      QRectF bb;
      qreal y = 0;
      for (int i = 0; i < _layout.size(); ++i) {
            TextBlock* t = &_layout[i];
            t->layout(this);
            const QRectF* r = &t->boundingRect();
            if (r->height() == 0)
                  r = &_layout[i-i].boundingRect();
            y += t->lineSpacing();
            t->setY(y);
            bb |= r->translated(0.0, y);
            }
      qreal yoff = 0;
      qreal h    = 0;
      if (parent()) {
#if 0
            if (parent()->type() == ElementType::SEGMENT) {
                  Segment* s = static_cast<Segment*>(parent());
                  System* system = s->measure()->system();
                  if (system) {
                        SysStaff* sstaff = system->staff(staffIdx());
                        yoff = sstaff->y();
                        }
                  }
#endif
            if (layoutToParentWidth()) {
                  if (parent()->type() == ElementType::HBOX || parent()->type() == ElementType::VBOX || parent()->type() == ElementType::TBOX) {
                        // consider inner margins of frame
                        Box* b = static_cast<Box*>(parent());
                        yoff = b->topMargin()  * MScore::DPMM;
                        h  = b->height() - yoff - b->bottomMargin()   * MScore::DPMM;
                        }
                  else
                        h  = parent()->height();
                  }
            }

      if (textStyle().align() & ALIGN_BOTTOM)
            yoff += h-bb.bottom();
      else if (textStyle().align() & ALIGN_VCENTER)
            yoff +=  (h - (bb.top() + bb.bottom())) * .5;
      else if (textStyle().align() & ALIGN_BASELINE)
            yoff += h * .5 - _layout.front().lineSpacing();
      else
            yoff += -bb.top();

      for (TextBlock& t : _layout)
            t.setY(t.y() + yoff);

      bb.translate(0.0, yoff);

      if (_editMode)
            bb |= cursorRect();
      setbbox(bb);
      if (textStyle().hasFrame())
            layoutFrame();
      }

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void Text::layoutFrame()
      {
      frame = bbox();
      if (textStyle().circle()) {
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
      qreal w = (textStyle().paddingWidth() + textStyle().frameWidth() * .5f).val() * _spatium;
      frame.adjust(-w, -w, w, w);
      w = textStyle().frameWidth().val() * _spatium;
      setbbox(frame.adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal Text::lineSpacing() const
      {
      return QFontMetricsF(textStyle().fontPx(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal Text::lineHeight() const
      {
      return QFontMetricsF(textStyle().fontPx(spatium())).height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal Text::baseLine() const
      {
      return QFontMetricsF(textStyle().fontPx(spatium())).ascent();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Text::startEdit(MuseScoreView*, const QPointF& pt)
      {
      setEditMode(true);
      _cursor.setLine(0);
      _cursor.setColumn(0);
      _cursor.clearSelection();
      if (_layout.isEmpty())
            layout();
      if (setCursor(pt))
            updateCursorFormat(&_cursor);
      else
            _cursor.initFromStyle(textStyle());
      undoPushProperty(P_ID::TEXT);
      }

//---------------------------------------------------------
//   XmlNesting
//---------------------------------------------------------

class XmlNesting : public QStack<QString> {
      QString* _s;

   public:
      XmlNesting(QString* s) { _s = s; }
      void pushToken(const QString& t) {
            *_s += "<";
            *_s += t;
            *_s += ">";
            push(t);
            }
      void pushB() { pushToken("b"); }
      void pushI() { pushToken("i"); }
      void pushU() { pushToken("u"); }

      QString popToken() {
            QString s = pop();
            *_s += "</";
            *_s += s;
            *_s += ">";
            return s;
            }
      void popToken(const char* t) {
            QStringList ps;
            for (;;) {
                  QString s = popToken();
                  if (s == t)
                        break;
                  ps += s;
                  }
            for (const QString& s : ps)
                  pushToken(s);
            }
      void popB() { popToken("b"); }
      void popI() { popToken("i"); }
      void popU() { popToken("u"); }
      };

//---------------------------------------------------------
//   genText
//---------------------------------------------------------

void Text::genText()
      {
      _text.clear();
      bool bold      = false;
      bool italic    = false;
      bool underline = false;

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (!f.format.bold() && textStyle().bold())
                        bold = true;
                  if (!f.format.italic() && textStyle().italic())
                        italic = true;
                  if (!f.format.underline() && textStyle().underline())
                        underline = true;
                  }
            }
      TextCursor cursor;
      cursor.initFromStyle(textStyle());
      XmlNesting xmlNesting(&_text);
      if (bold)
            xmlNesting.pushB();
      if (italic)
            xmlNesting.pushI();
      if (underline)
            xmlNesting.pushU();

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (f.text.isEmpty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                  const CharFormat& format = f.format;
                  if (format.type() == CharFormatType::TEXT) {
                        if (cursor.format()->bold() != format.bold()) {
                              if (format.bold())
                                    xmlNesting.pushB();
                              else
                                    xmlNesting.popB();
                              }
                        if (cursor.format()->italic() != format.italic()) {
                              if (format.italic())
                                    xmlNesting.pushI();
                              else
                                    xmlNesting.popI();
                              }
                        if (cursor.format()->underline() != format.underline()) {
                              if (format.underline())
                                    xmlNesting.pushU();
                              else
                                    xmlNesting.popU();
                              }

                        if (format.fontSize() != cursor.format()->fontSize())
                              _text += QString("<font size=\"%1\"/>").arg(format.fontSize());
                        if (format.fontFamily() != cursor.format()->fontFamily())
                              _text += QString("<font face=\"%1\"/>").arg(format.fontFamily());

                        VerticalAlignment va = format.valign();
                        VerticalAlignment cva = cursor.format()->valign();
                        if (cva != va) {
                              switch (va) {
                                    case VerticalAlignment::AlignNormal:
                                          xmlNesting.popToken(cva == VerticalAlignment::AlignSuperScript ? "sup" : "sub");
                                          break;
                                    case VerticalAlignment::AlignSuperScript:
                                          xmlNesting.pushToken("sup");
                                          break;
                                    case VerticalAlignment::AlignSubScript:
                                          xmlNesting.pushToken("sub");
                                          break;
                                    }
                              }
                        _text += Xml::xmlString(f.text);
                        cursor.setFormat(format);
                        }
                  else {
                        for (SymId id : f.ids)
                              _text += QString("<sym>%1</sym>").arg(Sym::id2name(id));
                        }
                  }
            if (block.eol())
                  _text += QChar::LineFeed;
            }
      while (!xmlNesting.isEmpty())
            xmlNesting.popToken();
      }

//---------------------------------------------------------
//   plainText
//    return plain text with symbols
//---------------------------------------------------------

QString Text::plainText(bool noSym) const
      {
      QString s;

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  const CharFormat& format = f.format;
                  if (format.type() == CharFormatType::TEXT) {
                        s += f.text;
                        }
                  else if (!noSym){
                        for (SymId id : f.ids)
                              s += QString("<sym>%1</sym>").arg(Sym::id2name(id));
                        }
                  }
            if (block.eol())
                  s += QChar::LineFeed;
            }
      return s;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
      {
      setEditMode(false);
      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));

      genText();
      if (links()) {
            foreach (Element* e, *links()) {
                  if (e != this)
                        e->undoChangeProperty(P_ID::TEXT, _text);
                  }
            }
      textChanged();
      }

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

const TextBlock& Text::curLine() const
      {
      return _layout[_cursor.line()];
      }

TextBlock& Text::curLine()
      {
      return _layout[_cursor.line()];
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Text::edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers modifiers, const QString& _s)
      {
      QRectF refresh(canvasBoundingRect());
      QString s = _s;
      QTextCursor::MoveMode mm = (modifiers & Qt::ShiftModifier)
         ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
      bool ctrlPressed = modifiers & Qt::ControlModifier;

      switch (key) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                  if (_cursor.hasSelection())
                        deleteSelectedText();

                  CharFormat* charFmt = _cursor.format();         // take current format
                  _layout.insert(_cursor.line() + 1, curLine().split(_cursor.column()));
                  _layout[_cursor.line()].setEol(true);
                  _cursor.setLine(_cursor.line() + 1);
                  _cursor.setColumn(0);
                  _cursor.setFormat(*charFmt);                    // restore orig. format at new line
                  s.clear();
                  _cursor.clearSelection();
                  break;
            }

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
                  if (!movePosition(ctrlPressed ? QTextCursor::StartOfLine : QTextCursor::Left, mm) && type() == ElementType::LYRICS)
                        return false;
                  s.clear();
                  break;

            case Qt::Key_Right:
                  if (!movePosition(ctrlPressed ? QTextCursor::EndOfLine : QTextCursor::Right, mm) && type() == ElementType::LYRICS)
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
                  modifiers = 0;
                  break;

            case Qt::Key_Minus:
                  s = "-";
                  modifiers = 0;
                  break;

            case Qt::Key_Underscore:
                  s = "_";
                  modifiers = 0;
                  break;

            case Qt::Key_A:
                  if (modifiers & Qt::ControlModifier) {
                        selectAll();
                        s.clear();
                  }
                  break;
            default:
                  break;
            }
      if (modifiers & Qt::ControlModifier) {
            switch (key) {
                  case Qt::Key_F:
                        insertSym(SymId::dynamicForte);
                        s.clear();
                        break;
                  case Qt::Key_M:
                        insertSym(SymId::dynamicNiente);
                        s.clear();
                        break;
                  case Qt::Key_P:
                        insertSym(SymId::dynamicPiano);
                        s.clear();
                        break;
                  case Qt::Key_Z:
                        insertSym(SymId::dynamicZ);
                        s.clear();
                        break;
                  case Qt::Key_S:
                        insertSym(SymId::dynamicSforzando);
                        s.clear();
                        break;
                  case Qt::Key_R:
                        s.clear();
                        insertSym(SymId::dynamicRinforzando);
                        break;
                  }
            }
      if (!s.isEmpty())
            insertText(s);
      layout1();
      if (parent() && parent()->type() == ElementType::TBOX) {
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
//   selectAll
//---------------------------------------------------------

void Text::selectAll()
      {
      _cursor.setSelectLine(0);
      _cursor.setSelectColumn(0);
      _cursor.setLine(_layout.size() - 1);
      _cursor.setColumn(curLine().columns());
      }

//---------------------------------------------------------
//   deletePreviousChar
//---------------------------------------------------------

bool Text::deletePreviousChar()
      {
      if (_cursor.column() == 0) {
            if (_cursor.line() == 0)
                  return false;
            const TextBlock& l1 = _layout.at(_cursor.line());
            TextBlock& l2       = _layout[_cursor.line() - 1];
            _cursor.setColumn(l2.columns());
            for (const TextFragment& f : l1.fragments())
                  l2.fragments().append(f);
            _layout.removeAt(_cursor.line());
            _cursor.setLine(_cursor.line()-1);
            }
      else {
            _cursor.setColumn(_cursor.column()-1);
            curLine().remove(_cursor.column());
            }
      _cursor.clearSelection();
      return true;
      }

//---------------------------------------------------------
//   deleteChar
//---------------------------------------------------------

bool Text::deleteChar()
      {
      if (_cursor.column() >= curLine().columns()) {
            if (_cursor.line() + 1 < _layout.size()) {
                  TextBlock& l1       = _layout[_cursor.line()];
                  const TextBlock& l2 = _layout[_cursor.line() + 1];
                  for (const TextFragment& f : l2.fragments())
                        l1.fragments().append(f);
                  _layout.removeAt(_cursor.line() + 1);
                  return true;
                  }
            return false;
            }
      curLine().remove(_cursor.column());
      _cursor.clearSelection();
      return true;
      }

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

bool Text::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode, int count)
      {
      for (int i=0; i < count; i++) {
            switch(op) {
                  case QTextCursor::Left:
                        if (_cursor.column() == 0) {
                              if (_cursor.line() == 0)
                                    return false;
                              _cursor.setLine(_cursor.line()-1);
                              _cursor.setColumn(curLine().columns());
                              }
                        else
                              _cursor.setColumn(_cursor.column()-1);
                        break;

                  case QTextCursor::Right:
                        if (_cursor.column() >= curLine().columns()) {
                              if (_cursor.line() >= _layout.size()-1)
                                    return false;
                              _cursor.setLine(_cursor.line()+1);
                              _cursor.setColumn(0);
                              }
                        else
                              _cursor.setColumn(_cursor.column()+1);
                        break;

                  case QTextCursor::Up:
                        if (_cursor.line() == 0)
                              return false;
                        _cursor.setLine(_cursor.line()-1);
                        if (_cursor.column() > curLine().columns())
                              _cursor.setColumn(curLine().columns());
                        break;

                  case QTextCursor::Down:
                        if (_cursor.line() >= _layout.size()-1)
                              return false;
                        _cursor.setLine(_cursor.line()+1);
                        if (_cursor.column() > curLine().columns())
                              _cursor.setColumn(curLine().columns());
                        break;

                  case QTextCursor::Start:
                        _cursor.setLine(0);
                        _cursor.setColumn(0);
                        break;

                  case QTextCursor::End:
                        _cursor.setLine(_layout.size() - 1);
                        _cursor.setColumn(curLine().columns());
                        break;

                  case QTextCursor::StartOfLine:
                        _cursor.setColumn(0);
                        break;

                  case QTextCursor::EndOfLine:
                        _cursor.setColumn(curLine().columns());
                        break;
                  default:
                        qDebug("Text::movePosition: not implemented");
                        return false;
                  }
            }
      if (mode == QTextCursor::MoveAnchor)
            _cursor.clearSelection();
      updateCursorFormat(&_cursor);
      return true;
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

bool Text::setCursor(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;
      _cursor.setLine(0);
      for (int row = 0; row < _layout.size(); ++row) {
            const TextBlock& l = _layout.at(row);
            if (l.y() > pt.y()) {
                  _cursor.setLine(row);
                  break;
                  }
            }
      _cursor.setColumn(curLine().column(pt.x(), this));
      score()->setUpdateAll(true);
      if (mode == QTextCursor::MoveAnchor)
            _cursor.clearSelection();
      if (_cursor.hasSelection())
            QApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
      return true;
      }

//---------------------------------------------------------
//   selectedText
//    return current selection
//---------------------------------------------------------

QString Text::selectedText() const
      {
      QString s;
      int r1 = _cursor.selectLine();
      int r2 = _cursor.line();
      int c1 = _cursor.selectColumn();
      int c2 = _cursor.column();

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
            const TextBlock& t = _layout.at(row);
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        s += t.text(c1, c2 - c1);
                  else if (row == r1)
                        s += t.text(c1, -1);
                  else if (row == r2)
                        s += t.text(0, c2);
                  else
                        s += t.text(0, -1);
                  }
            if (row != rows -1)
                  s += "\n";
            }
      return s;
      }

//---------------------------------------------------------
//   deleteSelectedText
//---------------------------------------------------------

void Text::deleteSelectedText()
      {
      int r1 = _cursor.selectLine();
      int r2 = _cursor.line();
      int c1 = _cursor.selectColumn();
      int c2 = _cursor.column();

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _layout.size();
      QList<TextBlock> toDelete;
      for (int row = 0; row < rows; ++row) {
            TextBlock& t = _layout[row];
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        t.remove(c1, c2 - c1);
                  else if (row == r1)
                        t.remove(c1, t.columns() - c1);
                  else if (row == r2)
                        t.remove(0, c2);
                  else {
                        toDelete.append(t);
                        }
                  }
            }
      if (r1 != r2) {
            TextBlock& l1 = _layout[r1];
            const TextBlock& l2 = _layout[r2];
            for (const TextFragment& f : l2.fragments())
                  l1.fragments().append(f);

            _layout.removeAt(r2);
            QMutableListIterator<TextBlock> i(_layout);
            while (i.hasNext()) {
                  if (toDelete.contains(i.next()))
                        i.remove();
                  }
            }
      _cursor.setLine(r1);
      _cursor.setColumn(c1);
      _cursor.clearSelection();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperties(xml, true, true);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml, bool writeText, bool writeStyle) const
      {
      Element::writeProperties(xml);
      if (writeStyle) {
            if (getProperty(P_ID::TEXT_STYLE_TYPE)  != propertyDefault(P_ID::TEXT_STYLE_TYPE))
                  xml.tag("style", textStyle().name());
            _textStyle.writeProperties(xml, score()->textStyle(_styleIndex));
            }
      if (writeText)
            xml.writeXml("text", text());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "style") {
            QString val(e.readElementText());
            int st;
            bool ok;
            int i = val.toInt(&ok);
            if (ok) {
                  // obsolete old text styles
                  switch (i) {
                        case 2:  i = TEXT_STYLE_TITLE;     break;
                        case 3:  i = TEXT_STYLE_SUBTITLE;  break;
                        case 4:  i = TEXT_STYLE_COMPOSER;  break;
                        case 5:  i = TEXT_STYLE_POET;      break;
                        case 6:  i = TEXT_STYLE_LYRIC1;    break;
                        case 7:  i = TEXT_STYLE_LYRIC2;    break;
                        case 8:  i = TEXT_STYLE_FINGERING; break;
                        case 9:  i = TEXT_STYLE_INSTRUMENT_LONG;    break;
                        case 10: i = TEXT_STYLE_INSTRUMENT_SHORT;   break;
                        case 11: i = TEXT_STYLE_INSTRUMENT_EXCERPT; break;

                        case 12: i = TEXT_STYLE_DYNAMICS;  break;
                        case 13: i = TEXT_STYLE_TECHNIQUE;   break;
                        case 14: i = TEXT_STYLE_TEMPO;     break;
                        case 15: i = TEXT_STYLE_METRONOME; break;
                        case 16: i = TEXT_STYLE_FOOTER;    break;  // TEXT_STYLE_COPYRIGHT
                        case 17: i = TEXT_STYLE_MEASURE_NUMBER; break;
                        case 18: i = TEXT_STYLE_FOOTER; break;    // TEXT_STYLE_PAGE_NUMBER_ODD
                        case 19: i = TEXT_STYLE_FOOTER; break;    // TEXT_STYLE_PAGE_NUMBER_EVEN
                        case 20: i = TEXT_STYLE_TRANSLATOR; break;
                        case 21: i = TEXT_STYLE_TUPLET;     break;

                        case 22: i = TEXT_STYLE_SYSTEM;         break;
                        case 23: i = TEXT_STYLE_STAFF;          break;
                        case 24: i = TEXT_STYLE_HARMONY;        break;
                        case 25: i = TEXT_STYLE_REHEARSAL_MARK; break;
                        case 26: i = TEXT_STYLE_REPEAT;         break;
                        case 27: i = TEXT_STYLE_VOLTA;          break;
                        case 28: i = TEXT_STYLE_FRAME;          break;
                        case 29: i = TEXT_STYLE_TEXTLINE;       break;
                        case 30: i = TEXT_STYLE_GLISSANDO;      break;
                        case 31: i = TEXT_STYLE_STRING_NUMBER;  break;

                        case 32: i = TEXT_STYLE_OTTAVA;  break;
                        case 33: i = TEXT_STYLE_BENCH;   break;
                        case 34: i = TEXT_STYLE_HEADER;  break;
                        case 35: i = TEXT_STYLE_FOOTER;  break;
                        case 0:
                        default:
                              qDebug("Text:readProperties: style %d<%s> invalid", i, qPrintable(val));
                              i = TEXT_STYLE_DEFAULT;
                              break;
                        }
                  st = i;
                  }
            else
                  st = score()->style()->textStyleType(val);
            setTextStyleType(st);
            }
      else if (tag == "styleName")          // obsolete, unstyled text
            e.skipCurrentElement(); // _styleName = val;
      else if (tag == "data")                  // obsolete
            e.readElementText();
      else if (tag == "html")
            setPlainText(QTextDocumentFragment::fromHtml(e.readXml()).toPlainText());
      else if (tag == "text")
            _text = e.readXml();
      else if (tag == "html-data")
            setText(convertFromHtml(e.readXml()));
      else if (tag == "subtype")          // obsolete
            e.skipCurrentElement();
      else if (tag == "frameWidth") {           // obsolete
            qreal spMM = spatium() / MScore::DPMM;
            textStyle().setFrameWidth(Spatium(e.readDouble() / spMM));
            }
      else if (tag == "paddingWidth") {          // obsolete
            qreal spMM = spatium() / MScore::DPMM;
            textStyle().setPaddingWidth(Spatium(e.readDouble() / spMM));
            }
      else if (_textStyle.readProperties(e))
            ;
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void Text::textStyleChanged()
      {
      setTextStyle(score()->textStyle(_styleIndex));
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void Text::setTextStyle(const TextStyle& st)
      {
      _textStyle = st;
      if (editMode()) {
            setText(plainText());
            createLayout();
            }
      }

//---------------------------------------------------------
//   setTextStyleType
//---------------------------------------------------------

void Text::setTextStyleType(int st)
      {
      _styleIndex = st;
      setTextStyle(score()->textStyle(st));
      }

//---------------------------------------------------------
//   insertText
//    insert text at cursor position and move cursor
//---------------------------------------------------------

void Text::insertText(const QString& s)
      {
      if (s.isEmpty())
            return;
      if (_cursor.hasSelection())
            deleteSelectedText();
      if (_cursor.format()->type() == CharFormatType::SYMBOL) {
            _cursor.format()->setFontFamily(textStyle().family());
            _cursor.format()->setType(CharFormatType::TEXT);
            }
      curLine().insert(&_cursor, s);
      _cursor.setColumn(_cursor.column() + s.size());
      _cursor.clearSelection();
      }

//---------------------------------------------------------
//   insertSym
//---------------------------------------------------------

void Text::insertSym(SymId id)
      {
      if (_cursor.hasSelection())
            deleteSelectedText();
      curLine().insert(&_cursor, id);
      _cursor.setColumn(_cursor.column() + 1);
      _cursor.clearSelection();
      layout();
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == ElementType::HBOX || parent()->type() == ElementType::VBOX || parent()->type() == ElementType::TBOX)) {
            QRectF r = parent()->abbox();
            Box* box = static_cast<Box*>(parent());
            qreal x = r.x() + box->leftMargin() * MScore::DPMM;
            qreal y = r.y() + box->topMargin() * MScore::DPMM;
            qreal h = r.height() - (box->topMargin() + box->bottomMargin()) * MScore::DPMM;
            qreal w = r.width()  - (box->leftMargin() + box->rightMargin()) * MScore::DPMM;

            // QSizeF ps = _doc->pageSize();
            // return QRectF(x, y, ps.width(), ps.height());

            return QRectF(x, y, w, h);
            }
      else
            return abbox();
      }

//---------------------------------------------------------
//   dragTo
//---------------------------------------------------------

void Text::dragTo(const QPointF& p)
      {
      setCursor(p, QTextCursor::KeepAnchor);
      score()->setUpdateAll();
      score()->end();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Text::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::TEXT_STYLE:
                  return QVariant::fromValue(_textStyle);
            case P_ID::TEXT_STYLE_TYPE:
                  return QVariant(_styleIndex);
            case P_ID::TEXT:
                  return text();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Text::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      bool rv = true;
      switch (propertyId) {
            case P_ID::TEXT_STYLE:
                  setTextStyle(v.value<TextStyle>());
                  break;
            case P_ID::TEXT_STYLE_TYPE:
                  setTextStyleType(v.toInt());
                  setGenerated(false);
                  break;
            case P_ID::TEXT:
                  setText(v.toString());
                  break;
            default:
                  rv = Element::setProperty(propertyId, v);
                  break;
            }
      score()->setLayoutAll(true);
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Text::propertyDefault(P_ID id) const
      {
      int idx;
      switch (type()) {
            case ElementType::DYNAMIC:           idx = TEXT_STYLE_DYNAMICS; break;
            case ElementType::FIGURED_BASS:      idx = TEXT_STYLE_FIGURED_BASS; break;
            case ElementType::FINGERING:         idx = TEXT_STYLE_FINGERING; break;
            case ElementType::HARMONY:           idx = TEXT_STYLE_HARMONY; break;
            case ElementType::INSTRUMENT_CHANGE: idx = TEXT_STYLE_INSTRUMENT_CHANGE; break;
            // case ElementType::INSTRUMENT_NAME: would need to differentiate long & short
            // probably best handle this with another override
            case ElementType::JUMP:              idx = TEXT_STYLE_REPEAT; break;
            case ElementType::LYRICS:            idx = TEXT_STYLE_LYRIC1; break;
            case ElementType::MARKER:            idx = TEXT_STYLE_REPEAT; break;
            case ElementType::REHEARSAL_MARK:    idx = TEXT_STYLE_REHEARSAL_MARK; break;
            case ElementType::STAFF_TEXT:        idx = TEXT_STYLE_STAFF; break;
            case ElementType::TEMPO_TEXT:        idx = TEXT_STYLE_TEMPO; break;
            default:
                  // if we cannot determine type, give up
                  return Element::propertyDefault(id);
            }
      switch (id) {
            case P_ID::TEXT_STYLE_TYPE:
                  return idx;
            case P_ID::TEXT_STYLE:
                  return score()->textStyle(idx).name();
            case P_ID::TEXT:
                  return QString("");
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void Text::paste()
      {
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
      if (MScore::debugMode)
            qDebug("Text::paste() <%s>", qPrintable(txt));
      insertText(txt);
      layoutEdit();
      bool lo = type() == ElementType::INSTRUMENT_NAME;
      score()->setLayoutAll(lo);
      score()->setUpdateAll();
      score()->end();
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool Text::mousePress(const QPointF& p, QMouseEvent* ev)
      {
      bool shift = ev->modifiers() & Qt::ShiftModifier;
      if (!setCursor(p, shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor))
            return false;
      if (ev->button() == Qt::MidButton)
            paste();
      return true;
      }

//---------------------------------------------------------
//   layoutEdit
//---------------------------------------------------------

void Text::layoutEdit()
      {
      layout();
      if (parent() && parent()->type() == ElementType::TBOX) {
            TBox* tbox = static_cast<TBox*>(parent());
            tbox->layout();
            System* system = tbox->system();
            system->setHeight(tbox->height());
            score()->doLayoutPages();
            score()->setUpdateAll(true);
            }
      else {
            static const qreal w = 2.0; // 8.0 / view->matrix().m11();
            score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Text::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      ElementType type = e->type();
      return type == ElementType::SYMBOL;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Text::drop(const DropData& data)
      {
      Element* e = data.element;

      switch(e->type()) {
            case ElementType::SYMBOL:
                  {
                  SymId id = static_cast<Symbol*>(e)->sym();
                  delete e;

                  if (_editMode) {
                        insert(&_cursor, id);
                        layout1();
                        static const qreal w = 2.0; // 8.0 / view->matrix().m11();
                        score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
                        }
                  else {
                        startEdit(data.view, data.pos);
                        curLine().insert(&_cursor, id);
                        endEdit();
                        }
                  }
                  return 0;

            default:
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   setPlainText
//---------------------------------------------------------

void Text::setPlainText(const QString& s)
      {
      setText(s.toHtmlEscaped());
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QString& s)
      {
      _text = s;
      textChanged();
      }

//---------------------------------------------------------
//   changeSelectionFormat
//---------------------------------------------------------

void Text::changeSelectionFormat(FormatId id, QVariant val)
      {
      if (!_cursor.hasSelection())
            return;
      int r1 = _cursor.selectLine();
      int r2 = _cursor.line();
      int c1 = _cursor.selectColumn();
      int c2 = _cursor.column();

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _layout.size();
      QList<TextBlock> toDelete;
      for (int row = 0; row < rows; ++row) {
            TextBlock& t = _layout[row];
            if (row < r1)
                  continue;
            if (row > r2)
                  break;
            if (row == r1 && r1 == r2)
                  t.changeFormat(id, val, c1, c2 - c1);
            else if (row == r1)
                  t.changeFormat(id, val, c1, t.columns() - c1);
            else if (row == r2)
                  t.changeFormat(id, val, 0, c2);
            else
                  t.changeFormat(id, val, 0, t.columns());
            }
      layout1();
      score()->addRefresh(canvasBoundingRect());
      }

//---------------------------------------------------------
//   setFormat
//---------------------------------------------------------

void Text::setFormat(FormatId id, QVariant val)
      {
      changeSelectionFormat(id, val);
      _cursor.format()->setFormat(id, val);
      }

//---------------------------------------------------------
//   restyle
//    restyle from old style type s
//---------------------------------------------------------

void Text::restyle(int oldType)
      {
      const TextStyle& os = score()->textStyle(oldType);
      const TextStyle& ns = score()->textStyle(textStyleType());
      _textStyle.restyle(os, ns);
      }

//---------------------------------------------------------
//   convertFromHtml
//---------------------------------------------------------

QString Text::convertFromHtml(const QString& ss) const
      {
      QTextDocument doc;
      doc.setHtml(ss);

      QString s;
      qreal size = textStyle().size();
      QString family = textStyle().family();
      for (auto b = doc.firstBlock(); b.isValid() ; b = b.next()) {
            if (!s.isEmpty())
                  s += "\n";
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                  QTextFragment f = it.fragment();
                  if (f.isValid()) {
                        QTextCharFormat tf = f.charFormat();
                        QFont font = tf.font();
                        if (fabs(size - font.pointSizeF()) > 0.1) {
                              size = font.pointSizeF();
                              s += QString("<font size=\"%1\"/>").arg(size);
                              }
                        if (family != font.family()) {
                              family = font.family();
                              s += QString("<font face=\"%1\"/>").arg(family);
                              }
                        if (font.bold())
                              s += "<b>";
                        if (font.italic())
                              s += "<i>";
                        if (font.underline())
                              s += "<u>";
                        s += f.text().toHtmlEscaped();
                        if (font.underline())
                              s += "</u>";
                        if (font.italic())
                              s += "</i>";
                        if (font.bold())
                              s += "</b>";
                        }
                  }
            }

      if (score()->mscVersion() <= 114) {
            s.replace(QChar(0xe10e), QString("<sym>accidentalNatural</sym>"));    //natural
            s.replace(QChar(0xe10c), QString("<sym>accidentalSharp</sym>"));    // sharp
            s.replace(QChar(0xe10d), QString("<sym>accidentalFlat</sym>"));    // flat
            s.replace(QChar(0xe104), QString("<sym>noteHalfUp</sym>")),    // note2_Sym
            s.replace(QChar(0xe105), QString("<sym>noteQuarterUp</sym>"));    // note4_Sym
            s.replace(QChar(0xe106), QString("<sym>note8thUp</sym>"));    // note8_Sym
            s.replace(QChar(0xe107), QString("<sym>note16thUp</sym>"));    // note16_Sym
            s.replace(QChar(0xe108), QString("<sym>note32ndUp</sym>"));    // note32_Sym
            s.replace(QChar(0xe109), QString("<sym>note64thUp</sym>"));    // note64_Sym
            s.replace(QChar(0xe10a), QString("<sym>textAugmentationDot</sym>"));    // dot
            s.replace(QChar(0xe10b), QString("<sym>textAugmentationDot</sym> <sym>textAugmentationDot</sym>"));    // dotdot
            s.replace(QChar(0xe167), QString("<sym>segno</sym>"));    // segno
            s.replace(QChar(0xe168), QString("<sym>coda</sym>"));    // coda
            s.replace(QChar(0xe169), QString("<sym>codaSquare</sym>"));    // varcoda
            }
      return s;
      }

}

