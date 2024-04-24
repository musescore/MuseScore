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

#include "barline.h"
#include "box.h"
#include "page.h"
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "segment.h"
#include "sym.h"
#include "system.h"
#include "textedit.h"
#include "textframe.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

static const qreal subScriptSize     = 0.6;
static const qreal subScriptOffset   = 0.5;       // of x-height
static const qreal superScriptOffset = -.9;      // of x-height

//static const qreal tempotextOffset = 0.4; // of x-height // 80% of 50% = 2 spatiums

//---------------------------------------------------------
//   accessibleChar
/// Return the name of common symbols and punctuation, or return the
/// character itself if the name is unknown. For screen readers.
//---------------------------------------------------------

static QString accessibleChar(QChar chr)
      {
      if (chr == " ") return QObject::tr("space");
      if (chr == "-") return QObject::tr("dash");
      if (chr == "=") return QObject::tr("equals");
      if (chr == ",") return QObject::tr("comma");
      if (chr == ".") return QObject::tr("period");
      if (chr == ":") return QObject::tr("colon");
      if (chr == ";") return QObject::tr("semicolon");
      if (chr == "(") return QObject::tr("left parenthesis");
      if (chr == ")") return QObject::tr("right parenthesis");
      if (chr == "[") return QObject::tr("left bracket");
      if (chr == "]") return QObject::tr("right bracket");
      return chr;
      }

//---------------------------------------------------------
//   accessibleCharacter
/// Given a string, if it has one character return its name, otherwise return
/// the string. Useful to force screen readers to speak punctuation when it
/// is alone but not when it forms part of a sentence.
//---------------------------------------------------------

static QString accessibleCharacter(QString str)
      {
      if (str.length() == 1)
            return accessibleChar(str.at(0));
      return str;
      }

//---------------------------------------------------------
//   isSorted
/// return true if (r1,c1) is at or before (r2,c2)
//---------------------------------------------------------

static bool isSorted(int r1, int c1, int r2, int c2)
      {
      if (r1 < r2)
            return true;
      if ((r1 == r2) && (c1 <= c2))
            return true;
      return false;
      }

//---------------------------------------------------------
//   swap
/// swap (r1,c1) with (r2,c2)
//---------------------------------------------------------

static void swap(int& r1, int& c1, int& r2, int& c2)
      {
      qSwap(r1, r2);
      qSwap(c1, c2);
      }

//---------------------------------------------------------
//   sort
/// swap (r1,c1) with (r2,c2) if they are not sorted
//---------------------------------------------------------

