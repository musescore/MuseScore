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
#include "jump.h"
#include "marker.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "box.h"
#include "page.h"
#include "textframe.h"
#include "sym.h"
#include "xml.h"
#include "undo.h"
#include "mscore.h"

namespace Ms {


static const qreal subScriptSize     = 0.6;
static const qreal subScriptOffset   = 0.5;       // of x-height
static const qreal superScriptOffset = -.9;      // of x-height

//static const qreal tempotextOffset = 0.4; // of x-height // 80% of 50% = 2 spatiums

//---------------------------------------------------------
//   TextEditData
//---------------------------------------------------------

struct TextEditData : public ElementEditData {
      TextCursor* cursor;

      TextEditData()  {
            cursor = 0;
            }
      ~TextEditData() {
            printf("====destructor TextEditData\n");
            }
      };

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
         || cf.preedit() != preedit()
         || cf.valign() != valign()
         || cf.fontSize() != fontSize()
         )
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
//   init
//---------------------------------------------------------

void TextCursor::init()
      {
      _format.setFontFamily(_text->family());
      _format.setFontSize(_text->size());
      _format.setBold(_text->bold());
      _format.setItalic(_text->italic());
      _format.setUnderline(_text->underline());
      _format.setPreedit(false);
      _format.setValign(VerticalAlignment::AlignNormal);
      }

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int TextCursor::columns() const
      {
      return _text->textBlock(_line).columns();
      }

//---------------------------------------------------------
//   currentCharacter
//---------------------------------------------------------