static void sort(int& r1, int& c1, int& r2, int& c2)
      {
      if (!isSorted(r1, c1, r2, c2))
            swap(r1, c1, r2, c2);
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool CharFormat::operator==(const CharFormat& cf) const
      {
      return cf.style()     == style()
         && cf.preedit()    == preedit()
         && cf.valign()     == valign()
         && cf.fontSize()   == fontSize()
         && cf.fontFamily() == fontFamily();
      }

//---------------------------------------------------------
//   operator=
//---------------------------------------------------------

CharFormat& CharFormat::operator=(const CharFormat& cf)
      {
      setStyle(cf.style());
      setValign(cf.valign());
      setFontSize(cf.fontSize());
      setFontFamily(cf.fontFamily());

      return *this;
      }

//---------------------------------------------------------
//   clearSelection
//---------------------------------------------------------

void TextCursor::clearSelection()
      {
      _selectLine   = _row;
      _selectColumn = _column;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void TextCursor::init()
      {
      _format.setFontFamily(_text->family());
      _format.setFontSize(_text->size());
      _format.setStyle(_text->fontStyle());
      _format.setPreedit(false);
      _format.setValign(VerticalAlignment::AlignNormal);
      }

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int TextCursor::columns() const
      {
      return _text->textBlock(_row).columns();
      }

//---------------------------------------------------------
//   currentCharacter
//---------------------------------------------------------

QChar TextCursor::currentCharacter() const
      {
      const TextBlock& t = _text->_layout[row()];
      QString s = t.text(column(), 1);
      if (s.isEmpty())
            return QChar();
      return s[0];
      }

//---------------------------------------------------------
//   currentWord
//---------------------------------------------------------

QString TextCursor::currentWord() const
      {
      const TextBlock& t = _text->_layout[row()];
      QString s = t.text(column(), -1);
      return s.remove(QRegularExpression(" .*"));
      }

//---------------------------------------------------------
//   currentLine
//---------------------------------------------------------

QString TextCursor::currentLine() const
      {
      const TextBlock& t = _text->_layout[row()];
      return t.text(0, -1);
      }

//---------------------------------------------------------
//   updateCursorFormat
//---------------------------------------------------------

void TextCursor::updateCursorFormat()
      {
      TextBlock* block = &_text->_layout[_row];
      int col = hasSelection() ? selectColumn() : column();
      // Get format at the LEFT of the cursor position
      const CharFormat* format = block->formatAt(std::max((col) -1, 0));
      if (!format || format->fontFamily() == "ScoreText")
            init();
      else
            setFormat(*format);
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
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

TextBlock& TextCursor::curLine() const
      {
      Q_ASSERT(!_text->_layout.empty());
      return _text->_layout[_row];
      }

//---------------------------------------------------------
//   changeSelectionFormat
//---------------------------------------------------------

void TextCursor::changeSelectionFormat(FormatId id, QVariant val)
      {
      if (!hasSelection())
            return;
      int r1 = selectLine();
      int r2 = row();
      int c1 = selectColumn();
      int c2 = column();
      sort(r1, c1, r2, c2);
      int rows = _text->rows();
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
      }

//---------------------------------------------------------
//   setFormat
//---------------------------------------------------------

void TextCursor::setFormat(FormatId id, QVariant val)
      {
      changeSelectionFormat(id, val);
      format()->setFormat(id, val);
      text()->setTextInvalid();
      }

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

bool TextCursor::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode, int count)
      {
      QString accMsg;
      int oldRow = _row;
      int oldCol = _column;

      QString oldSelection;
      if (hasSelection())
            oldSelection = selectedText();

      for (int i = 0; i < count; i++) {
            switch (op) {
                  case QTextCursor::Left:
                        if (hasSelection() && mode == QTextCursor::MoveAnchor) {
                              int r1 = _selectLine;
                              int r2 = _row;
                              int c1 = _selectColumn;
                              int c2 = _column;
                              sort(r1, c1, r2, c2);
                              clearSelection();
                              _row    = r1;
                              _column = c1;
                              }
                        else if (_column == 0) {
                              if (_row == 0)
                                    return false;
                              --_row;
                              _column = curLine().columns();
                              }
                        else
                              --_column;
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCurrentCharacter();
#endif
                        break;

                  case QTextCursor::Right:
                        if (hasSelection() && mode == QTextCursor::MoveAnchor) {
                              int r1 = _selectLine;
                              int r2 = _row;
                              int c1 = _selectColumn;
                              int c2 = _column;
                              sort(r1, c1, r2, c2);
                              clearSelection();
                              _row    = r2;
                              _column = c2;
                              }
                        else if (column() >= curLine().columns()) {
                              if (_row >= _text->rows() - 1)
                                    return false;
                              ++_row;
                              _column = 0;
                              }
                        else
                              ++_column;
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCurrentCharacter();
#endif
                        break;

                  case QTextCursor::Up:
                        if (_row == 0)
                              return false;
                        --_row;
                        if (_column > curLine().columns())
                              _column = curLine().columns();
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCharacter(currentLine()) + "\n";
#endif
                        break;

                  case QTextCursor::Down:
                        if (_row >= _text->rows() - 1)
                              return false;
                        ++_row;
                        if (_column > curLine().columns())
                              _column = curLine().columns();
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCharacter(currentLine()) + "\n";
#endif
                        break;

                  case QTextCursor::Start:
                        _row    = 0;
                        _column = 0;
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg = accessibleCharacter(currentLine());
#endif
                        break;

                  case QTextCursor::End:
                        _row    = _text->rows() - 1;
                        _column = curLine().columns();
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg = accessibleCharacter(currentLine());
#endif
                        break;

                  case QTextCursor::StartOfLine:
                        _column = 0;
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg = accessibleCurrentCharacter();
#endif
                        break;

                  case QTextCursor::EndOfLine:
                        _column = curLine().columns();
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg = accessibleCurrentCharacter();
#endif
                        break;

                  case QTextCursor::WordLeft:
                        if (_column > 0) {
                              --_column;
                              while (_column > 0 && currentCharacter().isSpace())
                                    --_column;
                              while (_column > 0 && !currentCharacter().isSpace())
                                    --_column;
                              if (currentCharacter().isSpace())
                                    ++_column;
                              }
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCharacter(currentWord()) + " ";
#endif
                        break;

                  case QTextCursor::NextWord: {
                        int cols =  columns();
                        if (_column < cols) {
                              ++_column;
                              while (_column < cols && !currentCharacter().isSpace())
                                    ++_column;
                              while (_column < cols && currentCharacter().isSpace())
                                    ++_column;
                              }
                        }
#if !defined(Q_OS_MAC)
                        if (mode == QTextCursor::MoveAnchor)
                              accMsg += accessibleCharacter(currentWord()) + " ";
#endif
                        break;

                  default:
                        qDebug("Text::movePosition: not implemented");
                        return false;
                  }
            if (mode == QTextCursor::MoveAnchor)
                  clearSelection();
            }
      accessibileMessage(accMsg, oldRow, oldCol, oldSelection, mode);
      _text->score()->setAccessibleMessage(accMsg);
      updateCursorFormat();
      _text->score()->addRefresh(_text->canvasBoundingRect());
      return true;
      }



//---------------------------------------------------------
//   doubleClickSelect
//---------------------------------------------------------

void TextCursor::doubleClickSelect()
      {
      clearSelection();

      // if clicked on a space, select surrounding spaces
      // otherwise select surround non-spaces
      const bool selectSpaces = currentCharacter().isSpace();

      //handle double-clicking inside a word
      int startPosition = _column;

      while (_column > 0 && currentCharacter().isSpace() == selectSpaces)
            --_column;

      if (currentCharacter().isSpace() != selectSpaces)
            ++_column;

      _selectColumn = _column;

      _column = startPosition;
      while (_column  < curLine().columns() && currentCharacter().isSpace() == selectSpaces)
            ++_column;

      updateCursorFormat();
      _text->score()->addRefresh(_text->canvasBoundingRect());
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

bool TextCursor::set(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - _text->canvasPos();
      if (!_text->bbox().contains(pt))
            return false;

//      if (_text->_layout.empty())
//            _text->_layout.append(TextBlock());
      _row = 0;
      for (int row = 0; row < _text->rows(); ++row) {
            const TextBlock& l = _text->_layout.at(row);
            if (l.y() > pt.y()) {
                  _row = row;
                  break;
                  }
            }
      _column = curLine().column(pt.x(), _text);

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
      int r1 = selectLine();
      int r2 = _row;
      int c1 = selectColumn();
      int c2 = column();
      sort(r1, c1, r2, c2);
      return extractText(r1, c1, r2, c2);
      }

//---------------------------------------------------------
//   extractText
//    return text between (r1,c1) and (r2,c2).
//---------------------------------------------------------

QString TextCursor::extractText(int r1, int c1, int r2, int c2) const
      {
      Q_ASSERT(isSorted(r1, c1, r2, c2));
      const QList<TextBlock>& tb = _text->_layout;

      if (r1 == r2)
            return tb.at(r1).text(c1, c2 - c1);

      QString str = tb.at(r1).text(c1, -1) + "\n";

      for (int r = r1 + 1; r < r2; ++r)
            str += tb.at(r).text(0, -1) + "\n";

      str += tb.at(r2).text(0, c2);
      return str;
      }

//---------------------------------------------------------
//   accessibleCurrentCharacter
/// Return current character or its name in the case of a symbol. For screen readers.
//---------------------------------------------------------

QString TextCursor::accessibleCurrentCharacter() const
      {
      if (_column < curLine().columns())
            return accessibleChar(currentCharacter());
      if (_row < _text->rows() - 1)
            return QObject::tr("line feed");
      return QObject::tr("blank"); // end of text
      }

//---------------------------------------------------------
//   accessibileMessage
/// Set the accMsg string to describe the result of having moved the cursor
/// from P(oldRow,oldCol) to the current position. For use by screen readers.
///
/// The default message simply contains the characters that the cursor
/// skipped over when moving between the old and current positions. This
/// mimics the behavior of VoiceOver on macOS, but other screen readers do
/// things a bit differently. (Many report the character or word to the right
/// of the cursor regardless of the direction that the cursor moved, but that
/// behavior is not implemented here.) You can override the default message
/// by setting accMsg to a different value before this function is called.
///
/// If the cursor's movement caused a change in selection then the message
/// will say which characters were selected and which were deselected (both
/// can happen in a single move operation). All screen readers do this, so
/// this message cannot be overridden.
//---------------------------------------------------------

void TextCursor::accessibileMessage(QString& accMsg, int oldRow, int oldCol, QString oldSelection, QTextCursor::MoveMode mode) const
      {
      int r1 = oldRow;
      int c1 = oldCol;
      int r2 = _row;
      int c2 = _column;

      const bool movedForwards = isSorted(r1, c1, r2, c2);

      // ensure P1 before P2
      if (!movedForwards)
            swap(r1, c1, r2, c2);

      if (mode == QTextCursor::MoveAnchor) {
            if (accMsg.isEmpty()) {
                  // Provide a default message based on skipped characters.
                  accMsg = accessibleCharacter(extractText(r1, c1, r2, c2));
                  }

            if (!oldSelection.isEmpty()) {
                  // Cursor's movement has cancelled a previous selection.
                  oldSelection = QObject::tr("%1 unselected").arg(oldSelection);

                  if (accMsg.isEmpty()) // no characters were skipped
                        accMsg = oldSelection;
                  else
                        accMsg = QObject::tr("%1, %2").arg(accMsg, oldSelection);
                  }

            return;
            }

      // Skipped characters were added and/or removed from selection.
      const int rs = _selectLine;
      const int cs = _selectColumn;

      bool selectForwards;
      bool anchorOutsideRange;

      if (isSorted(rs, cs, r1, c1)) {
            // Selection anchor is before range of skipped characters.
            anchorOutsideRange = true;
            selectForwards = true; // forward movement selects characters
            }
      else if (isSorted(r2, c2, rs, cs)) {
            // Selection anchor is after range of skipped characters.
            anchorOutsideRange = true;
            selectForwards = false; // forward movement deselects characters
            }
      else {
            // Selection anchor is within the range of skipped characters
            // so some characters have been selected and others deselected.
            anchorOutsideRange = false;
            selectForwards = false;
      }

      if (anchorOutsideRange) {
            // Entire range of skipped characters was selected or deselected.
            accMsg = accessibleCharacter(extractText(r1, c1, r2, c2));

            if (movedForwards == selectForwards)
                  accMsg = QObject::tr("%1 selected").arg(accMsg);
            else
                  accMsg = QObject::tr("%1 unselected").arg(accMsg);

            return;
            }

      // cursor skipped over the selection anchor
      QString str1 = accessibleCharacter(extractText(r1, c1, rs, cs));
      QString str2 = accessibleCharacter(extractText(rs, cs, r2, c2));

      if (movedForwards) {
            str1 = QObject::tr("%1 unselected").arg(str1);
            str2 = QObject::tr("%1 selected").arg(str2);
            }
      else {
            str1 = QObject::tr("%1 selected").arg(str1);
            str2 = QObject::tr("%1 unselected").arg(str2);
            }

      accMsg =  QObject::tr("%1, %2").arg(str1, str2);
      }

//---------------------------------------------------------
//   TextFragment
//---------------------------------------------------------

TextFragment::TextFragment()
      {
      }

TextFragment::TextFragment(const QString& s)
      {
      text = s;
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

      for (const QChar& c : qAsConst(text)) {
            if (col == column) {
                  if (idx) {
                        if (idx < text.size()) {
                              f.text = text.mid(idx);
                              text   = text.left(idx);
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
      for (const QChar& c : qAsConst(text)) {
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
      return format == f.format && text == f.text;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextFragment::draw(QPainter* p, const TextBase* t) const
      {
      QFont f(font(t));
      f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
#ifndef Q_OS_MACOS
      TextBase::drawTextWorkaround(p, f, pos, text);
#else
      p->setFont(f);
      p->drawText(pos, text);
#endif
      }

//---------------------------------------------------------
//   drawTextWorkaround
//---------------------------------------------------------

void TextBase::drawTextWorkaround(QPainter* p, QFont& f, const QPointF pos, const QString text)
      {
      qreal mm = p->worldTransform().m11();
      if (!(MScore::pdfPrinting) && (mm < 1.0) && f.bold() && !(f.underline() || f.strikeOut())) {
            // workaround for https://musescore.org/en/node/284218
            // and https://musescore.org/en/node/281601
            // only needed for certain artificially emboldened fonts
            // see https://musescore.org/en/node/281601#comment-900261
            // in Qt 5.12.x this workaround should be no more necessary if
            // env variable QT_MAX_CACHED_GLYPH_SIZE is set to 1.
            // The workaround works badly if the text is at the same time
            // bold and underlined.
            p->save();
            qreal dx = p->worldTransform().dx();
            qreal dy = p->worldTransform().dy();
            // diagonal elements will now be changed to 1.0
            p->setTransform(QTransform(1.0, 0.0, 0.0, 1.0, dx, dy));
            // correction factor for bold text drawing, due to the change of the diagonal elements
            qreal factor = 1.0 / mm;
            QFont fnew(f, p->device());
            fnew.setPointSizeF(f.pointSizeF() / factor);
            QRawFont fRaw = QRawFont::fromFont(fnew);
            QTextLayout textLayout(text, f, p->device());
            textLayout.beginLayout();
            while (true) {
                  QTextLine line = textLayout.createLine();
                  if (!line.isValid())
                        break;
                  }
            textLayout.endLayout();
            // glyphruns with correct positions, but potentially wrong glyphs
            // (see bug https://musescore.org/en/node/117191 regarding positions and DPI)
            QList<QGlyphRun> glyphruns = textLayout.glyphRuns();
            qreal offset = 0;
            // glyphrun drawing has an offset equal to the max ascent of the text fragment
            for (int i = 0; i < glyphruns.length(); i++) {
                  qreal value = glyphruns.at(i).rawFont().ascent() / factor;
                  if (value > offset)
                        offset = value;
                  }
            for (int i = 0; i < glyphruns.length(); i++) {
                  QVector<QPointF> positions1 = glyphruns.at(i).positions();
                  QVector<QPointF> positions2;
                  // calculate the new positions for the scaled geometry
                  for (int j = 0; j < positions1.length(); j++) {
                        QPointF newPoint = positions1.at(j) / factor;
                        positions2.append(newPoint);
                        }
                  QGlyphRun glyphrun2 = glyphruns.at(i);
                  glyphrun2.setPositions(positions2);
                  // change the glyphs with the correct glyphs
                  // and account for glyph substitution
                  if (glyphrun2.rawFont().familyName() != fnew.family()) {
                        QFont f2(fnew);
                        f2.setFamily(glyphrun2.rawFont().familyName());
                        glyphrun2.setRawFont(QRawFont::fromFont(f2));
                        }
                  else
                        glyphrun2.setRawFont(fRaw);
                  p->drawGlyphRun(QPointF(pos.x() / factor, pos.y() / factor - offset),glyphrun2);
                  positions2.clear();
                  }
            // Restore the QPainter to its former state
            p->setTransform(QTransform(mm, 0.0, 0.0, mm, dx, dy));
            p->restore();
            }
      else {
            p->setFont(f);
            p->drawText(pos, text);
            }
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextFragment::font(const TextBase* t) const
      {
      QFont font;

      qreal m = format.fontSize();

      if (t->sizeIsSpatiumDependent())
            m *= t->spatium() / SPATIUM20;
      if (format.valign() != VerticalAlignment::AlignNormal)
            m *= subScriptSize;

      QString family;
      if (format.fontFamily() == "ScoreText") {
            family = t->score()->styleSt(Sid::MusicalTextFont);

            // check if all symbols are available
            font.setFamily(family);
            QFontMetricsF fm(font);

            bool fail = false;
            for (int i = 0; i < text.size(); ++i) {
                  QChar c = text[i];
                  if (c.isHighSurrogate()) {
                        if (i+1 == text.size())
                              qFatal("bad string");
                        QChar c2 = text[i+1];
                        ++i;
                        uint v = QChar::surrogateToUcs4(c, c2);
                        if (!fm.inFontUcs4(v)) {
                              fail = true;
                              break;
                              }
                        }
                  else {
                        if (!fm.inFont(c)) {
                              fail = true;
                              break;
                              }
                        }
                  }
            if (fail)
                  family = ScoreFont::fallbackTextFont();
            }
      else {
            family = format.fontFamily();

            font.setBold(format.bold());
            font.setItalic(format.italic());
            font.setUnderline(format.underline());
            font.setStrikeOut(format.strike());
            }

      font.setFamily(family);
      Q_ASSERT(m > 0.0);

      font.setPointSizeF(m * t->mag());
      return font;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextBlock::draw(QPainter* p, const TextBase* t) const
      {
      p->translate(0.0, _y);
      for (const TextFragment& f : _fragments)
            f.draw(p, t);
      p->translate(0.0, -_y);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBlock::layout(TextBase* t)
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
                        Box* b = toBox(e);
                        layoutWidth -= ((b->leftMargin() + b->rightMargin()) * DPMM);
                        lm = b->leftMargin() * DPMM;
                        }
                        break;
                  case ElementType::PAGE: {
                        Page* p = toPage(e);
                        layoutWidth -= (p->lm() + p->rm());
                        lm = p->lm();
                        }
                        break;
                  case ElementType::MEASURE: {
                        // ignore courtesy keysig, timesig, but fall back if needed
                        Measure* m = toMeasure(e);
                        const BarLine* bl = m->endBarLine();
                        layoutWidth = bl ? bl->segment()->x() + bl->width() : m->bbox().width();
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
      else if (_fragments.size() == 1 && _fragments.at(0).text.isEmpty()) {
            auto fi = _fragments.begin();
            TextFragment& f = *fi;
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
            else {
                  f.pos.setY(0.0);
                  }

            QRectF temp(0.0, -fm.ascent(), 1.0, fm.descent());
            _bbox |= temp;
            _lineSpacing = qMax(_lineSpacing, fm.lineSpacing());
            }
      else {
            const auto fiLast = --_fragments.end();
            for (auto fi = _fragments.begin(); fi != _fragments.end(); ++fi) {
                  TextFragment& f = *fi;
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
                  else {
                        f.pos.setY(0.0);
                        }

                  // Optimization: don't calculate character position
                  // for the next fragment if there is no next fragment
                  if (fi != fiLast) {
                        const qreal w  = fm.width(f.text);
                        x += w;
                        }

                  _bbox   |= fm.tightBoundingRect(f.text).translated(f.pos);
                  _lineSpacing = qMax(_lineSpacing, fm.lineSpacing());
                  }
            }

      // Apply style/custom line spacing
      _lineSpacing *= t->textLineSpacing();

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
//   fragmentsWithoutEmpty
//---------------------------------------------------------

QList<TextFragment>* TextBlock::fragmentsWithoutEmpty()
      {
      QList<TextFragment>* list = new QList<TextFragment>();
      for (const auto &x :qAsConst(_fragments)) {
            if (x.text.isEmpty()) {
                  continue;
                  }
            else {
                  list->append(x);
                  }
            }
      return list;
      }

//---------------------------------------------------------
//   xpos
//---------------------------------------------------------

qreal TextBlock::xpos(int column, const TextBase* t) const
      {
      int col = 0;
      for (const TextFragment& f : _fragments) {
            if (column == col)
                  return f.pos.x();
            QFontMetricsF fm(f.font(t), MScore::paintDevice());
            int idx = 0;
            for (const QChar& c : qAsConst(f.text)) {
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
            for (const QChar& c : qAsConst(f->text)) {
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

QRectF TextBlock::boundingRect(int col1, int col2, const TextBase* t) const
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
            for (const QChar& c : qAsConst(f.text)) {
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

int TextBlock::column(qreal x, TextBase* t) const
      {
      int col = 0;
      for (const TextFragment& f : _fragments) {
            int idx = 0;
            if (x <= f.pos.x())
                  return col;
            qreal px = 0.0;
            for (const QChar& c : qAsConst(f.text)) {
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
      removeEmptyFragment(); // since we are going to write text, we don't need an empty fragment to hold format info. if such exists, delete it
      auto i = fragment(cursor->column(), &rcol, &ridx);
      if (i != _fragments.end()) {
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
            if (!_fragments.empty() && _fragments.back().format == *cursor->format())
                  _fragments.back().text.append(s);
            else
                  _fragments.append(TextFragment(cursor, s));
            }
      }

//---------------------------------------------------------
//
//   insertEmptyFragmentIfNeeded
//   used to insert an empty TextFragment in TextBlocks that have none
//   that way, the formatting information (most importantly the font size) of the line is preserved
//
//---------------------------------------------------------

void TextBlock::insertEmptyFragmentIfNeeded(TextCursor* cursor)
      {
      if (_fragments.size() == 0 || _fragments.at(0).text.isEmpty()) {
            _fragments.insert(0, TextFragment(cursor, ""));
            }
      }

//---------------------------------------------------------
//   removeEmptyFragment
//---------------------------------------------------------

void TextBlock::removeEmptyFragment()
      {
      if (_fragments.size() > 0 && _fragments.at(0).text.isEmpty())
            _fragments.removeAt(0);
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
            for (const QChar& c : qAsConst(i->text)) {
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

QString TextBlock::remove(int column, TextCursor* cursor)
      {
      int col = 0;
      QString s;
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
            int idx  = 0;
            for (const QChar& c : qAsConst(i->text)) {
                  if (col == column) {
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
                        simplify();
                        insertEmptyFragmentIfNeeded(cursor); // without this, cursorRect can't calculate the y position of the cursor correctly
                        return s;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  }
            }
      insertEmptyFragmentIfNeeded(cursor); // without this, cursorRect can't calculate the y position of the cursor correctly
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

QString TextBlock::remove(int start, int n, TextCursor* cursor)
      {
      if (n == 0)
            return QString();
      int col = 0;
      QString s;
      for (auto i = _fragments.begin(); i != _fragments.end();) {
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
                        if (i->text.isEmpty() && (_fragments.size() > 1)) {
                              i = _fragments.erase(i);
                              inc = false;
                              }
                        --n;
                        if (n == 0) {
                              insertEmptyFragmentIfNeeded(cursor); // without this, cursorRect can't calculate the y position of the cursor correctly
                              return s;
                              }
                        continue;
                        }
                  ++idx;
                  if (c.isHighSurrogate())
                        continue;
                  ++col;
                  }
            if (inc)
                  ++i;
            }
      insertEmptyFragmentIfNeeded(cursor); // without this, cursorRect can't calculate the y position of the cursor correctly
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
                  setBold(data.toBool());
                  break;
            case FormatId::Italic:
                  setItalic(data.toBool());
                  break;
            case FormatId::Underline:
                  setUnderline(data.toBool());
                  break;
            case FormatId::Strike:
                  setStrike(data.toBool());
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

TextBlock TextBlock::split(int column, Ms::TextCursor* cursor)
      {
      TextBlock tl;

      int col = 0;
      for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
            int idx = 0;
            for (const QChar& c : qAsConst(i->text)) {
                  if (col == column) {
                        if (idx) {
                              if (idx < i->text.size()) {
                                    TextFragment tf(i->text.mid(idx));
                                    tf.format = i->format;
                                    tl._fragments.append(tf);
                                    i->text = i->text.left(idx);
                                    ++i;
                                    }
                              }
                        for (; i != _fragments.end(); i = _fragments.erase(i))
                              tl._fragments.append(*i);
                        if (_fragments.size() == 0)
                              insertEmptyFragmentIfNeeded(cursor);
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
      else if (_fragments.size() == 0)
            insertEmptyFragmentIfNeeded(cursor);
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
      for (const auto &f : _fragments) {
            if (f.text.isEmpty())
                  continue;
            for (const QChar& c : qAsConst(f.text)) {
                  if (col >= col1 && (len < 0 || ((col-col1) < len)))
                        s += XmlWriter::xmlString(c.unicode());
                  if (!c.isHighSurrogate())
                        ++col;
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

TextBase::TextBase(Score* s, Tid tid, ElementFlags f)
   : Element(s, f | ElementFlag::MOVABLE)
      {
      _tid                    = tid;
      _family                 = "Edwin";
      _size                   = 10.0;
      _textLineSpacing        = 1.0;
      _fontStyle              = FontStyle::Normal;
      _bgColor                = QColor(255, 255, 255, 0);
      _frameColor             = QColor(0, 0, 0, 255);
      _align                  = Align::LEFT;
      _frameType              = FrameType::NO_FRAME;
      _frameWidth             = Spatium(0.1);
      _paddingWidth           = Spatium(0.2);
      _frameRound             = 0;
      }

TextBase::TextBase(Score* s, ElementFlags f)
   : TextBase(s, Tid::DEFAULT, f)
      {
      }

TextBase::TextBase(const TextBase& st)
   : Element(st)
      {
      _text                        = st._text;
      textInvalid                  = st.textInvalid;
      layoutInvalid                = st.layoutInvalid;
      _layout                      = st._layout;
      frame                        = st.frame;
      _layoutToParentWidth         = st._layoutToParentWidth;
      hexState                     = -1;

      _tid                         = st._tid;
      _family                      = st._family;
      _size                        = st._size;
      _textLineSpacing             = st._textLineSpacing;
      _fontStyle                   = st._fontStyle;
      _bgColor                     = st._bgColor;
      _frameColor                  = st._frameColor;
      _align                       = st._align;
      _frameType                   = st._frameType;
      _frameWidth                  = st._frameWidth;
      _paddingWidth                = st._paddingWidth;
      _frameRound                  = st._frameRound;

      size_t n = _elementStyle->size() + TEXT_STYLE_SIZE;
      delete[] _propertyFlagsList;
      _propertyFlagsList = new PropertyFlags[n];
      for (size_t i = 0; i < n; ++i)
            _propertyFlagsList[i] = st._propertyFlagsList[i];
      _links = 0;
      }

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void TextBase::drawSelection(QPainter* p, const QRectF& r) const
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
//   textColor
//---------------------------------------------------------

QColor TextBase::textColor() const
      {
      return curColor();
      }

//---------------------------------------------------------
//   insert
//    insert character
//---------------------------------------------------------

void TextBase::insert(TextCursor* cursor, uint code)
      {
      if (cursor->row() >= rows())
            _layout.append(TextBlock());
      if (code == '\t')
            code = ' ';

      QString s;
      if (QChar::requiresSurrogates(code))
            s = QString(QChar(QChar::highSurrogate(code))).append(QChar(QChar::lowSurrogate(code)));
      else
            s = QString(code);
      _layout[cursor->row()].insert(cursor, s);

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

void TextBase::createLayout()
      {
      // reset all previous formatting information
      _layout.clear();
      TextCursor cursor(this);
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
                  else if (c == '\n') {
                        if (rows() <= cursor.row())
                              _layout.append(TextBlock());
                        if(_layout[cursor.row()].fragments().size() == 0)
                              _layout[cursor.row()].insertEmptyFragmentIfNeeded(&cursor); // used to preserve the Font size of the line (font info is held in TextFragments, see PR #5881)
                        _layout[cursor.row()].setEol(true);
                        cursor.setRow(cursor.row() + 1);
                        cursor.setColumn(0);
                        if (rows() <= cursor.row())
                              _layout.append(TextBlock());
                        if (_layout[cursor.row()].fragments().size() == 0)
                              _layout[cursor.row()].insertEmptyFragmentIfNeeded(&cursor); // an empty fragment may be needed on either side of the newline
                        }
                  else {
                        if (symState)
                              sym += c;
                        else {
                              if (c.isHighSurrogate()) {
                                    i++;
                                    Q_ASSERT(i < _text.length());
                                    insert(&cursor, QChar::surrogateToUcs4(c, _text[i]));
                                    }
                              else
                                    insert(&cursor, c.unicode());
                              }
                        }
                  }
            else if (state == 1) {
                  if (c == '>') {
                        bool unstyleFontStyle = false;
                        state = 0;
                        if (token == "b") {
                              cursor.format()->setBold(true);
                              unstyleFontStyle = true;
                              }
                        else if (token == "/b")
                              cursor.format()->setBold(false);
                        else if (token == "i") {
                              cursor.format()->setItalic(true);
                              unstyleFontStyle = true;
                              }
                        else if (token == "/i")
                              cursor.format()->setItalic(false);
                        else if (token == "u") {
                              cursor.format()->setUnderline(true);
                              unstyleFontStyle = true;
                              }
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
                              SymId id = Sym::name2id(sym);
                              if (id != SymId::noSym) {
                                    CharFormat fmt = *cursor.format();  // save format
                                    // uint code = score()->scoreFont()->sym(id).code();
                                    uint code = ScoreFont::fallbackFont()->sym(id).code();
                                    cursor.format()->setFontFamily("ScoreText");
                                    cursor.format()->setBold(false);
                                    cursor.format()->setItalic(false);
                                    insert(&cursor, code);
                                    cursor.setFormat(fmt);  // restore format
                                    }
                              else {
                                    qDebug("unknown symbol <%s>", qPrintable(sym));
                                    }
                              }
                        else if (token.startsWith("font ")) {
                              token = token.mid(5);
                              if (token.startsWith("size=\"")) {
                                    cursor.format()->setFontSize(parseNumProperty(token.mid(6)));
                                    setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
                                    }
                              else if (token.startsWith("face=\"")) {
                                    QString face = parseStringProperty(token.mid(6));
                                    face = unEscape(face);
                                    cursor.format()->setFontFamily(face);
                                    setPropertyFlags(Pid::FONT_FACE, PropertyFlags::UNSTYLED);
                                    }
                              else
                                    qDebug("cannot parse html property <%s> in text <%s>",
                                       qPrintable(token), qPrintable(_text));
                              }
                        if (unstyleFontStyle)
                              setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
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
                        else {
                              // TODO insert(&cursor, Sym::name2id(token));
                              }
                        }
                  else
                        token += c;
                  }
            }
      if (_layout.empty())
            _layout.append(TextBlock());
      layoutInvalid = false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBase::layout()
      {
      setPos(QPointF());
      if (!parent())
            setOffset(0.0, 0.0);
//      else if (isStyled(Pid::OFFSET))                                   // TODO: should be set already
//            setOffset(propertyDefault(Pid::OFFSET).toPointF());
      if (placeBelow())
            rypos() = staff() ? staff()->height() : 0.0;
      layout1();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TextBase::layout1()
      {
      if (layoutInvalid)
            createLayout();
      if (_layout.empty())
            _layout.append(TextBlock());
      QRectF bb;
      qreal y = 0;

      // adjust the bounding box for the text item
      for (int i = 0; i < rows(); ++i) {
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

                        if (b->height() < bb.bottom())
                              h = b->height() / 2 + bb.height();
                        else
                              h  = b->height() - yoff - b->bottomMargin() * DPMM;
                        }
                  else if (parent()->isPage()) {
                        Page* p = toPage(parent());
                        h = p->height() - p->tm() - p->bm();
                        yoff = _layoutRelativeToBottom ? -p->bm() : p->tm(); // used to keep footers inside page margins
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
      else if (align() & Align::VCENTER) {
            yoff +=  (h - (bb.top() + bb.bottom())) * .5;
            }
      else if (align() & Align::BASELINE)
            yoff += h * .5 - _layout.front().lineSpacing();
      else
            yoff += -bb.top();

      for (TextBlock& t : _layout)
            t.setY(t.y() + yoff);

      bb.translate(0.0, yoff);

      setbbox(bb);
      if (hasFrame())
            layoutFrame();
      score()->addRefresh(canvasBoundingRect());
      }

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void TextBase::layoutFrame()
      {
//      if (empty()) {    // or bbox.width() <= 1.0
      if (bbox().width() <= 1.0 || bbox().height() < 1.0) {    // or bbox.width() <= 1.0
            // this does not work for Harmony:
            QFontMetricsF fm = QFontMetricsF(font(), MScore::paintDevice());
            qreal ch = fm.ascent();
            qreal cw = fm.width('n');
            frame = QRectF(0.0, -ch, cw, ch);
            }
      else
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
#else
            // make sure width >= height
            if (frame.height() > frame.width()) {
                  qreal w = frame.height() - frame.width();
                  frame.adjust(-w * .5, 0.0, w * .5, 0.0);
                  }
#endif
            }
      else if (circle()) {
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

qreal TextBase::lineSpacing() const
      {
      return fontMetrics().lineSpacing() * MScore::pixelRatio;
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal TextBase::lineHeight() const
      {
      return fontMetrics().height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal TextBase::baseLine() const
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
      void pushS() { pushToken("s"); }

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
            for (const QString& s : qAsConst(ps))
                  pushToken(s);
            }
      void popB() { popToken("b"); }
      void popI() { popToken("i"); }
      void popU() { popToken("u"); }
      void popS() { popToken("s"); }
      };

//---------------------------------------------------------
//   genText
//---------------------------------------------------------

void TextBase::genText() const
      {
      _text.clear();
      bool bold_      = false;
      bool italic_    = false;
      bool underline_ = false;
      bool strike_    = false;

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (!f.format.bold() && bold())
                        bold_ = true;
                  if (!f.format.italic() && italic())
                        italic_ = true;
                  if (!f.format.underline() && underline())
                        underline_ = true;
                  if (!f.format.strike() && strike())
                        strike_ = true;
                  }
            }
      CharFormat fmt;
      fmt.setFontFamily(family());
      fmt.setFontSize(size());
      fmt.setStyle(fontStyle());
      fmt.setPreedit(false);
      fmt.setValign(VerticalAlignment::AlignNormal);

      XmlNesting xmlNesting(&_text);
      if (bold_)
            xmlNesting.pushB();
      if (italic_)
            xmlNesting.pushI();
      if (underline_)
            xmlNesting.pushU();
      if (strike_)
            xmlNesting.pushS();

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  // don't skip, empty text fragments hold information for empty lines
//                  if (f.text.isEmpty())                     // skip empty fragments, not to
//                        continue;                           // insert extra HTML formatting
                  const CharFormat& format = f.format;
                  if (fmt.bold() != format.bold()) {
                        if (format.bold())
                              xmlNesting.pushB();
                        else
                              xmlNesting.popB();
                        }
                  if (fmt.italic() != format.italic()) {
                        if (format.italic())
                              xmlNesting.pushI();
                        else
                              xmlNesting.popI();
                        }
                  if (fmt.underline() != format.underline()) {
                        if (format.underline())
                              xmlNesting.pushU();
                        else
                              xmlNesting.popU();
                        }
                  if (fmt.strike() != format.strike()) {
                        if (format.strike())
                              xmlNesting.pushS();
                        else
                              xmlNesting.popS();
                        }

                  if (format.fontSize() != fmt.fontSize())
                        _text += QString("<font size=\"%1\"/>").arg(format.fontSize());
                  if (format.fontFamily() != fmt.fontFamily())
                        _text += QString("<font face=\"%1\"/>").arg(TextBase::escape(format.fontFamily()));

                  VerticalAlignment va = format.valign();
                  VerticalAlignment cva = fmt.valign();
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
                  _text += XmlWriter::xmlString(f.text);
                  fmt = format;
                  }
            if (block.eol())
                  _text += QChar::LineFeed;
            }
      while (!xmlNesting.empty())
            xmlNesting.popToken();
      textInvalid = false;
      }

//---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void TextBase::selectAll(TextCursor* _cursor)
      {
      _cursor->setSelectLine(0);
      _cursor->setSelectColumn(0);
      _cursor->setRow(rows() - 1);
      _cursor->setColumn(_cursor->curLine().columns());
      }

//---------------------------------------------------------
//   multiClickSelect
//    for double and triple clicks
//---------------------------------------------------------

void TextBase::multiClickSelect(EditData& editData, MultiClick clicks)
      {
      switch (clicks) {
            case MultiClick::Double:
                  cursor(editData)->doubleClickSelect();
                  break;
            case MultiClick::Triple:
                  selectAll(cursor(editData));
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextBase::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      writeProperties(xml, true, true);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextBase::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextBase::writeProperties(XmlWriter& xml, bool writeText, bool /*writeStyle*/) const
      {
      Element::writeProperties(xml);
      writeProperty(xml, Pid::SUB_STYLE);

      for (const StyledProperty& spp : *_elementStyle) {
            if (!isStyled(spp.pid))
                  writeProperty(xml, spp.pid);
            }
      for (const StyledProperty& spp : *textStyle(tid())) {
            if (!isStyled(spp.pid))
                  writeProperty(xml, spp.pid);
            }
      if (writeText)
            xml.writeXml("text", xmlText());
      }

static constexpr std::array<Pid, 18> pids { {
      Pid::SUB_STYLE,
      Pid::FONT_FACE,
      Pid::FONT_SIZE,
      Pid::TEXT_LINE_SPACING,
      Pid::FONT_STYLE,
      Pid::COLOR,
      Pid::FRAME_TYPE,
      Pid::FRAME_WIDTH,
      Pid::FRAME_PADDING,
      Pid::FRAME_ROUND,
      Pid::FRAME_FG_COLOR,
      Pid::FRAME_BG_COLOR,
      Pid::ALIGN,
      } };

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextBase::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      for (Pid i :pids) {
            if (readProperty(tag, e, i))
                  return true;
            }
      if (tag == "text")
            setXmlText(e.readXml());
      else if (tag == "bold") {
            bool val = e.readInt();
            setBold(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "italic") {
            bool val = e.readInt();
            setItalic(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "underline") {
            bool val = e.readInt();
            setUnderline(val);            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (tag == "strike") {
            bool val = e.readInt();
            setStrike(val);
            if (isStyled(Pid::FONT_STYLE))
                  setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid TextBase::propertyId(const QStringRef& name) const
      {
      if (name == "text")
            return Pid::TEXT;
      for (Pid pid : pids) {
            if (propertyName(pid) == name)
                  return pid;
            }
      return Element::propertyId(name);
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF TextBase::pageRectangle() const
      {
      if (parent() && (parent()->isHBox() || parent()->isVBox() || parent()->isTBox())) {
            Box* box = toBox(parent());
            QRectF r = box->abbox();
            qreal x = r.x() + box->leftMargin() * DPMM;
            qreal y = r.y() + box->topMargin() * DPMM;
            qreal h = r.height() - (box->topMargin() + box->bottomMargin()) * DPMM;
            qreal w = r.width()  - (box->leftMargin() + box->rightMargin()) * DPMM;

            // QSizeF ps = _doc->pageSize();
            // return QRectF(x, y, ps.width(), ps.height());

            return QRectF(x, y, w, h);
            }
      if (parent() && parent()->isPage()) {
            Page* box  = toPage(parent());
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

void TextBase::dragTo(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;
      _cursor->set(ed.pos, QTextCursor::KeepAnchor);
      score()->setUpdateAll();
      score()->update();
      }

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<QLineF> TextBase::dragAnchorLines() const
      {
      QVector<QLineF> result(genericDragAnchorLines());

      if (layoutToParentWidth() && !result.empty()) {
            QLineF& line = result[0];
            line.setP2(line.p2() + bbox().topLeft());
            }

      return result;
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool TextBase::mousePress(EditData& ed)
      {
      bool shift = ed.modifiers & Qt::ShiftModifier;
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      if (!ted->cursor.set(ed.startMove, shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor))
            return false;
      if (ed.buttons == Qt::MidButton)
            paste(ed);
      score()->setUpdateAll();
      return true;
      }

//---------------------------------------------------------
//   layoutEdit
//---------------------------------------------------------

void TextBase::layoutEdit()
      {
      layout();
      if (parent() && parent()->type() == ElementType::TBOX) {
            TBox* tbox = toTBox(parent());
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

bool TextBase::acceptDrop(EditData& data) const
      {
      // do not accept the drop if this text element is not being edited
      ElementEditData* eed = data.getData(this);
      if (!eed || eed->type() != EditDataType::TextEditData)
            return false;
      ElementType type = data.dropElement->type();
      return type == ElementType::SYMBOL || type == ElementType::FSYMBOL;
      }

//---------------------------------------------------------
//   setXmlText
//---------------------------------------------------------

void TextBase::setXmlText(const QString& s)
      {
      _text = s;
      layoutInvalid = true;
      textInvalid   = false;
      }

//---------------------------------------------------------
//   fragmentList
//---------------------------------------------------------

/*
 Return the text as a single list of TextFragment
 Used by the MusicXML formatted export to avoid parsing the xml text format
 */

QList<TextFragment> TextBase::fragmentList() const
      {
      QList<TextFragment> res;

      const TextBase* text = this;
      std::unique_ptr<TextBase> tmpText;
      if (layoutInvalid) {
            // Create temporary text object to avoid side effects
            // of createLayout() call.
            tmpText.reset(toTextBase(this->clone()));
            tmpText->createLayout();
            text = tmpText.get();
            }
      for (const TextBlock& block : text->_layout) {
            for (const TextFragment& f : block.fragments()) {
                  /* TODO TBD
                  if (f.text.empty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                   */
                  res.append(f);
                  if (block.eol()) {
                        // simply append a newline
                        res.last().text += "\n";
                        }
                  }
            }
      return res;
      }

//---------------------------------------------------------
//   plainText
//    return plain text with symbols
//---------------------------------------------------------

QString TextBase::plainText() const
      {
      QString s;

      const TextBase* text = this;
      std::unique_ptr<TextBase> tmpText;
      if (layoutInvalid) {
            // Create temporary text object to avoid side effects
            // of createLayout() call.
            tmpText.reset(toTextBase(this->clone()));
            tmpText->createLayout();
            text = tmpText.get();
            }

      for (const TextBlock& block : text->_layout) {
            for (const TextFragment& f : block.fragments())
                  s += f.text;
            if (block.eol())
                  s += QChar::LineFeed;
            }
      return s;
      }

//---------------------------------------------------------
//   xmlText
//---------------------------------------------------------

QString TextBase::xmlText() const
      {
#if 1
      // this is way too expensive
      // what side effects has genText() ?
      // this method is const by design

      const TextBase* text = this;
      std::unique_ptr<TextBase> tmpText;
      if (textInvalid) {
            // Create temporary text object to avoid side effects
            // of genText() call.
            tmpText.reset(toTextBase(this->clone()));
            tmpText->genText();
            text = tmpText.get();
            }
      return text->_text;
#else
      if (textInvalid)
            genText();
      return _text;
#endif
      }

//---------------------------------------------------------
//   convertFromHtml
//---------------------------------------------------------

QString TextBase::convertFromHtml(const QString& ss) const
      {
      QTextDocument doc;
      doc.setHtml(ss);

      QString s;
      qreal size_ = size();
      QString family_ = family();
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
                        if (fabs(size_ - htmlSize) > 0.1) {
                              size_ = htmlSize;
                              s += QString("<font size=\"%1\"/>").arg(size_);
                              }
                        if (family_ != font.family()) {
                              family_ = font.family();
                              s += QString("<font face=\"%1\"/>").arg(family_);
                              }
                        if (font.bold())
                              s += "<b>";
                        if (font.italic())
                              s += "<i>";
                        if (font.underline())
                              s += "<u>";
                        if (font.strikeOut())
                              s += "<s>";
                        s += f.text().toHtmlEscaped();
                        if (font.strikeOut())
                              s += "</s>";
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

QString TextBase::convertToHtml(const QString& s, const TextStyle& /*st*/)
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

QString TextBase::tagEscape(QString s)
      {
      QStringList tags = { "sym", "b", "i", "u", "sub", "sup" };
      for (const QString &tag : tags) {
            QString openTag = "<" + tag + ">";
            QString openProxy = "!!" + tag + "!!";
            QString closeTag = "</" + tag + ">";
            QString closeProxy = "!!/" + tag + "!!";
            s.replace(openTag, openProxy);
            s.replace(closeTag, closeProxy);
            }
      s = XmlWriter::xmlString(s);
      for (const QString &tag : tags) {
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

QString TextBase::unEscape(QString s)
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

QString TextBase::escape(QString s)
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

QString TextBase::accessibleInfo() const
      {
      QString rez;
      switch (tid()) {
            case Tid::TITLE:
            case Tid::SUBTITLE:
            case Tid::COMPOSER:
            case Tid::POET:
            case Tid::TRANSLATOR:
            case Tid::MEASURE_NUMBER:
            case Tid::MMREST_RANGE:
                  rez = score() ? score()->getTextStyleUserName(tid()) : textStyleUserName(tid());
                  break;
            default:
                  rez = Element::accessibleInfo();
                  break;
            }
      QString s = plainText().simplified();
      if (s.length() > 20) {
            s.truncate(20);
            s += "…";
            }
      return  QString("%1: %2").arg(rez, s);
      }

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString TextBase::screenReaderInfo() const
      {
      QString rez;

      switch (tid()) {
            case Tid::TITLE:
            case Tid::SUBTITLE:
            case Tid::COMPOSER:
            case Tid::POET:
            case Tid::TRANSLATOR:
            case Tid::MEASURE_NUMBER:
            case Tid::MMREST_RANGE:
                  rez = score() ? score()->getTextStyleUserName(tid()) : textStyleUserName(tid());
                  break;
            default:
                  rez = Element::accessibleInfo();
                  break;
            }
      QString s = plainText().simplified();
      return  QString("%1: %2").arg(rez, s);
      }

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int TextBase::subtype() const
      {
      return int(tid());
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString TextBase::subtypeName() const
      {
      return score() ? score()->getTextStyleUserName(tid()) : textStyleUserName(tid());
      }

//---------------------------------------------------------
//   validateText
//    check if s is a valid musescore xml text string
//    - simple bugs are automatically adjusted
//   return true if text is valid or could be fixed
//  (this is incomplete/experimental)
//---------------------------------------------------------

bool TextBase::validateText(QString& s)
      {
      QString d;
      for (int i = 0; i < s.size(); ++i) {
            QChar c = s[i];
            if (c == '&') {
                  const char* ok[] { "amp;", "lt;", "gt;", "quot;" };
                  QString t = s.mid(i+1);
                  bool found = false;
                  for (auto k : ok) {
                        if (t.startsWith(k)) {
                              d.append(c);
                              d.append(k);
                              i += int(strlen(k));
                              found = true;
                              break;
                              }
                        }
                  if (!found)
                        d.append("&amp;");
                  }
            else if (c == '<') {
                  const char* ok[] { "b>", "/b>", "i>", "/i>", "u>", "/u", "s>", "/s>", "font ", "/font>", "sym>", "/sym>", "sub>", "/sub>", "sup>", "/sup>" };
                  QString t = s.mid(i+1);
                  bool found = false;
                  for (auto k : ok) {
                        if (t.startsWith(k)) {
                              d.append(c);
                              d.append(k);
                              i += int(strlen(k));
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
      XmlReader xml(ss);
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
//   font
//---------------------------------------------------------

QFont TextBase::font() const
      {
      qreal m = _size;
      if (sizeIsSpatiumDependent())
            m *= spatium() / SPATIUM20;
      QFont f(_family, m, bold() ? QFont::Bold : QFont::Normal, italic());
      if (underline())
            f.setUnderline(underline());
      if (strike())
            f.setStrikeOut(strike());

      return f;
      }

//---------------------------------------------------------
//   fontMetrics
//---------------------------------------------------------

QFontMetricsF TextBase::fontMetrics() const
      {
      return QFontMetricsF(font());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextBase::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SUB_STYLE:
                  return int(tid());
            case Pid::FONT_FACE:
                  return family();
            case Pid::FONT_SIZE:
                  return size();
            case Pid::FONT_STYLE:
                  return int(fontStyle());
            case Pid::TEXT_LINE_SPACING:
                return textLineSpacing();
            case Pid::FRAME_TYPE:
                  return int(frameType());
            case Pid::FRAME_WIDTH:
                  return frameWidth();
            case Pid::FRAME_PADDING:
                  return paddingWidth();
            case Pid::FRAME_ROUND:
                  return frameRound();
            case Pid::FRAME_FG_COLOR:
                  return frameColor();
            case Pid::FRAME_BG_COLOR:
                  return bgColor();
            case Pid::ALIGN:
                  return QVariant::fromValue(align());
            case Pid::TEXT:
                  return xmlText();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextBase::setProperty(Pid pid, const QVariant& v)
      {
      if (textInvalid)
            genText();
      bool rv = true;
      switch (pid) {
            case Pid::SUB_STYLE:
                  initTid(Tid(v.toInt()));
                  break;
            case Pid::FONT_FACE:
                  setFamily(v.toString());
                  break;
            case Pid::FONT_SIZE:
                  setSize(v.toReal());
                  break;
            case Pid::FONT_STYLE:
                  setFontStyle(FontStyle(v.toInt()));
                  break;
            case Pid::TEXT_LINE_SPACING:
                  setTextLineSpacing(v.toReal());
                  break;
            case Pid::FRAME_TYPE:
                  setFrameType(FrameType(v.toInt()));
                  break;
            case Pid::FRAME_WIDTH:
                  setFrameWidth(v.value<Spatium>());
                  break;
            case Pid::FRAME_PADDING:
                  setPaddingWidth(v.value<Spatium>());
                  break;
            case Pid::FRAME_ROUND:
                  setFrameRound(v.toInt());
                  break;
            case Pid::FRAME_FG_COLOR:
                  setFrameColor(v.value<QColor>());
                  break;
            case Pid::FRAME_BG_COLOR:
                  setBgColor(v.value<QColor>());
                  break;
            case Pid::TEXT:
                  setXmlText(v.toString());
                  break;
            case Pid::ALIGN:
                  setAlign(v.value<Align>());
                  break;
            default:
                  rv = Element::setProperty(pid, v);
                  break;
            }
      layoutInvalid = true;
      triggerLayout();
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextBase::propertyDefault(Pid id) const
      {
      if (id == Pid::Z)
            return Element::propertyDefault(id);
      if (composition()) {
            QVariant v = parent()->propertyDefault(id);
            if (v.isValid())
                  return v;
            }
      Sid sid = getPropertyStyle(id);
      if (sid != Sid::NOSTYLE)
            return styleValue(id, sid);
      QVariant v;
      switch (id) {
            case Pid::SUB_STYLE:
                  v = int(Tid::DEFAULT);
                  break;
            case Pid::TEXT:
                  v = QString();
                  break;
            default:
                  for (const StyledProperty& p : *textStyle(Tid::DEFAULT)) {
                        if (p.pid == id)
                              return styleValue(id, p.sid);
                        }
                  return Element::propertyDefault(id);
            }
      return v;
      }

//---------------------------------------------------------
//   getPropertyFlagsIdx
//---------------------------------------------------------

int TextBase::getPropertyFlagsIdx(Pid id) const
      {
      int i = 0;
      for (const StyledProperty& p : *_elementStyle) {
            if (p.pid == id)
                  return i;
            ++i;
            }
      for (const StyledProperty& p : *textStyle(tid())) {
            if (p.pid == id)
                  return i;
            ++i;
            }
      return -1;
      }

//---------------------------------------------------------
//   offsetSid
//---------------------------------------------------------

Sid TextBase::offsetSid() const
      {
      Tid defaultTid = Tid(propertyDefault(Pid::SUB_STYLE).toInt());
      if (tid() != defaultTid)
            return Sid::NOSTYLE;
      bool above = placeAbove();
      switch (tid()) {
            case Tid::DYNAMICS:
                  return above ? Sid::dynamicsPosAbove : Sid::dynamicsPosBelow;
            case Tid::LYRICS_ODD:
            case Tid::LYRICS_EVEN:
                  return above ? Sid::lyricsPosAbove : Sid::lyricsPosBelow;
            case Tid::REHEARSAL_MARK:
                  return above ? Sid::rehearsalMarkPosAbove : Sid::rehearsalMarkPosBelow;
            case Tid::STAFF:
                  return above ? Sid::staffTextPosAbove : Sid::staffTextPosBelow;
            case Tid::STICKING:
                  return above ? Sid::stickingPosAbove : Sid::stickingPosBelow;
            case Tid::SYSTEM:
                  return above ? Sid::systemTextPosAbove : Sid::systemTextPosBelow;
            case Tid::TEMPO:
                  return above ? Sid::tempoPosAbove : Sid::tempoPosBelow;
            case Tid::MEASURE_NUMBER:
                  return above ? Sid::measureNumberPosAbove : Sid::measureNumberPosBelow;
            case Tid::MMREST_RANGE:
                  return above ? Sid::mmRestRangePosAbove : Sid::mmRestRangePosBelow;
            default:
                  break;
            }
      return Sid::NOSTYLE;
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TextBase::getPropertyStyle(Pid id) const
      {
      if (id == Pid::OFFSET) {
            Sid sid = offsetSid();
            if (sid != Sid::NOSTYLE)
                  return sid;
            }
      for (const StyledProperty& p : *_elementStyle) {
            if (p.pid == id)
                  return p.sid;
            }
      for (const StyledProperty& p : *textStyle(tid())) {
            if (p.pid == id)
                  return p.sid;
            }
      return Sid::NOSTYLE;
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void TextBase::styleChanged()
      {
      if (!styledProperties()) {
            qDebug("no styled properties");
            return;
            }
      int i = 0;
      for (const StyledProperty& spp : *_elementStyle) {
            PropertyFlags f = _propertyFlagsList[i];
            if (f == PropertyFlags::STYLED)
                  setProperty(spp.pid, safePropertyStyleValue(spp.pid));
            ++i;
            }
      for (const StyledProperty& spp : *textStyle(tid())) {
            PropertyFlags f = _propertyFlagsList[i];
            if (f == PropertyFlags::STYLED)
                  setProperty(spp.pid, safePropertyStyleValue(spp.pid));
            ++i;
            }
      }

//---------------------------------------------------------
//   initElementStyle
//---------------------------------------------------------

void TextBase::initElementStyle(const ElementStyle* ss)
      {
      _elementStyle = ss;
      size_t n      = ss->size() + TEXT_STYLE_SIZE;

      delete[] _propertyFlagsList;
      _propertyFlagsList = new PropertyFlags[n];
      for (size_t i = 0; i < n; ++i)
            _propertyFlagsList[i] = PropertyFlags::STYLED;
      for (const StyledProperty& p : *_elementStyle)
            setProperty(p.pid, styleValue(p.pid, p.sid));
      for (const StyledProperty& p : *textStyle(tid()))
            setProperty(p.pid, styleValue(p.pid, p.sid));
      }

//---------------------------------------------------------
//   initTid
//---------------------------------------------------------

void TextBase::initTid(Tid tid, bool preserveDifferent)
      {
      if (! preserveDifferent)
            initTid(tid);
      else {
            setTid(tid);
            for (const StyledProperty& p : *textStyle(tid)) {
                  if (getProperty(p.pid) == propertyDefault(p.pid))
                        setProperty(p.pid, styleValue(p.pid, p.sid));
                  }
            }
      }

void TextBase::initTid(Tid tid)
      {
      setTid(tid);
      for (const StyledProperty& p : *textStyle(tid)) {
            setProperty(p.pid, styleValue(p.pid, p.sid));
            }
      }

//---------------------------------------------------------
//   editCut
//---------------------------------------------------------

void TextBase::editCut(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;
      QString s = _cursor->selectedText();

      if (!s.isEmpty()) {
            QApplication::clipboard()->setText(s);
            ed.curGrip = Grip::START;
            ed.key     = Qt::Key_Delete;
            ed.s       = QString();
            edit(ed);
            }
      }

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void TextBase::editCopy(EditData& ed)
      {
      //
      // store selection as plain text
      //
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;
      QString s = _cursor->selectedText();
      if (!s.isEmpty())
            QApplication::clipboard()->setText(s);
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

TextCursor* TextBase::cursor(const EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      Q_ASSERT(ted);
      return &ted->cursor;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextBase::draw(QPainter* p) const
      {
      if (hasFrame()) {
            qreal baseSpatium = MScore::baseStyle().value(Sid::spatium).toDouble();
            if (!qFuzzyIsNull(frameWidth().val())) {
                  QColor fColor = curColor(visible(), frameColor());
                  qreal frameWidthVal = frameWidth().val() * (sizeIsSpatiumDependent() ? spatium() : baseSpatium);

                  QPen pen(fColor, frameWidthVal, Qt::SolidLine,
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
                  qreal frameRoundFactor = (sizeIsSpatiumDependent() ? (spatium()/baseSpatium) / 2 : 0.5f);

                  int r2 = frameRound() * frameRoundFactor;
                  if (r2 > 99)
                        r2 = 99;
                  p->drawRoundedRect(frame, frameRound() * frameRoundFactor, r2);
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

void TextBase::drawEditMode(QPainter* p, EditData& ed)
      {
      QPointF pos(canvasPos());
      p->translate(pos);

      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      if (!ted) {
            qDebug("ted not found");
            return;
            }
      TextCursor* _cursor = &ted->cursor;

      if (_cursor->hasSelection()) {
            p->setBrush(Qt::NoBrush);
            p->setPen(textColor());
            int r1 = _cursor->selectLine();
            int r2 = _cursor->row();
            int c1 = _cursor->selectColumn();
            int c2 = _cursor->column();
            sort(r1, c1, r2, c2);
            int row = 0;
            for (const TextBlock& t : qAsConst(_layout)) {
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

      // Don't draw cursor if there is a selection
      if (!_cursor->hasSelection())
            p->drawRect(_cursor->cursorRect());

      QTransform transform = p->worldTransform();
      p->translate(-pos);
      p->setPen(QPen(QBrush(Qt::lightGray), 4.0 / transform.m11()));  // 4 pixel pen size
      p->setBrush(Qt::NoBrush);

      qreal m = spatium();
      QRectF r = canvasBoundingRect().adjusted(-m, -m, m, m);
//      qDebug("%f %f %f %f\n", r.x(), r.y(), r.width(), r.height());

      p->drawRect(r);
      pen = QPen(MScore::defaultColor, 0.0);
      }

//---------------------------------------------------------
//   hasCustomFormatting
//---------------------------------------------------------

bool TextBase::hasCustomFormatting() const
      {
      CharFormat fmt;
      fmt.setFontFamily(family());
      fmt.setFontSize(size());
      fmt.setStyle(fontStyle());
      fmt.setPreedit(false);
      fmt.setValign(VerticalAlignment::AlignNormal);

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (f.text.isEmpty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                  const CharFormat& format = f.format;
                  if (fmt.style() != format.style())
                        return true;
                  if (format.fontSize() != fmt.fontSize())
                        return true;
                  if (format.fontFamily() != fmt.fontFamily())
                        return true;

                  VerticalAlignment va = format.valign();
                  VerticalAlignment cva = fmt.valign();
                  if (cva != va)
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   stripText
//    remove some custom text formatting and return
//    result as xml string
//---------------------------------------------------------

QString TextBase::stripText(bool removeStyle, bool removeSize, bool removeFace) const
      {
      QString _txt;
      bool bold_      = false;
      bool italic_    = false;
      bool underline_ = false;
      bool strike_    = false;

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (!f.format.bold() && bold())
                        bold_ = true;
                  if (!f.format.italic() && italic())
                        italic_ = true;
                  if (!f.format.underline() && underline())
                        underline_ = true;
                  if (!f.format.strike() && strike())
                        strike_ = true;
                  }
            }
      CharFormat fmt;
      fmt.setFontFamily(family());
      fmt.setFontSize(size());
      fmt.setStyle(fontStyle());
      fmt.setPreedit(false);
      fmt.setValign(VerticalAlignment::AlignNormal);

      XmlNesting xmlNesting(&_txt);
      if (!removeStyle) {
            if (bold_)
                  xmlNesting.pushB();
            if (italic_)
                  xmlNesting.pushI();
            if (underline_)
                  xmlNesting.pushU();
            if (strike_)
                  xmlNesting.pushS();
            }

      for (const TextBlock& block : _layout) {
            for (const TextFragment& f : block.fragments()) {
                  if (f.text.isEmpty())                     // skip empty fragments, not to
                        continue;                           // insert extra HTML formatting
                  const CharFormat& format = f.format;
                  if (!removeStyle) {
                        if (fmt.bold() != format.bold()) {
                              if (format.bold())
                                    xmlNesting.pushB();
                              else
                                    xmlNesting.popB();
                              }
                        if (fmt.italic() != format.italic()) {
                              if (format.italic())
                                    xmlNesting.pushI();
                              else
                                    xmlNesting.popI();
                              }
                        if (fmt.underline() != format.underline()) {
                              if (format.underline())
                                    xmlNesting.pushU();
                              else
                                    xmlNesting.popU();
                              }
                        if (fmt.strike() != format.strike()) {
                              if (format.strike())
                                    xmlNesting.pushS();
                              else
                                    xmlNesting.popS();
                              }
                        }

                  if (!removeSize && (format.fontSize() != fmt.fontSize()))
                        _txt += QString("<font size=\"%1\"/>").arg(format.fontSize());
                  if (!removeFace && (format.fontFamily() != fmt.fontFamily()))
                        _txt += QString("<font face=\"%1\"/>").arg(TextBase::escape(format.fontFamily()));

                  VerticalAlignment va = format.valign();
                  VerticalAlignment cva = fmt.valign();
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
                  _txt += XmlWriter::xmlString(f.text);
                  fmt = format;
                  }
            if (block.eol())
                  _txt += QChar::LineFeed;
            }
      while (!xmlNesting.empty())
            xmlNesting.popToken();
      return _txt;
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void TextBase::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (ps == PropertyFlags::STYLED && v == propertyDefault(id)) {
            // this is a reset
            // remove some custom formatting
            if (id == Pid::FONT_STYLE)
                  undoChangeProperty(Pid::TEXT, stripText(true, false, false), propertyFlags(id));
            else if (id == Pid::FONT_SIZE)
                  undoChangeProperty(Pid::TEXT, stripText(false, true, false), propertyFlags(id));
            else if (id == Pid::FONT_FACE)
                  undoChangeProperty(Pid::TEXT, stripText(false, false, true), propertyFlags(id));
            }
      Element::undoChangeProperty(id, v, ps);
      }

}