QChar TextCursor::currentCharacter() const
      {
      const TextBlock& t = _text->_layout[line()];
      QString s = t.text(column(), column());
      if (s.isEmpty())
            return QChar();
      return s[0];
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
      format.setType(CharFormatType::TEXT);
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
      QFont f(font(t));
      f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
      p->setFont(f);
      p->drawText(pos, text);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextFragment::font(const Text* t) const
      {
      QFont font;

      qreal m = format.fontSize();

      if (t->sizeIsSpatiumDependent())
            m *= t->spatium() / SPATIUM20;
      if (format.valign() != VerticalAlignment::AlignNormal)
            m *= subScriptSize;

      font.setUnderline(format.underline() || format.preedit());
      if (format.type() == CharFormatType::TEXT) {
            font.setFamily(format.fontFamily());
            font.setBold(format.bold());
            font.setItalic(format.italic());
            }
      else {
            text.clear();
            ScoreFont* sf = ScoreFont::fallbackFont();
            for (SymId id : ids)
                  text.append(sf->toString(id));
            QString sfn = t->score()->styleSt(StyleIdx::MusicalTextFont);
            font.setFamily(sfn);
            font.setWeight(QFont::Normal);      // if not set we get system default
            //font.setStyleStrategy(QFont::NoFontMerging);
            font.setHintingPreference(QFont::PreferVerticalHinting);
            }

      Q_ASSERT(m > 0.0);

      font.setPointSizeF(m);
      return font;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextBlock::draw(QPainter* p, const Text* t) const
      {
      p->translate(0.0, _y);
      for (const TextFragment& f : _fragments)
            f.draw(p, t);
      p->translate(0.0, -_y);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBlock::layout(Text* t)
      {
      _bbox        = QRectF();
      qreal x      = 0.0;
      _lineSpacing = 0.0;
      qreal lm     = 0.0;

      qreal layoutWidth = 0;
      Element* e = t->parent();
      if (e && t->layoutToParentWidth()) {
            layoutWidth = e->width();
            switch(e->type()) {
                  case ElementType::HBOX:
                  case ElementType::VBOX:
                  case ElementType::TBOX: {
                        Box* b = static_cast<Box*>(e);
                        layoutWidth -= ((b->leftMargin() + b->rightMargin()) * DPMM);
                        lm = b->leftMargin() * DPMM;
                        }
                        break;
                  case ElementType::PAGE: {
                        Page* p = static_cast<Page*>(e);
                        layoutWidth -= (p->lm() + p->rm());
                        lm = p->lm();
                        }
                        break;
                  case ElementType::MEASURE: {
                        Measure* m = static_cast<Measure*>(e);
                        layoutWidth = m->bbox().width();
                        }
                        break;
                  default:
                        break;
                  }
            }
      if (_fragments.empty()) {
            QFontMetricsF fm = t->fontMetrics();
            _bbox.setRect(0.0, -fm.ascent(), 1.0, fm.descent());
            _lineSpacing = fm.lineSpacing();
            }
      else {
            for (TextFragment& f : _fragments) {
                  f.pos.setX(x);
                  QFontMetricsF fm(f.font(t), MScore::paintDevice());
                  if (f.format.valign() != VerticalAlignment::AlignNormal) {
                        qreal voffset = fm.xHeight() / subScriptSize;   // use original height
                        if (f.format.valign() == VerticalAlignment::AlignSubScript)
                              voffset *= subScriptOffset;
                        else
                              voffset *= superScriptOffset;
                        f.pos.setY(voffset);
                        }
                  else
                        f.pos.setY(0.0);
                  qreal w  = fm.width(f.text);
                  _bbox   |= fm.tightBoundingRect(f.text).translated(f.pos);
                  x += w;
                  // _lineSpacing = (_lineSpacing == 0 || fm.lineSpacing() == 0) ? qMax(_lineSpacing, fm.lineSpacing()) : qMin(_lineSpacing, fm.lineSpacing());
                  _lineSpacing = qMax(_lineSpacing, fm.lineSpacing());
                  }
            }
      qreal rx;
      if (t->align() & Align::RIGHT)
            rx = layoutWidth-_bbox.right();
      else if (t->align() & Align::HCENTER)
            rx = (layoutWidth - (_bbox.left() + _bbox.right())) * .5;
      else  // Align::LEFT
            rx = -_bbox.left();
      rx += lm;
      for (TextFragment& f : _fragments)
            f.pos.rx() += rx;
      _bbox.translate(rx, 0.0);
      }

//---------------------------------------------------------
//   xpos
//---------------------------------------------------------

qreal TextBlock::xpos(int column, const Text* t) const
      {
      int col = 0;
      for (const TextFragment& f : _fragments) {
            if (column == col)
                  return f.pos.x();
            QFontMetricsF fm(f.font(t), MScore::paintDevice());
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
      if (_fragments.empty())
            return 0;
      int col = 0;
      auto f = _fragments.begin();
      for (; f != _fragments.end(); ++f) {
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
      for (const TextFragment& f : _fragments) {
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
      for (const TextFragment& f : _fragments) {
            int idx = 0;
            if (x <= f.pos.x())
                  return col;
            qreal px = 0.0;
            for (const QChar& c : f.text) {
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  QFontMetricsF fm(f.font(t), MScore::paintDevice());
                  qreal xo = fm.width(f.text.left(idx));
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
      int rcol, ridx;
      auto i = fragment(cursor->column(), &rcol, &ridx);
      if (i != _fragments.end()) {
            if (i->format.type() == CharFormatType::TEXT) {
                  if (!(i->format == *cursor->format())) {
                        if (rcol == 0)
                              _fragments.insert(i, TextFragment(cursor, s));
                        else {
                              TextFragment f2 = i->split(rcol);
                              i = _fragments.insert(i+1, TextFragment(cursor, s));
                              _fragments.insert(i+1, f2);
                              }
                        }
                  else
                        i->text.insert(ridx, s);
                  }
            else {
                  if (rcol == 0) {
                        if (i != _fragments.begin() && (i-1)->format == *cursor->format())
                              (i-1)->text.append(s);
                        else
                              _fragments.insert(i, TextFragment(cursor, s));
                        }
                  else {
                        TextFragment f2 = i->split(rcol);
                        i = _fragments.insert(i+1, TextFragment(cursor, s));
                        f2.format = *cursor->format();
                        f2.format.setType(CharFormatType::SYMBOL);
                        _fragments.insert(i+1, f2);
                        }
                  }
            }
      else {
            if (!_fragments.empty() && _fragments.back().format.type() == CharFormatType::TEXT && _fragments.back().format == *cursor->format())
                  _fragments.back().text.append(s);
            else
                  _fragments.append(TextFragment(cursor, s));
            }
      }

void TextBlock::insert(TextCursor* cursor, SymId id)
      {
      int rcol, ridx;
      auto i = fragment(cursor->column(), &rcol, &ridx);
      if (i != _fragments.end()) {
            if (i->format.type() == CharFormatType::SYMBOL) {
                  if (!(i->format == *cursor->format())) {
                        if (rcol == 0)
                              _fragments.insert(i, TextFragment(cursor, id));
                        else {
                              TextFragment f2 = i->split(rcol);
                              i = _fragments.insert(i+1, TextFragment(cursor, id));
                              _fragments.insert(i+1, f2);
                              }
                        }
                  else
                        i->ids.insert(rcol, id);
                  }
            else if (i->format.type() == CharFormatType::TEXT) {
                  if (rcol == 0) {
                        if (i != _fragments.begin() && (i-1)->format == *cursor->format())
                              (i-1)->ids.append(id);
                        else
                              _fragments.insert(i, TextFragment(cursor, id));
                        }
                  else {
                        TextFragment f2 = i->split(rcol);
                        i = _fragments.insert(i+1, TextFragment(cursor, id));
                        _fragments.insert(i+1, f2);
                        }
                  }
            }
      else {
            if (!_fragments.empty() && _fragments.back().format.type() == CharFormatType::SYMBOL)
                  _fragments.back().ids.append(id);
            else
                  _fragments.append(TextFragment(cursor, id));
            }
      }

//---------------------------------------------------------
//   fragment
//    inputs:
//      column is the column relative to the start of the TextBlock.
//    outputs:
//      rcol will be the column relative to the start of the TextFragment that the input column is in.
//      ridx will be the QChar index into TextFragment's text QString relative to the start of that TextFragment.
//
//---------------------------------------------------------

QList<TextFragment>::iterator TextBlock::fragment(int column, int* rcol, int* ridx)
      {
      int col = 0;
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
            *rcol = 0;
            *ridx = 0;
            for (const QChar& c : i->text) {
                  if (col == column)
                        return i;
                  ++*ridx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++*rcol;
                  }
            }
      return _fragments.end();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

QString TextBlock::remove(int column)
      {
      int col = 0;
      QString s;
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
            int idx  = 0;
            int rcol = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (i->format.type() == CharFormatType::SYMBOL) {
                              i->ids.removeAt(idx);
                              if (i->ids.empty())
                                    _fragments.erase(i);
                              }
                        else {
                              if (c.isSurrogate()) {
                                    s = i->text.mid(idx, 2);
                                    i->text.remove(idx, 2);
                                    }
                              else {
                                    s = i->text.mid(idx, 1);
                                    i->text.remove(idx, 1);
                                    }
                              if (i->text.isEmpty())
                                    _fragments.erase(i);
                              }
                        simplify();
                        return s;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  ++rcol;
                  }
            }
      return s;
//      qDebug("TextBlock::remove: column %d not found", column);
      }

//---------------------------------------------------------
//   simplify
//---------------------------------------------------------

void TextBlock::simplify()
      {
      if (_fragments.size() < 2)
            return;
      auto i = _fragments.begin();
      TextFragment* f = &*i;
      ++i;
      for (; i != _fragments.end(); ++i) {
            while (i != _fragments.end() && (i->format == f->format)) {
                  if (f->format.type() == CharFormatType::SYMBOL)
                        f->ids.append(i->ids);
                  else
                        f->text.append(i->text);
                  i = _fragments.erase(i);
                  }
            if (i == _fragments.end())
                  break;
            f = &*i;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

QString TextBlock::remove(int start, int n)
      {
      if (n == 0)
            return QString();
      int col = 0;
      QString s;
      for (auto i = _fragments.begin(); i != _fragments.end();) {
            int rcol = 0;
            bool inc = true;
            for( int idx = 0; idx < i->text.length(); ) {
                  QChar c = i->text[idx];
                  if (col == start) {
                        if (c.isHighSurrogate()) {
                              s += c;
                              i->text.remove(idx, 1);
                              c = i->text[idx];
                              }
                        s += c;
                        i->text.remove(idx, 1);
                        if (i->format.type() == CharFormatType::SYMBOL)
                              i->ids.removeAt(idx);
                        if (i->text.isEmpty() && (_fragments.size() > 1)) {
                              i = _fragments.erase(i);
                              inc = false;
                              }
                        --n;
                        if (n == 0)
                              return s;
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
      return s;
      }

//---------------------------------------------------------
//   changeFormat
//---------------------------------------------------------

void TextBlock::changeFormat(FormatId id, QVariant data, int start, int n)
      {
      int col = 0;
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
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
                  i = _fragments.insert(i+1, f);
                  }
            else if (start > col && ((start+n) < endCol)) {
                  // middle
                  TextFragment lf = i->split(start+n - col);
                  TextFragment mf = i->split(start - col);
                  mf.changeFormat(id, data);
                  i = _fragments.insert(i+1, mf);
                  i = _fragments.insert(i+1, lf);
                  }
            else if (start > col) {
                  // right
                  TextFragment f = i->split(start - col);
                  f.changeFormat(id, data);
                  i = _fragments.insert(i+1, f);
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
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
            int idx = 0;
            for (const QChar& c : i->text) {
                  if (col == column) {
                        if (idx) {
                              if (idx < i->text.size()) {
                                    TextFragment tf(i->text.mid(idx));
                                    tf.format = i->format;
                                    tl._fragments.append(tf);
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
                                          tl._fragments.back().format.setType(CharFormatType::SYMBOL);
                                          tl._fragments.back().ids = l2;
                                          }
                                    ++i;
                                    }
                              }
                        for (; i != _fragments.end(); i = _fragments.erase(i))
                              tl._fragments.append(*i);
                        return tl;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  }
            }
      TextFragment tf("");
      if (_fragments.size() > 0)
            tf.format = _fragments.last().format;
      tl._fragments.append(tf);
      return tl;
      }

//---------------------------------------------------------
//   text
//    extract text, symbols are marked with <sym>xxx</sym>
//---------------------------------------------------------

QString TextBlock::text(int col1, int len) const
      {
      QString s;
      int col = 0;
      for (auto f : _fragments) {
            if (f.text.isEmpty())
                  continue;
            if (f.format.type() == CharFormatType::TEXT) {
                  for (const QChar& c : f.text) {
                        if (col >= col1 && (len < 0 || ((col-col1) < len)))
                              s += XmlWriter::xmlString(c.unicode());
                        if (!c.isHighSurrogate())
                              ++col;
                        }
                  }
            else {
                  for (SymId id : f.ids) {
                        if (col >= col1 && (len < 0 || ((col-col1) < len)))
                              s += QString("<sym>%1</sym>").arg(Sym::id2name(id));
                        ++col;
                        }
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
      _size = 10.0;
      initSubStyle(SubStyle::DEFAULT);          // we assume all properties are set
      setFlag(ElementFlag::MOVABLE, true);
      }

Text::Text(SubStyle st, Score* s)
   : Element(s)
      {
      _family                 = "FreeSerif";
      _size                   = 10.0;
      _bold                   = false;
      _italic                 = false;
      _underline              = false;
      _bgColor                = QColor(255, 255, 255, 0);
      _frameColor             = QColor(0, 0, 0, 255);
      _align                  = Align::LEFT;
      _hasFrame               = false;
      _circle                 = false;
      _square                 = false;
      _sizeIsSpatiumDependent = true;
      _frameWidth             = Spatium(0.1);
      _paddingWidth           = Spatium(0.2);
      _frameRound             = 0;
      _offset                 = QPointF();
      _offsetType             = OffsetType::SPATIUM;
      initSubStyle(st);
      setFlag(ElementFlag::MOVABLE, true);
      }

Text::Text(const Text& st)
   : Element(st)
      {
      _text                        = st._text;
      _layout                      = st._layout;
      textInvalid                  = st.textInvalid;
      layoutInvalid                = st.layoutInvalid;

      frame                        = st.frame;
      _subStyle                    = st._subStyle;
      _layoutToParentWidth         = st._layoutToParentWidth;
//      _editMode                    = false;
      hexState                     = -1;
      _family                      = st._family;
      _size                        = st._size;
      _bold                        = st._bold;
      _italic                      = st._italic;
      _underline                   = st._underline;
      _bgColor                     = st._bgColor;
      _frameColor                  = st._frameColor;
      _align                       = st._align;
      _hasFrame                    = st._hasFrame;
      _circle                      = st._circle;
      _square                      = st._square;
      _sizeIsSpatiumDependent      = st._sizeIsSpatiumDependent;
      _frameWidth                  = st._frameWidth;
      _paddingWidth                = st._paddingWidth;
      _frameRound                  = st._frameRound;
      _offset                      = st._offset;
      _offsetType                  = st._offsetType;
      _familyStyle                 = st._familyStyle;
      _sizeStyle                   = st._sizeStyle;
      _boldStyle                   = st._boldStyle;
      _italicStyle                 = st._italicStyle;
      _underlineStyle              = st._underlineStyle;
      _bgColorStyle                = st._bgColorStyle;
      _frameColorStyle             = st._frameColorStyle;
      _alignStyle                  = st._alignStyle;
      _hasFrameStyle               = st._hasFrameStyle;
      _circleStyle                 = st._circleStyle;
      _squareStyle                 = st._squareStyle;
      _sizeIsSpatiumDependentStyle = st._sizeIsSpatiumDependentStyle;
      _frameWidthStyle             = st._frameWidthStyle;
      _paddingWidthStyle           = st._paddingWidthStyle;
      _frameRoundStyle             = st._frameRoundStyle;
      _offsetStyle                 = st._offsetStyle;
      _offsetTypeStyle             = st._offsetTypeStyle;
      }

Text::~Text()
      {
      }

//---------------------------------------------------------
//   updateCursorFormat
//---------------------------------------------------------

void TextCursor::updateCursorFormat()
      {
      TextBlock* block = &_text->_layout[line()];
      int col = hasSelection() ? selectColumn() : column();
      const CharFormat* format = block->formatAt(col);
      if (format)
            setFormat(*format);
      else
            init();
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
//   cursorRect
//---------------------------------------------------------

QRectF TextCursor::cursorRect() const
      {
      const TextBlock& tline       = curLine();
      const TextFragment* fragment = tline.fragment(column());

      QFont _font  = fragment ? fragment->font(_text) : _text->font();
      qreal ascent = QFontMetricsF(_font, MScore::paintDevice()).ascent();
      qreal h = ascent;
      qreal x = tline.xpos(column(), _text);
      qreal y = tline.y() - ascent * .9;
      return QRectF(x, y, 4.0, h);
      }

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

QColor Text::textColor() const
      {
      if (score() && !score()->printing()) {
            if (selected())
                  return (track() > -1) ? MScore::selectColor[voice()] : MScore::selectColor[0];
            if (!visible())
                  return Qt::gray;
            }
      return color();
      }

//---------------------------------------------------------
//   insert
//     version for Supplementary Unicode, which must be inputted together as a pair
//---------------------------------------------------------

void Text::insert(TextCursor* cursor, QChar highSurrogate, QChar lowSurrogate)
      {
      if (cursor->hasSelection())
            deleteSelectedText(cursor);
      if (cursor->line() >= _layout.size())
            _layout.append(TextBlock());
      QString surrogatePair = QString(highSurrogate).append(lowSurrogate);
      _layout[cursor->line()].insert(cursor, surrogatePair);
      cursor->setColumn(cursor->column() + 1);
      cursor->clearSelection();
      }

//---------------------------------------------------------
//   insert
//     version for Basic Unicode
//---------------------------------------------------------

void Text::insert(TextCursor* cursor, QChar c)
      {
      if (cursor->hasSelection())
            deleteSelectedText(cursor);
      if (cursor->line() >= _layout.size())
            _layout.append(TextBlock());
      if (c == QChar::Tabulation)
            c = QChar::Space;
      else if (c == QChar::LineFeed) {
            _layout[cursor->line()].setEol(true);
            cursor->setLine(cursor->line() + 1);
            cursor->setColumn(0);
            if (_layout.size() <= cursor->line())
                  _layout.append(TextBlock());
            }
      else {
            _layout[cursor->line()].insert(cursor, QString(c));
            cursor->setColumn(cursor->column() + 1);
            }
      cursor->clearSelection();
      }

//---------------------------------------------------------
//   insert
//     version for SMUFL symbols
//---------------------------------------------------------

void Text::insert(TextCursor* cursor, SymId id)
      {
      if (cursor->hasSelection())
            deleteSelectedText(cursor);
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
      TextCursor cursor((Text*)this);
      cursor.init();

      int state = 0;
      QString token;
      QString sym;
      bool symState = false;
      for (int i = 0; i < _text.length(); i++) {
            const QChar& c = _text[i];
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
                        else {
                              if (c.isHighSurrogate()) {
                                    const QChar& highSurrogate = c;
                                    i++;
                                    Q_ASSERT(i < _text.length());
                                    const QChar& lowSurrogate = _text[i];
                                    insert(&cursor, highSurrogate, lowSurrogate);
                                    }
                              else
                                    insert(&cursor, c);
                              }
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
                              else if (token.startsWith("face=\"")) {
                                    QString face = parseStringProperty(token.mid(6));
                                    face = Text::unEscape(face);
                                    cursor.format()->setFontFamily(face);
                                    }
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
      layoutInvalid = false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
      QPointF o(_offset * (_offsetType == OffsetType::SPATIUM ? spatium() : DPI));

      setPos(o);
      layout1();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Text::layout1()
      {
      if (layoutInvalid)
            createLayout();

      if (_layout.empty())
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
            if (layoutToParentWidth()) {
                  if (parent()->isTBox()) {
                        // hack: vertical alignment is always TOP
                        _align = Align(((char)_align) & ((char)Align::HMASK)) | Align::TOP;
                        }
                  else if (parent()->isBox()) {
                        // consider inner margins of frame
                        Box* b = toBox(parent());
                        yoff = b->topMargin()  * DPMM;
                        h  = b->height() - yoff - b->bottomMargin() * DPMM;
                        }
                  else if (parent()->isPage()) {
                        Page* p = toPage(parent());
                        h = p->height() - p->tm() - p->bm();
                        yoff = p->tm();
                        }
                  else if (parent()->isMeasure())
                        ;
                  else
                        h  = parent()->height();
                  }
            }
      else
            setPos(QPointF());

      if (align() & Align::BOTTOM)
            yoff += h - bb.bottom();
      else if (align() & Align::VCENTER)
            yoff +=  (h - (bb.top() + bb.bottom())) * .5;
      else if (align() & Align::BASELINE)
            yoff += h * .5 - _layout.front().lineSpacing();
      else
            yoff += -bb.top();

      for (TextBlock& t : _layout)
            t.setY(t.y() + yoff);

      bb.translate(0.0, yoff);

#if 0
      if (_editMode)
            bb |= cursorRect();
#endif
      setbbox(bb);
      if (hasFrame())
            layoutFrame();
      }

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void Text::layoutFrame()
      {
      frame = bbox();
      if (square()) {
#if 0
            // "real" square
            if (frame.width() > frame.height()) {
                  qreal w = frame.width() - frame.height();
                  frame.adjust(0.0, -w * .5, 0.0, w * .5);
                  }
            else {
                  qreal w = frame.height() - frame.width();
                  frame.adjust(-w * .5, 0.0, w * .5, 0.0);
                  }
#endif
            // make sure width >= height
            if (frame.height() > frame.width()) {
                  qreal w = frame.height() - frame.width();
                  frame.adjust(-w * .5, 0.0, w * .5, 0.0);
                  }
            }
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

qreal Text::lineSpacing() const
      {
      return fontMetrics().lineSpacing() * MScore::pixelRatio;
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal Text::lineHeight() const
      {
      return fontMetrics().height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal Text::baseLine() const
      {
      return fontMetrics().ascent();
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
      bool _bold      = false;
      bool _italic    = false;
      bool _underline = false;

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (!f.format.bold() && bold())
                        _bold = true;
                  if (!f.format.italic() && italic())
                        _italic = true;
                  if (!f.format.underline() && underline())
                        _underline = true;
                  }
            }
      TextCursor cursor(this);
      cursor.init();

      XmlNesting xmlNesting(&_text);
      if (_bold)
            xmlNesting.pushB();
      if (_italic)
            xmlNesting.pushI();
      if (_underline)
            xmlNesting.pushU();

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (f.text.isEmpty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                  const CharFormat& format = f.format;
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
                        _text += QString("<font face=\"%1\"/>").arg(Text::escape(format.fontFamily()));

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
                  if (format.type() == CharFormatType::TEXT)
                        _text += XmlWriter::xmlString(f.text);
                  else {
                        for (SymId id : f.ids)
                              _text += QString("<sym>%1</sym>").arg(Sym::id2name(id));
                        }
                  cursor.setFormat(format);
                  }
            if (block.eol())
                  _text += QChar::LineFeed;
            }
      while (!xmlNesting.empty())
            xmlNesting.popToken();
      textInvalid = false;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Text::startEdit(EditData& ed)
      {
      TextEditData* ted = new TextEditData();
      ted->e      = this;
      ted->cursor = new TextCursor(this);
      ted->cursor->setText(this);
      ted->cursor->setLine(0);
      ted->cursor->setColumn(0);
      ted->cursor->clearSelection();

printf("%p Text::startEdit %f\n", this, ed.startMove.x());
      if (!ted->cursor->set(ed.startMove)) {
            ted->cursor->init();
            printf("    init Cursor\n");
            }
      ed.addData(ted);
      if (layoutInvalid)
            layout();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit(EditData&)
      {
printf("%p Text::endEdit\n", this);
      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

const TextBlock& TextCursor::curLine() const
      {
      return _text->_layout[_line];
      }

TextBlock& TextCursor::curLine()
      {
      return _text->_layout[_line];
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Text::edit(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;

      // do nothing on Shift, it messes up IME on Windows. See #64046
      if (ed.key == Qt::Key_Shift)
            return false;
      QString s         = ed.s;
      bool ctrlPressed  = ed.modifiers & Qt::ControlModifier;
      bool shiftPressed = ed.modifiers & Qt::ShiftModifier;

      QTextCursor::MoveMode mm = shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

      bool wasHex = false;
      if (hexState >= 0) {
            if (ed.modifiers == (Qt::ControlModifier | Qt::ShiftModifier | Qt::KeypadModifier)) {
                  switch (ed.key) {
                        case Qt::Key_0 ... Qt::Key_9:
                              s = QChar::fromLatin1(ed.key);
                              ++hexState;
                              wasHex = true;
                              break;
                        default:
                              break;
                        }
                  }
            else if (ed.modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
                  switch (ed.key) {
                        case Qt::Key_A ... Qt::Key_F:
                              s = QChar::fromLatin1(ed.key);
                              ++hexState;
                              wasHex = true;
                              break;
                        default:
                              break;
                        }
                  }
            }

      if (!wasHex) {
            switch (ed.key) {
                  case Qt::Key_Enter:
                  case Qt::Key_Return: {
                        if (_cursor->hasSelection())
                              deleteSelectedText(_cursor);
                        int line = _cursor->line();

                        CharFormat* charFmt = _cursor->format();         // take current format
                        _layout.insert(line + 1, _cursor->curLine().split(_cursor->column()));
                        _layout[line].setEol(true);
                        if (_layout.last() != _layout[line+1])
                              _layout[line+1].setEol(true);

                        _cursor->setLine(line+1);
                        _cursor->setColumn(0);
                        _cursor->setFormat(*charFmt);                    // restore orig. format at new line
                        s.clear();
                        _cursor->clearSelection();
                        break;
                        }

                  case Qt::Key_Backspace:
                        if (_cursor->hasSelection())
                              deleteSelectedText(_cursor);
                        else {
                              if (!_cursor->movePosition(QTextCursor::Left))
                                    return false;
                              return _cursor->deleteChar();
                              }
                        s.clear();
                        break;

                  case Qt::Key_Delete:
                        if (_cursor->hasSelection())
                              deleteSelectedText(_cursor);
                        else if (!_cursor->deleteChar())
                              return false;
                        s.clear();
                        break;

                  case Qt::Key_Left:
                        if (!_cursor->movePosition(ctrlPressed ? QTextCursor::WordLeft : QTextCursor::Left, mm) && type() == ElementType::LYRICS)
                              return false;
                        s.clear();
                        break;

                  case Qt::Key_Right:
                        if (!_cursor->movePosition(ctrlPressed ? QTextCursor::NextWord : QTextCursor::Right, mm) && type() == ElementType::LYRICS)
                              return false;
                        s.clear();
                        break;

                  case Qt::Key_Up:
                        _cursor->movePosition(QTextCursor::Up, mm);
                        s.clear();
                        break;

                  case Qt::Key_Down:
                        _cursor->movePosition(QTextCursor::Down, mm);
                        s.clear();
                        break;

                  case Qt::Key_Home:
                        _cursor->movePosition(QTextCursor::Start, mm);
                        s.clear();
                        break;

                  case Qt::Key_End:
                        _cursor->movePosition(QTextCursor::End, mm);
                        s.clear();
                        break;

                  case Qt::Key_Tab:
                  case Qt::Key_Space:
                        s = " ";
                        ed.modifiers = 0;
                        break;

                  case Qt::Key_Minus:
                        if (ed.modifiers == 0)
                              s = "-";
                        break;

                  case Qt::Key_Underscore:
                        if (ed.modifiers == 0)
                              s = "_";
                        break;

                  case Qt::Key_A:
                        if (ctrlPressed) {
                              selectAll(_cursor);
                              s.clear();
                        }
                        break;
                  default:
                        break;
                  }
            if (ctrlPressed && shiftPressed) {
                  switch (ed.key) {
                        case Qt::Key_U:
                              if (hexState == -1) {
                                    hexState = 0;
                                    s = "u";
                                    }
                              break;
                        case Qt::Key_B:
                              _cursor->insertSym(SymId::accidentalFlat);
                              s.clear();
                              break;
                        case Qt::Key_NumberSign:
                              _cursor->insertSym(SymId::accidentalSharp);
                              s.clear();
                              break;
                        case Qt::Key_H:
                              _cursor->insertSym(SymId::accidentalNatural);
                              s.clear();
                              break;
                        case Qt::Key_Space:
                              _cursor->insertSym(SymId::space);
                              s.clear();
                              break;
                        case Qt::Key_F:
                              _cursor->insertSym(SymId::dynamicForte);
                              s.clear();
                              break;
                        case Qt::Key_M:
                              _cursor->insertSym(SymId::dynamicMezzo);
                              s.clear();
                              break;
                        case Qt::Key_N:
                              _cursor->insertSym(SymId::dynamicNiente);
                              s.clear();
                              break;
                        case Qt::Key_P:
                              _cursor->insertSym(SymId::dynamicPiano);
                              s.clear();
                              break;
                        case Qt::Key_S:
                              _cursor->insertSym(SymId::dynamicSforzando);
                              s.clear();
                              break;
                        case Qt::Key_R:
                              _cursor->insertSym(SymId::dynamicRinforzando);
                              s.clear();
                              break;
                        case Qt::Key_Z:
                              // Ctrl+Z is normally "undo"
                              // but this code gets hit even if you are also holding Shift
                              // so Shift+Ctrl+Z works
                              _cursor->insertSym(SymId::dynamicZ);
                              s.clear();
                              break;
                        }
                  }
            }
      if (!s.isEmpty()) {
            score()->undoStack()->push1(new InsertText(_cursor, s));
            editInsertText(_cursor, s);
            }
      return true;
      }

//---------------------------------------------------------
//   editInsertText
//---------------------------------------------------------

void Text::editInsertText(TextCursor* _cursor, const QString& s)
      {
      textInvalid = true;
      QRectF refresh(canvasBoundingRect());
      insertText(_cursor, s);
      layout1();
      if (parent() && parent()->isTBox()) {
            TBox* tbox = toTBox(parent());
            tbox->layout();
            System* system = tbox->system();
            system->setHeight(tbox->height());
            score()->setUpdateAll();
            }
      else {
            static const qreal w = 2.0; // 8.0 / view->matrix().m11();
            refresh |= canvasBoundingRect();
            score()->addRefresh(refresh.adjusted(-w, -w, w, w));
            }
      }

//---------------------------------------------------------
//   endHexState
//---------------------------------------------------------

void Text::endHexState()
      {
#if 0
      if (hexState >= 0) {
            if (hexState > 0) {
                  int c2 = _cursor->column();
                  int c1 = c2 - (hexState + 1);

                  TextBlock& t = _layout[_cursor->line()];
                  QString ss   = t.remove(c1, hexState + 1);
                  bool ok;
                  int code     = ss.mid(1).toInt(&ok, 16);
                  _cursor->setColumn(c1);
                  _cursor->clearSelection();
                  if (ok)
                        editInsertText(QString(code));
                  else
                        qDebug("cannot convert hex string <%s>, state %d (%d-%d)",
                           qPrintable(ss.mid(1)), hexState, c1, c2);
                  }
            hexState = -1;
            }
#endif
      }

//---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void Text::selectAll(TextCursor* _cursor)
      {
      _cursor->setSelectLine(0);
      _cursor->setSelectColumn(0);
      _cursor->setLine(_layout.size() - 1);
      _cursor->setColumn(_cursor->curLine().columns());
      }

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

bool TextCursor::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode, int count)
      {
      for (int i = 0; i < count; i++) {
            switch (op) {
                  case QTextCursor::Left:
                        if (hasSelection() && mode == QTextCursor::MoveAnchor) {
                              int r1 = selectLine();
                              int r2 = line();
                              int c1 = selectColumn();
                              int c2 = column();

                              if (r1 > r2) {
                                    qSwap(r1, r2);
                                    qSwap(c1, c2);
                                    }
                              else if (r1 == r2) {
                                    if (c1 > c2)
                                           qSwap(c1, c2);
                                    }
                              clearSelection();
                              setLine(r1);
                              setColumn(c1);
                              }
                        else if (column() == 0) {
                              if (line() == 0)
                                    return false;
                              setLine(line()-1);
                              setColumn(curLine().columns());
                              }
                        else
                              setColumn(column()-1);
                        break;

                  case QTextCursor::Right:
                        if (hasSelection() && mode == QTextCursor::MoveAnchor) {
                              int r1 = selectLine();
                              int r2 = line();
                              int c1 = selectColumn();
                              int c2 = column();

                              if (r1 > r2) {
                                    qSwap(r1, r2);
                                    qSwap(c1, c2);
                                    }
                              else if (r1 == r2) {
                                    if (c1 > c2)
                                           qSwap(c1, c2);
                                    }
                              clearSelection();
                              setLine(r2);
                              setColumn(c2);
                              }
                        else if (column() >= curLine().columns()) {
                              if (line() >= _text->_layout.size()-1)
                                    return false;
                              setLine(line()+1);
                              setColumn(0);
                              }
                        else
                              setColumn(column()+1);
                        break;

                  case QTextCursor::Up:
                        if (line() == 0)
                              return false;
                        setLine(line()-1);
                        if (column() > curLine().columns())
                              setColumn(curLine().columns());
                        break;

                  case QTextCursor::Down:
                        if (line() >= _text->_layout.size()-1)
                              return false;
                        setLine(line()+1);
                        if (column() > curLine().columns())
                              setColumn(curLine().columns());
                        break;

                  case QTextCursor::Start:
                        setLine(0);
                        setColumn(0);
                        break;

                  case QTextCursor::End:
                        setLine(_text->_layout.size() - 1);
                        setColumn(curLine().columns());
                        break;

                  case QTextCursor::StartOfLine:
                        setColumn(0);
                        break;

                  case QTextCursor::EndOfLine:
                        setColumn(curLine().columns());
                        break;

                  case QTextCursor::WordLeft:
                        if (column() > 0) {
                              setColumn(column()-1);
                              while (column() > 0 && currentCharacter().isSpace())
                                    setColumn(column()-1);
                              while (column() > 0 && !currentCharacter().isSpace())
                                    setColumn(column()-1);
                              if (currentCharacter().isSpace())
                                    setColumn(column()+1);
                              }
                        break;

                  case QTextCursor::NextWord: {
                        int cols =  columns();
                        if (column() < cols) {
                              setColumn(column() + 1);
                              while (column() < cols && !currentCharacter().isSpace())
                                    setColumn(column()+1);
                              while (column() < cols && currentCharacter().isSpace())
                                    setColumn(column()+1);
                              }
                        }
                        break;

                  default:
                        qDebug("Text::movePosition: not implemented");
                        return false;
                  }
            if (mode == QTextCursor::MoveAnchor)
                  clearSelection();
            }
      updateCursorFormat();
      _text->score()->addRefresh(_text->canvasBoundingRect());
      return true;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

bool TextCursor::set(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - _text->canvasPos();
      if (!_text->bbox().contains(pt))
            return false;
      setLine(0);
      for (int row = 0; row < _text->_layout.size(); ++row) {
            const TextBlock& l = _text->_layout.at(row);
            if (l.y() > pt.y()) {
                  setLine(row);
                  break;
                  }
            }
      setColumn(curLine().column(pt.x(), _text));
printf("cursor set col %d\n", _column);

      _text->score()->setUpdateAll();
      if (mode == QTextCursor::MoveAnchor)
            clearSelection();
      if (hasSelection())
            QApplication::clipboard()->setText(selectedText(), QClipboard::Selection);
      updateCursorFormat();
      return true;
      }

//---------------------------------------------------------
//   selectedText
//    return current selection
//---------------------------------------------------------

QString TextCursor::selectedText() const
      {
      QString s;
      int r1 = selectLine();
      int r2 = line();
      int c1 = selectColumn();
      int c2 = column();

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _text->_layout.size();
      for (int row = 0; row < rows; ++row) {
            const TextBlock& t = _text->_layout.at(row);
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        s += t.text(c1, c2 - c1);
                  else if (row == r1) {
                        s += t.text(c1, -1);
                        s += "\n";
                        }
                  else if (row == r2)
                        s += t.text(0, c2);
                  else {
                        s += t.text(0, -1);
                        s += "\n";
                        }
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   deleteSelectedText
//---------------------------------------------------------

void Text::deleteSelectedText(TextCursor* _cursor)
      {
      int r1 = _cursor->selectLine();
      int r2 = _cursor->line();
      int c1 = _cursor->selectColumn();
      int c2 = _cursor->column();

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
            TextBlock& t = _layout[row];
            if (row >= r1 && row <= r2) {
                  if (row == r1 && r1 == r2)
                        t.remove(c1, c2 - c1);
                  else if (row == r1)
                        t.remove(c1, t.columns() - c1);
                  else if (row == r2)
                        t.remove(0, c2);
                  }
            }
      if (r1 != r2) {
            TextBlock& l1       = _layout[r1];
            const TextBlock& l2 = _layout[r2];
            for (const TextFragment& f : l2.fragments())
                  l1.fragments().append(f);
            _layout.erase(_layout.begin() + r1 + 1, _layout.begin() + r2 + 1);
            if (_layout.last() == l1)
                  l1.setEol(false);
            }
      _cursor->setLine(r1);
      _cursor->setColumn(c1);
      _cursor->clearSelection();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(XmlWriter& xml) const
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

static const std::array<P_ID, 18> pids { {
      P_ID::SUB_STYLE,
      P_ID::FONT_FACE,
      P_ID::FONT_SIZE,
      P_ID::FONT_BOLD,
      P_ID::FONT_ITALIC,
      P_ID::FONT_UNDERLINE,
      P_ID::FRAME,
      P_ID::FRAME_SQUARE,
      P_ID::FRAME_CIRCLE,
      P_ID::FRAME_WIDTH,
      P_ID::FRAME_PADDING,
      P_ID::FRAME_ROUND,
      P_ID::FRAME_FG_COLOR,
      P_ID::FRAME_BG_COLOR,
      P_ID::FONT_SPATIUM_DEPENDENT,
      P_ID::ALIGN,
      P_ID::OFFSET,
      P_ID::OFFSET_TYPE
      } };

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(XmlWriter& xml, bool writeText, bool /*writeStyle*/) const
      {
      Element::writeProperties(xml);
      for (P_ID i :pids)
            writeProperty(xml, i);
      if (writeText)
            xml.writeXml("text", xmlText());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "style") {
            SubStyle s = subStyleFromName(e.readElementText());
            initSubStyle(s);
            return true;
            }

      for (P_ID i :pids) {
            if (readProperty(tag, e, i)) {
                  setPropertyFlags(i, PropertyFlags::UNSTYLED);
                  return true;
                  }
            }
      if (tag == "text")
            //_text = e.readXml();
            setXmlText(e.readXml());
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   insertText
//    insert text at cursor position and move cursor
//---------------------------------------------------------

void Text::insertText(TextCursor* _cursor, const QString& s)
      {
      if (_cursor->hasSelection())
            deleteSelectedText(_cursor);
      if (_cursor->format()->type() == CharFormatType::SYMBOL) {
            QString face = family();
            _cursor->format()->setFontFamily(face);
            _cursor->format()->setType(CharFormatType::TEXT);
            }
      _cursor->curLine().insert(_cursor, s);
      _cursor->setColumn(_cursor->column() + s.size());
      _cursor->clearSelection();
      }

//---------------------------------------------------------
//   insertSym
//---------------------------------------------------------

void TextCursor::insertSym(SymId id)
      {
      if (hasSelection())
            _text->deleteSelectedText(this);
      curLine().insert(this, id);
      setColumn(column() + 1);
      clearSelection();
      _text->layout();
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == ElementType::HBOX || parent()->type() == ElementType::VBOX || parent()->type() == ElementType::TBOX)) {
            Box* box = static_cast<Box*>(parent());
            QRectF r = box->abbox();
            qreal x = r.x() + box->leftMargin() * DPMM;
            qreal y = r.y() + box->topMargin() * DPMM;
            qreal h = r.height() - (box->topMargin() + box->bottomMargin()) * DPMM;
            qreal w = r.width()  - (box->leftMargin() + box->rightMargin()) * DPMM;

            // QSizeF ps = _doc->pageSize();
            // return QRectF(x, y, ps.width(), ps.height());

            return QRectF(x, y, w, h);
            }
      if (parent() && parent()->type() == ElementType::PAGE) {
            Page* box  = static_cast<Page*>(parent());
            QRectF r = box->abbox();
            qreal x = r.x() + box->lm();
            qreal y = r.y() + box->tm();
            qreal h = r.height() - box->tm() - box->bm();
            qreal w = r.width()  - box->lm() - box->rm();
            return QRectF(x, y, w, h);
            }
      return abbox();
      }

//---------------------------------------------------------
//   dragTo
//---------------------------------------------------------

void Text::dragTo(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      _cursor->set(ed.pos, QTextCursor::KeepAnchor);
      score()->setUpdateAll();
      score()->update();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Text::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp;
      if (parent()->type() == ElementType::SEGMENT) {
            System* system = static_cast<Segment*>(parent())->measure()->system();
            yp = system->staffCanvasYpage(staffIdx());
            }
      else
            yp = parent()->canvasPos().y();
      QPointF p1(xp, yp);
      QPointF p2 = canvasPos();
      if (layoutToParentWidth())
            p2 += bbox().topLeft();
      return QLineF(p1, p2);
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void Text::paste(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;

      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
      if (MScore::debugMode)
            qDebug("Text::paste() <%s>", qPrintable(txt));

      int state = 0;
      QString token;
      QString sym;
      bool symState = false;

      for (int i = 0; i < txt.length(); i++ ) {
            QChar c = txt[i];
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
                        else {
                              if (c.isHighSurrogate()) {
                                    QChar highSurrogate = c;
                                    Q_ASSERT(i + 1 < txt.length());
                                    i++;
                                    QChar lowSurrogate = txt[i];
                                    insert(_cursor, highSurrogate, lowSurrogate);
                                    }
                              else {
                                    insert(_cursor, c);
                                    }
                              }
                        }
                  }
            else if (state == 1) {
                  if (c == '>') {
                        state = 0;
                        if (token == "sym") {
                              symState = true;
                              sym.clear();
                              }
                        else if (token == "/sym") {
                              symState = false;
                              _cursor->insertSym(Sym::name2id(sym));
                              }
                        }
                  else
                        token += c;
                  }
            else if (state == 2) {
                  if (c == ';') {
                        state = 0;
                        if (token == "lt")
                              insertText(_cursor, "<");
                        else if (token == "gt")
                              insertText(_cursor, ">");
                        else if (token == "amp")
                              insertText(_cursor, "&");
                        else if (token == "quot")
                              insertText(_cursor, "\"");
                        else
                              _cursor->insertSym(Sym::name2id(token));
                        }
                  else if (!c.isLetter()) {
                        state = 0;
                        insertText(_cursor, "&");
                        insertText(_cursor, token);
                        insertText(_cursor, c);
                        }
                  else
                        token += c;
                  }
            }
      if (state == 2) {
          insertText(_cursor, "&");
          insertText(_cursor, token);
          }
      layoutEdit();
      score()->setUpdateAll();
      if (type() == ElementType::INSTRUMENT_NAME)
            score()->setLayoutAll();
      triggerLayout();
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool Text::mousePress(EditData& ed, QMouseEvent* ev)
      {
printf("========================mousePRess %p\n", this);
      QPointF p = ed.startMove;
      bool shift = ev->modifiers() & Qt::ShiftModifier;
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      if (!_cursor->set(p, shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor))
            return false;
      if (ev->button() == Qt::MidButton)
            paste(ed);
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
            triggerLayout();
            }
      else {
            static const qreal w = 2.0; // 8.0 / view->matrix().m11();
            score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Text::acceptDrop(EditData& data) const
      {
      ElementType type = data.element->type();
      return type == ElementType::SYMBOL || type == ElementType::FSYMBOL;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Text::drop(EditData& /*data*/)
      {
#if 0
      Element* e = data.element;
      switch(e->type()) {
            case ElementType::SYMBOL:
                  {
                  SymId id = static_cast<Symbol*>(e)->sym();
                  delete e;

                  if (_editMode) {
                        insert(_cursor, id);
                        layout1();
                        static const qreal w = 2.0; // 8.0 / view->matrix().m11();
                        score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
                        }
                  else {
                        startEdit(data);
                        curLine().insert(_cursor, id);
                        endEdit(data);
                        }
                  }
                  return 0;

            case ElementType::FSYMBOL:
                  {
                  int code = static_cast<FSymbol*>(e)->code();
                  delete e;

                  if (_editMode) {
                        if (QChar::requiresSurrogates(code))
                              insert(_cursor, QChar::highSurrogate(code), QChar::lowSurrogate(code));
                        else
                              insert(_cursor, QChar(code));
                        layout1();
                        static const qreal w = 2.0; // 8.0 / view->matrix().m11();
                        score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
                        }
                  else {
                        startEdit(data);
                        if (QChar::requiresSurrogates(code)) {
                              QString surrogatePair = QString(QChar::highSurrogate(code)).append(QChar::lowSurrogate(code));
                              curLine().insert(_cursor, surrogatePair);
                              }
                        else
                              curLine().insert(_cursor, QChar(code));
                        endEdit(data);
                        }
                  }
                  return 0;

            default:
                  break;
            }
#endif
      return 0;
      }

//---------------------------------------------------------
//   setPlainText
//---------------------------------------------------------

void Text::setPlainText(const QString& s)
      {
      setXmlText(s.toHtmlEscaped());
      }

//---------------------------------------------------------
//   setXmlText
//---------------------------------------------------------

void Text::setXmlText(const QString& s)
      {
      _text = s;
      layoutInvalid = true;
      textInvalid   = false;
      textChanged();
      }

//---------------------------------------------------------
//   plainText
//    return plain text with symbols
//---------------------------------------------------------

QString Text::plainText(bool noSym) const
      {
      QString s;

      if (layoutInvalid)
            ((Text*)(this))->createLayout();  // ugh!

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  const CharFormat& format = f.format;
                  if (format.type() == CharFormatType::TEXT) {
                        s += f.text;
                        }
                  else if (noSym) {
                        // do some simple symbol substitution
                        for (SymId id : f.ids) {
                              switch (id) {
                                    case SymId::accidentalFlat:
                                          s += "b";
                                          break;
                                    case SymId::accidentalSharp:
                                          s += "#";
                                          break;
                                    default:
                                          break;
                                    }
                              }
                        }
                  else {
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
//   xmlText
//---------------------------------------------------------

QString Text::xmlText() const
      {
      if (textInvalid)
            ((Text*)(this))->genText();    // ugh!
      return _text;
      }

//---------------------------------------------------------
//   changeSelectionFormat
//---------------------------------------------------------

void TextCursor::changeSelectionFormat(FormatId id, QVariant val)
      {
      if (!hasSelection())
            return;
      int r1 = selectLine();
      int r2 = line();
      int c1 = selectColumn();
      int c2 = column();

      if (r1 > r2) {
            qSwap(r1, r2);
            qSwap(c1, c2);
            }
      else if (r1 == r2) {
            if (c1 > c2)
                  qSwap(c1, c2);
            }
      int rows = _text->_layout.size();
      QList<TextBlock> toDelete;
      for (int row = 0; row < rows; ++row) {
            TextBlock& t = _text->_layout[row];
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
      _text->layout1();
      _text->score()->addRefresh(_text->canvasBoundingRect());
      }

//---------------------------------------------------------
//   setFormat
//---------------------------------------------------------

void TextCursor::setFormat(FormatId id, QVariant val)
      {
      changeSelectionFormat(id, val);
      format()->setFormat(id, val);
      }

//---------------------------------------------------------
//   restyle
//    restyle from old style type s
//---------------------------------------------------------

#if 0
void Text::restyle(StyledPropertyListIdx oldType)
      {
      const TextStyle& os = score()->textStyle(oldType);
      const TextStyle& ns = score()->textStyle(textStyleType());
      _textStyle.restyle(os, ns);
      }
#endif

//---------------------------------------------------------
//   convertFromHtml
//---------------------------------------------------------

QString Text::convertFromHtml(const QString& ss) const
      {
      QTextDocument doc;
      doc.setHtml(ss);

      QString s;
      qreal _size = size();
      QString _family = family();
      for (auto b = doc.firstBlock(); b.isValid() ; b = b.next()) {
            if (!s.isEmpty())
                  s += "\n";
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                  QTextFragment f = it.fragment();
                  if (f.isValid()) {
                        QTextCharFormat tf = f.charFormat();
                        QFont font = tf.font();
                        qreal htmlSize = font.pointSizeF();
                        // html font sizes may have spatium adjustments; need to undo this
                        if (sizeIsSpatiumDependent())
                              htmlSize *= SPATIUM20 / spatium();
                        if (fabs(_size - htmlSize) > 0.1) {
                              _size = htmlSize;
                              s += QString("<font size=\"%1\"/>").arg(_size);
                              }
                        if (_family != font.family()) {
                              _family = font.family();
                              s += QString("<font face=\"%1\"/>").arg(_family);
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

      if (score() && score()->mscVersion() <= 114) {
            s.replace(QChar(0xe10e), QString("<sym>accidentalNatural</sym>"));  //natural
            s.replace(QChar(0xe10c), QString("<sym>accidentalSharp</sym>"));    // sharp
            s.replace(QChar(0xe10d), QString("<sym>accidentalFlat</sym>"));     // flat
            s.replace(QChar(0xe104), QString("<sym>metNoteHalfUp</sym>")),      // note2_Sym
            s.replace(QChar(0xe105), QString("<sym>metNoteQuarterUp</sym>"));   // note4_Sym
            s.replace(QChar(0xe106), QString("<sym>metNote8thUp</sym>"));       // note8_Sym
            s.replace(QChar(0xe107), QString("<sym>metNote16thUp</sym>"));      // note16_Sym
            s.replace(QChar(0xe108), QString("<sym>metNote32ndUp</sym>"));      // note32_Sym
            s.replace(QChar(0xe109), QString("<sym>metNote64thUp</sym>"));      // note64_Sym
            s.replace(QChar(0xe10a), QString("<sym>metAugmentationDot</sym>")); // dot
            s.replace(QChar(0xe10b), QString("<sym>metAugmentationDot</sym><sym>space</sym><sym>metAugmentationDot</sym>"));    // dotdot
            s.replace(QChar(0xe167), QString("<sym>segno</sym>"));              // segno
            s.replace(QChar(0xe168), QString("<sym>coda</sym>"));               // coda
            s.replace(QChar(0xe169), QString("<sym>codaSquare</sym>"));         // varcoda
            }
      return s;
      }

//---------------------------------------------------------
//   convertToHtml
//    convert from internal html format to Qt
//---------------------------------------------------------

QString Text::convertToHtml(const QString& s, const TextStyle& /*st*/)
      {
//TODO      qreal size     = st.size();
//      QString family = st.family();
      qreal size     = 10;
      QString family = "arial";
      return QString("<html><body style=\"font-family:'%1'; font-size:%2pt;\">%3</body></html>").arg(family).arg(size).arg(s);
      }

//---------------------------------------------------------
//   tagEscape
//---------------------------------------------------------

QString Text::tagEscape(QString s)
      {
      QStringList tags = { "sym", "b", "i", "u", "sub", "sup" };
      for (QString tag : tags) {
            QString openTag = "<" + tag + ">";
            QString openProxy = "!!" + tag + "!!";
            QString closeTag = "</" + tag + ">";
            QString closeProxy = "!!/" + tag + "!!";
            s.replace(openTag, openProxy);
            s.replace(closeTag, closeProxy);
            }
      s = XmlWriter::xmlString(s);
      for (QString tag : tags) {
            QString openTag = "<" + tag + ">";
            QString openProxy = "!!" + tag + "!!";
            QString closeTag = "</" + tag + ">";
            QString closeProxy = "!!/" + tag + "!!";
            s.replace(openProxy, openTag);
            s.replace(closeProxy, closeTag);
            }
      return s;
      }

//---------------------------------------------------------
//   unEscape
//---------------------------------------------------------

QString Text::unEscape(QString s)
      {
      s.replace("&lt;", "<");
      s.replace("&gt;", ">");
      s.replace("&amp;", "&");
      s.replace("&quot;", "\"");
      return s;
      }

//---------------------------------------------------------
//   escape
//---------------------------------------------------------

QString Text::escape(QString s)
      {
      s.replace("<", "&lt;");
      s.replace(">", "&gt;");
      s.replace("&", "&amp;");
      s.replace("\"", "&quot;");
      return s;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Text::accessibleInfo() const
      {
      QString rez;
      switch (subStyle()) {
            case SubStyle::TITLE:
            case SubStyle::SUBTITLE:
            case SubStyle::COMPOSER:
            case SubStyle::POET:
            case SubStyle::TRANSLATOR:
            case SubStyle::MEASURE_NUMBER:
                  rez = subStyleUserName(subStyle());
                  break;
            default:
                  rez = Element::accessibleInfo();
                  break;
            }
      QString s = plainText(true).simplified();
      if (s.length() > 20) {
            s.truncate(20);
            s += "...";
            }
      return  QString("%1: %2").arg(rez).arg(s);
      }

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Text::subtype() const
      {
      switch (subStyle()) {
            case SubStyle::TITLE:
            case SubStyle::SUBTITLE:
            case SubStyle::COMPOSER:
            case SubStyle::POET:
            case SubStyle::FRAME:
            case SubStyle::INSTRUMENT_EXCERPT:
                  return int(subStyle());
            default: return -1;
            }
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Text::subtypeName() const
      {
      QString rez;
      switch (subStyle()) {
            case SubStyle::TITLE:
            case SubStyle::SUBTITLE:
            case SubStyle::COMPOSER:
            case SubStyle::POET:
            case SubStyle::FRAME:
            case SubStyle::INSTRUMENT_EXCERPT:
                  rez = subStyleUserName(subStyle());
                  break;
            default: rez = "";
            }
      return rez;
      }

//---------------------------------------------------------
//   fragmentList
//---------------------------------------------------------

/**
 Return the text as a single list of TextFragment
 Used by the MusicXML formatted export to avoid parsing the xml text format
 */

QList<TextFragment> Text::fragmentList() const
      {
      QList<TextFragment> res;
      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  /* TODO TBD
                  if (f.text.empty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                   */
                  res.append(f);
                  if (block.eol()) {
                        if (f.format.type() == CharFormatType::TEXT) {
                              // simply append a newline
                              res.last().text += "\n";
                              }
                        else {
                              // create and append a fragment containing only a newline,
                              // with the same formatting as f
                              TextFragment newline("\n");
                              newline.changeFormat(FormatId::FontSize, f.format.fontSize());
                              newline.changeFormat(FormatId::FontFamily, f.format.fontFamily());
                              newline.changeFormat(FormatId::Bold, f.format.bold());
                              newline.changeFormat(FormatId::Underline, f.format.underline());
                              newline.changeFormat(FormatId::Italic, f.format.italic());
                              res.append(newline);
                              }
                        }
                  }
            }
      return res;
      }

//---------------------------------------------------------
//   validateText
//    check if s is a valid musescore xml text string
//    - simple bugs are automatically adjusted
//   return true if text is valid or could be fixed
//  (this is incomplete/experimental)
//---------------------------------------------------------

bool Text::validateText(QString& s)
      {
      QString d;
      for (int i = 0; i < s.size(); ++i) {
            QChar c = s[i];
            if (c == '&') {
                  const char* ok[] { "amp;", "lt;", "gt;", "quot" };
                  QString t = s.mid(i+1);
                  bool found = false;
                  for (auto k : ok) {
                        if (t.startsWith(k)) {
                              d.append(c);
                              d.append(k);
                              i += strlen(k);
                              found = true;
                              break;
                              }
                        }
                  if (!found)
                        d.append("&amp;");
                  }
            else if (c == '<') {
                  const char* ok[] { "b>", "/b>", "i>", "/i>", "u>", "/u", "font ", "/font>", "sym>", "/sym>" };
                  QString t = s.mid(i+1);
                  bool found = false;
                  for (auto k : ok) {
                        if (t.startsWith(k)) {
                              d.append(c);
                              d.append(k);
                              i += strlen(k);
                              found = true;
                              break;
                              }
                        }
                  if (!found)
                        d.append("&lt;");
                  }
            else
                  d.append(c);
            }
      QString ss = "<data>" + d + "</data>\n";
      XmlReader xml(0, ss);
      while (xml.readNextStartElement())
            ; // qDebug("  token %d <%s>", int(xml.tokenType()), qPrintable(xml.name().toString()));
      if (xml.error() == QXmlStreamReader::NoError) {
            s = d;
            return true;
            }
      qDebug("xml error at line %lld column %lld: %s",
            xml.lineNumber(),
            xml.columnNumber(),
            qPrintable(xml.errorString()));
      qDebug ("text: |%s|", qPrintable(ss));
      return false;
      }

//---------------------------------------------------------
//   inputTransition
//---------------------------------------------------------

void Text::inputTransition(QInputMethodEvent* ie)
      {
#if 0
      // remove preedit string
      int n = preEdit.size();
      while (n--) {
            if (movePosition(QTextCursor::Left))
                  _cursor->deleteChar();
            }

      qDebug("Text::inputTransition <%s><%s> len %d start %d, preEdit size %d",
         qPrintable(ie->commitString()),
         qPrintable(ie->preeditString()),
         ie->replacementLength(), ie->replacementStart(), preEdit.size());

      if (!ie->commitString().isEmpty()) {
            _cursor->format()->setPreedit(false);
            editInsertText(ie->commitString());
            preEdit.clear();
            }
      else  {
            preEdit = ie->preeditString();
            if (!preEdit.isEmpty()) {
#if 0
                  for (auto a : ie->attributes()) {
                        switch(a.type) {
                              case QInputMethodEvent::TextFormat:
                                    {
                                    printf("attribute TextFormat: %d-%d\n", a.start, a.length);
                                    QTextFormat tf = a.value.value<QTextFormat>();
                                    }
                                    break;
                              case QInputMethodEvent::Cursor:
                                    printf("attribute Cursor at %d\n", a.start);
                                    break;
                              default:
                                    printf("attribute %d\n", a.type);
                              }
                        }
#endif
                  _cursor->format()->setPreedit(true);
                  editInsertText(preEdit);
                  ie->accept();
                  score()->update();
                  }
            }
#endif
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Text::font() const
      {
      qreal m = _size;
      if (_sizeIsSpatiumDependent)
            m *= spatium() / SPATIUM20;
      QFont f(_family, m, _bold ? QFont::Bold : QFont::Normal, _italic);
      if (_underline)
            f.setUnderline(_underline);
      return f;
      }

//---------------------------------------------------------
//   fontMetrics
//---------------------------------------------------------

QFontMetricsF Text::fontMetrics() const
      {
      return QFontMetricsF(font());
      }

//---------------------------------------------------------
//   initSubStyle
//---------------------------------------------------------

void Text::initSubStyle(SubStyle s)
      {
      _subStyle = s;
      Element::initSubStyle(s);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Text::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::FONT_FACE:
                  return family();
            case P_ID::FONT_SIZE:
                  return size();
            case P_ID::FONT_BOLD:
                  return bold();
            case P_ID::FONT_ITALIC:
                  return italic();
            case P_ID::FONT_UNDERLINE:
                  return underline();
            case P_ID::FRAME:
                  return hasFrame();
            case P_ID::FRAME_SQUARE:
                  return square();
            case P_ID::FRAME_CIRCLE:
                  return circle();
            case P_ID::FRAME_WIDTH:
                  return frameWidth();
            case P_ID::FRAME_PADDING:
                  return paddingWidth();
            case P_ID::FRAME_ROUND:
                  return frameRound();
            case P_ID::FRAME_FG_COLOR:
                  return frameColor();
            case P_ID::FRAME_BG_COLOR:
                  return bgColor();
            case P_ID::FONT_SPATIUM_DEPENDENT:
                  return sizeIsSpatiumDependent();
            case P_ID::ALIGN:
                  return QVariant::fromValue(align());
            case P_ID::TEXT:
                  return xmlText();
            case P_ID::SUB_STYLE:
                  return int(subStyle());
            case P_ID::OFFSET:
                  return offset();
            case P_ID::OFFSET_TYPE:
                  return int(offsetType());
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
            case P_ID::FONT_FACE:
                  setFamily(v.toString());
                  break;
            case P_ID::FONT_SIZE:
                  setSize(v.toReal());
                  break;
            case P_ID::FONT_BOLD:
                  setBold(v.toBool());
                  break;
            case P_ID::FONT_ITALIC:
                  setItalic(v.toBool());
                  break;
            case P_ID::FONT_UNDERLINE:
                  setUnderline(v.toBool());
                  break;
            case P_ID::FRAME:
                  setHasFrame(v.toBool());
                  break;
            case P_ID::FRAME_SQUARE:
                  setSquare(v.toBool());
                  break;
            case P_ID::FRAME_CIRCLE:
                  setCircle(v.toBool());
                  break;
            case P_ID::FRAME_WIDTH:
                  setFrameWidth(v.value<Spatium>());
                  break;
            case P_ID::FRAME_PADDING:
                  setPaddingWidth(v.value<Spatium>());
                  break;
            case P_ID::FRAME_ROUND:
                  setFrameRound(v.toInt());
                  break;
            case P_ID::FRAME_FG_COLOR:
                  setFrameColor(v.value<QColor>());
                  break;
            case P_ID::FRAME_BG_COLOR:
                  setBgColor(v.value<QColor>());
                  break;
            case P_ID::FONT_SPATIUM_DEPENDENT:
                  setSizeIsSpatiumDependent(v.toBool());
                  break;
            case P_ID::TEXT:
                  setXmlText(v.toString());
                  break;
            case P_ID::ALIGN:
                  setAlign(v.value<Align>());
                  break;
            case P_ID::SUB_STYLE:
                  setSubStyle(SubStyle(v.toInt()));
                  break;
            case P_ID::OFFSET:
                  setOffset(v.toPointF());
                  break;
            case P_ID::OFFSET_TYPE:
                  setOffsetType(OffsetType(v.toInt()));
                  break;
            default:
                  rv = Element::setProperty(propertyId, v);
                  break;
            }
      triggerLayout();
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Text::propertyDefault(P_ID id) const
      {
      for (const StyledProperty& p : Ms::subStyle(_subStyle)) {
            if (p.propertyIdx == id)
                  return score()->styleV(p.styleIdx);
            }
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::DEFAULT);
            case P_ID::TEXT:
                  return QString();
            case P_ID::OFFSET:
                  return QPointF();
            case P_ID::OFFSET_TYPE:
                  return int (OffsetType::SPATIUM);
            default:
                  for (const StyledProperty& p : Ms::subStyle(SubStyle::DEFAULT)) {
                        if (p.propertyIdx == id)
                              return score()->styleV(p.styleIdx);
                        }
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Text::resetProperty(P_ID id)
      {
      PropertyFlags* p = propertyFlagsP(id);
      if (p) {
            setProperty(id, propertyDefault(id));
            *p = PropertyFlags::STYLED;
            return;
            }

      switch (id) {
            default:
                  return Element::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Text::reset()
      {
      for (const StyledProperty& p : Ms::subStyle(_subStyle))
            undoResetProperty(p.propertyIdx);
      Element::reset();
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Text::getPropertyStyle(P_ID id) const
      {
      for (auto sp : Ms::subStyle(_subStyle)) {
            if (sp.propertyIdx == id)
                  return sp.styleIdx;
            }
      return Element::getPropertyStyle(id);
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Text::styleChanged()
      {
      for (const StyledProperty& p : Ms::subStyle(_subStyle)) {
            if (propertyFlags(p.propertyIdx) == PropertyFlags::STYLED)
                  setProperty(p.propertyIdx, propertyDefault(p.propertyIdx));
            }
      Element::styleChanged();
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags Text::propertyFlags(P_ID id) const
      {
      const PropertyFlags* p = ((Text*)this)->propertyFlagsP(id); // ugh!
      if (p)
            return *p;
      return Element::propertyFlags(id);
      }

//---------------------------------------------------------
//   setPropertyFlags
//---------------------------------------------------------

void Text::setPropertyFlags(P_ID id, PropertyFlags f)
      {
      PropertyFlags* p = propertyFlagsP(id);
      if (p)
            *p = f;
      else
            Element::setPropertyFlags(id, f);
      }

//---------------------------------------------------------
//   propertyFlagsP
//---------------------------------------------------------

PropertyFlags* Text::propertyFlagsP(P_ID id)
      {
      switch (id) {
            case P_ID::FONT_FACE:
                  return &_familyStyle;
            case P_ID::FONT_SIZE:
                  return &_sizeStyle;
            case P_ID::FONT_BOLD:
                  return &_boldStyle;
            case P_ID::FONT_ITALIC:
                  return &_italicStyle;
            case P_ID::FONT_UNDERLINE:
                  return &_underlineStyle;
            case P_ID::FRAME:
                  return &_hasFrameStyle;
            case P_ID::FRAME_SQUARE:
                  return &_squareStyle;
            case P_ID::FRAME_CIRCLE:
                  return &_circleStyle;
            case P_ID::FRAME_WIDTH:
                  return &_frameWidthStyle;
            case P_ID::FRAME_PADDING:
                  return &_paddingWidthStyle;
            case P_ID::FRAME_ROUND:
                  return &_frameRoundStyle;
            case P_ID::FRAME_FG_COLOR:
                  return &_frameColorStyle;
            case P_ID::FRAME_BG_COLOR:
                  return &_bgColorStyle;
            case P_ID::FONT_SPATIUM_DEPENDENT:
                  return &_sizeIsSpatiumDependentStyle;
            case P_ID::ALIGN:
                  return &_alignStyle;
            default:
                  // qDebug("unknown id: %d %s", int(id), propertyName(id));
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   editCut
//---------------------------------------------------------

void Text::editCut(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      QString s = _cursor->selectedText();

      if (!s.isEmpty()) {
            QApplication::clipboard()->setText(s, QClipboard::Clipboard);
            ed.curGrip = Grip::START;
            ed.key     = Qt::Key_Delete;
            ed.s       = QString();
            edit(ed);
            }
      }

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void Text::editCopy(EditData& ed)
      {
      //
      // store selection as plain text
      //
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      QString s = _cursor->selectedText();
      if (!s.isEmpty())
            QApplication::clipboard()->setText(s, QClipboard::Clipboard);
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

TextCursor* Text::cursor(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      return ted->cursor;
      }

//---------------------------------------------------------
//   curFormat
//---------------------------------------------------------

CharFormat* Text::curFormat(EditData& ed)
      {
      TextCursor* _cursor = cursor(ed);
      return _cursor->format();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Text::draw(QPainter* p) const
      {
      if (hasFrame()) {
            if (frameWidth().val() != 0.0) {
                  QColor fColor = frameColor();
                  QPen pen(fColor, frameWidth().val() * spatium(), Qt::SolidLine,
                     Qt::SquareCap, Qt::MiterJoin);
                  p->setPen(pen);
                  }
            else
                  p->setPen(Qt::NoPen);
            QColor bg(bgColor());
            p->setBrush(bg.alpha() ? QBrush(bg) : Qt::NoBrush);
            if (circle())
                  p->drawEllipse(frame);
            else {
                  int r2 = frameRound();
                  if (r2 > 99)
                        r2 = 99;
                  p->drawRoundedRect(frame, frameRound(), r2);
                  }
            }
      p->setBrush(Qt::NoBrush);
      p->setPen(textColor());
      for (const TextBlock& t : _layout)
            t.draw(p, this);
      }

//---------------------------------------------------------
//   drawEditMode
//    draw edit mode decorations
//---------------------------------------------------------

void Text::drawEditMode(QPainter* p, EditData& ed)
      {
      QPointF pos(canvasPos());
      p->translate(pos);

      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      if (!ted) {
            qDebug("ted not found");
            return;
            }
      TextCursor* _cursor = ted->cursor;

      if (_cursor->hasSelection()) {
            p->setBrush(Qt::NoBrush);
            p->setPen(textColor());
            int r1 = _cursor->selectLine();
            int r2 = _cursor->line();
            int c1 = _cursor->selectColumn();
            int c2 = _cursor->column();

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
      p->setBrush(curColor());
      QPen pen(curColor());
      pen.setJoinStyle(Qt::MiterJoin);
      p->setPen(pen);
      p->drawRect(_cursor->cursorRect());

      QMatrix matrix = p->matrix();
      p->translate(-pos);
      p->setPen(QPen(QBrush(Qt::lightGray), 4.0 / matrix.m11()));  // 4 pixel pen size
      p->setBrush(Qt::NoBrush);

      qreal m = spatium();
      QRectF r = pageBoundingRect().adjusted(-m, -m, m, m);
      p->drawRect(r);
      pen = QPen(MScore::defaultColor, 0.0);
      }

//---------------------------------------------------------
//   deleteChar
//---------------------------------------------------------

bool TextCursor::deleteChar()
      {
      TextBlock& l1 = curLine();
      if (_column == l1.columns()) {
            if (_line + 1 < _text->_layout.size()) {
                  const TextBlock& l2 = _text->_layout[_line + 1];
                  for (const TextFragment& f : l2.fragments())
                        l1.fragments().append(f);
                  _text->_layout.removeAt(_line + 1);
                  if (_text->_layout.last() == l1)
                        l1.setEol(false);
                  }
            else
                  return false;
            }
      else {
            QString s = l1.remove(_column);
            printf("remove <%s>\n", qPrintable(s));
            _text->score()->undoStack()->push1(new RemoveText(this, s));
            }
      clearSelection();
      _text->triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   undoRedoInsertText
//---------------------------------------------------------

void Text::undoRedoInsertText(EditData& ed, ChangeText* ct)
      {
      TextEditData* ted   = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      *_cursor            = ct->cursor();
      QString s           = ct->string();
      score()->select(this);

// printf("Text: undoRedoInsertText <%s>\n", qPrintable(s));
      editInsertText(_cursor, s);
      triggerLayout();
      }

//---------------------------------------------------------
//   undoRedoRemoveText
//---------------------------------------------------------

void Text::undoRedoRemoveText(EditData& ed, ChangeText* ct)
      {
      TextEditData* ted   = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = ted->cursor;
      *_cursor            = ct->cursor();
      QString s           = ct->string();
      score()->select(this);

// printf("Text: undoRedoRemoveText <%s>\n", qPrintable(s));
      TextBlock& l = _cursor->curLine();
      int column   = _cursor->column();
      for (int n = 0; n < s.size(); ++n)
            l.remove(column);
      }
}

