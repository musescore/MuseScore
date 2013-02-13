//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: text.cpp 5627 2012-05-14 20:18:41Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "text.h"
#include "xml.h"
#include "style.h"
#include "score.h"
#include "utils.h"
#include "page.h"
#include "sym.h"
#include "symbol.h"
#include "textline.h"
#include "system.h"
#include "measure.h"
#include "box.h"
#include "segment.h"
#include "mscore.h"
#include "textframe.h"

QReadWriteLock docRenderLock;

QTextCursor* Text::_cursor;

//---------------------------------------------------------
//   createDoc
//---------------------------------------------------------

void Text::createDoc()
      {
      _doc = new QTextDocument(0);
      _doc->setDocumentMargin(0);
      _doc->setUseDesignMetrics(true);
      _doc->setUndoRedoEnabled(true);
      _doc->documentLayout()->setProperty("cursorWidth", QVariant(2));
      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      _doc->setDefaultTextOption(to);
      _doc->setDefaultFont(textStyle().font(spatium()));
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : SimpleText(s)
      {
      setFlag(ELEMENT_MOVABLE, true);
      _doc        = 0;
      _editMode   = false;
      _styleIndex = TEXT_STYLE_DEFAULT;
      }

Text::Text(const Text& e)
   : SimpleText(e)
      {
      if (e._doc)
            _doc = e._doc->clone();
      else
            _doc = 0;
      _styleIndex = e._styleIndex;
      _editMode   = false;
      }

Text::~Text()
      {
      delete _doc;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QString& s)
      {
      if (styled())
            SimpleText::setText(s);
      else
            setUnstyledText(s);
      textChanged();
      }

//---------------------------------------------------------
//   setUnstyledText
//---------------------------------------------------------

void Text::setUnstyledText(const QString& s)
      {
      Align align = textStyle().align();
      _doc->clear();

      QTextCursor c(_doc);
      c.setVisualNavigation(true);
      c.movePosition(QTextCursor::Start);
      Qt::Alignment a;
      if (align & ALIGN_HCENTER)
            a = Qt::AlignHCenter;
      else if (align & ALIGN_RIGHT)
            a = Qt::AlignRight;
      else
            a = Qt::AlignLeft;
      QTextBlockFormat bf = c.blockFormat();
      bf.setAlignment(a);
      c.setBlockFormat(bf);

      QTextCharFormat tf = c.charFormat();
      tf.setFont(textStyle().font(spatium()));
      c.setBlockCharFormat(tf);
      c.insertText(s);
      textChanged();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QTextDocumentFragment& f)
      {
      setHtml(f.toHtml());
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void Text::setHtml(const QString& s)
      {
      setUnstyled();
      _doc->clear();
      _doc->setHtml(s);
      textChanged();
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString Text::getText() const
      {
      return (styled() && !_editMode) ? SimpleText::getText() : _doc->toPlainText();
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString Text::getHtml() const
      {
      return styled() ? "" : _doc->toHtml("utf-8");
      }

//---------------------------------------------------------
//   systemFlag
//---------------------------------------------------------

bool Text::systemFlag() const
      {
      return textStyle().systemFlag();
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Text::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
      layout1();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Text::layout1()
      {
      if (styled() && !_editMode)
            SimpleText::layout();
      else {
            QPointF o(textStyle().offset(spatium()));

            _doc->setDefaultFont(textStyle().font(spatium()));
            qreal w = -1.0;

            if (parent() && layoutToParentWidth()) {
                  Element* e = parent();
                  w = e->width();
                  if (e->type() == HBOX || e->type() == VBOX || e->type() == TBOX) {
                        Box* b = static_cast<Box*>(e);
                        w -= ((b->leftMargin() + b->rightMargin()) * MScore::DPMM);
                        }
                  }

            QTextOption to = _doc->defaultTextOption();
            to.setUseDesignMetrics(true);
            to.setWrapMode(w <= 0.0 ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
            _doc->setDefaultTextOption(to);

            if (w <= 0.0)
                  w = _doc->idealWidth();
            _doc->setTextWidth(w);

            QSizeF size(_doc->size());

            if (align() & ALIGN_BOTTOM) {
                  o.ry() += 3;
                  o.ry() -= size.height();
                  }
            else if (align() & ALIGN_VCENTER)
                  o.ry() -= (size.height() * .5);
            else if (align() & ALIGN_BASELINE)
                  o.ry() -= baseLine();
            else {
                  if (_editMode) {
                        QFontMetricsF fm(textStyle().font(spatium()));
                        QRectF r1(fm.boundingRect(SimpleText::firstLine()));
                        QRectF r(fm.tightBoundingRect(SimpleText::firstLine()));
                        o.ry() -= (r1.height() - r.height()) * .5;
                        o.ry() -= 2;
                        }
                  }

            if (align() & ALIGN_RIGHT)
                  o.rx() -= size.width();
            else if (align() & ALIGN_HCENTER)
                  o.rx() -= (size.width() * .5);
            else {
                  if (styled() && _editMode) {
                        //
                        // this does not work for codepoints > 0xffff
                        // (musical symbols)
                        //
                        QFontMetricsF fm(textStyle().font(spatium()));
                        QString s = SimpleText::firstLine();
                        if (!s.isEmpty()) {
                              QChar c = s[0];
                              qreal lb = fm.leftBearing(c);
printf("lb %f\n", lb);
                              if (lb < 0.0)
                                    o.rx() += -lb;
                              }
                        }
                  }

            bbox().setRect(0.0, 0.0, size.width(), size.height());
            _doc->setModified(false);
            setPos(o);
            }
      if (parent()) {
            Element* e = parent();
            qreal w, h, xo, yo;
            if (layoutToParentWidth()) {
                  if (e->type() == HBOX || e->type() == VBOX || e->type() == TBOX) {
                        // consider inner margins of frame
                        Box* b = static_cast<Box*>(e);
                        xo = b->leftMargin() * MScore::DPMM;
                        yo = b->topMargin()  * MScore::DPMM;
                        w  = b->width()  - xo - b->rightMargin() * MScore::DPMM;
                        h  = b->height() - yo - b->bottomMargin()   * MScore::DPMM;
                        }
                  else {
                        w  = e->width();
                        h  = e->height();
                        xo = 0.0;
                        yo = 0.0;
                        }
                  QPointF ro(_textStyle.reloff() * .01);
                  rxpos() += xo + ro.x() * w;
                  rypos() += yo + ro.y() * h;
                  }
            if (e->type() == SEGMENT) {
                  Segment* s = static_cast<Segment*>(e);
                  rypos() += s->measure()->system()->staff(staffIdx())->y();
                  }
            }

      if (hasFrame())
            layoutFrame();
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX)) {
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
//   draw
//---------------------------------------------------------

void Text::draw(QPainter* painter) const
      {
      drawFrame(painter);
      if (styled() && !_editMode) {
            SimpleText::draw(painter);
            return;
            }
      QAbstractTextDocumentLayout::PaintContext c;
      bool printing = score() && score()->printing();
      if (_cursor
         && _doc
         && _cursor->document() == _doc
         && !printing) {
            if (_cursor->hasSelection()) {
                  QAbstractTextDocumentLayout::Selection selection;
                  selection.cursor = *_cursor;
                  selection.format.setBackground(c.palette.brush(QPalette::Active, QPalette::Highlight));
                  selection.format.setForeground(c.palette.brush(QPalette::Active, QPalette::HighlightedText));
                  c.selections.append(selection);
                  }
            c.cursorPosition = _cursor->position();
            }
      else
            c.cursorPosition = -1;

      if ((printing || !score()->showInvisible()) && !visible())
            return;
      c.palette.setColor(QPalette::Text, textColor());

#if 1
      // make it thread save
      {
      QWriteLocker locker(&docRenderLock);
      QScopedPointer<QTextDocument> __doc(_doc->clone());
      __doc.data()->documentLayout()->draw(painter, c);
      // _doc->documentLayout()->draw(painter, c);
      }
#else
      _doc->documentLayout()->draw(painter, c);
#endif
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperties(xml, true);
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

void Text::writeProperties(Xml& xml, bool writeText) const
      {
      Element::writeProperties(xml);
      if (xml.clipboardmode || styled())
            xml.tag("style", textStyle().name());
      if (xml.clipboardmode || !styled())
            _textStyle.writeProperties(xml);
      if (writeText) {
            if (styled())
                  xml.tag("text", getText());
            else {
                  xml.stag("html-data");
                  xml.writeHtml(_doc->toHtml("utf-8"));
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//   isSimpleText
//    check if _doc can be converted to simple text
//---------------------------------------------------------

bool Text::isSimpleText() const
      {
      if (_doc->blockCount() > 1)
            return false;
      int n = 0;
      QTextBlock b(_doc->firstBlock());
      QTextBlock::iterator i(_doc->firstBlock().begin());
      for (; !i.atEnd(); ++i)
            ++n;
      return n <= 1;
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
                        case 1:  i = TEXT_STYLE_UNSTYLED;  break;
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
                        case 13: i = TEXT_STYLE_TECHNIK;   break;
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
                              i = TEXT_STYLE_UNSTYLED;
                              break;
                        }
                  st = i;
                  }
            else
                  st = score()->style()->textStyleType(val);

            if (st == TEXT_STYLE_UNSTYLED)
                  setUnstyled();
            else if (st == TEXT_STYLE_UNKNOWN)
                  _styleIndex = st;
            else
                  setTextStyleType(st);
            }
      else if (tag == "styleName")          // obsolete, unstyled text
            e.skipCurrentElement(); // _styleName = val;
      else if (tag == "data")                  // obsolete
            _doc->setHtml(e.readElementText());
      else if (tag == "html") {
            QString s = Xml::htmlToString(e);
            setHtml(s);
            }
      else if (tag == "text")
            setText(e.readElementText());
      else if (tag == "html-data") {
            QString s = Xml::htmlToString(e);
            if (score()->mscVersion() <= 114) {
                  s.replace("MScore1", "FreeSerifMscore");
                  s.replace(QChar(0xe10e), QChar(0x266e));    //natural
                  s.replace(QChar(0xe10c), QChar(0x266f));    // sharp
                  s.replace(QChar(0xe10d), QChar(0x266d));    // flat
                  s.replace(QChar(0xe104), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd5e))),    // note2_Sym
                  s.replace(QChar(0xe105), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd5f)));    // note4_Sym
                  s.replace(QChar(0xe106), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd60)));    // note8_Sym
                  s.replace(QChar(0xe107), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd61)));    // note16_Sym
                  s.replace(QChar(0xe108), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd62)));    // note32_Sym
                  s.replace(QChar(0xe109), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd63)));    // note64_Sym
                  s.replace(QChar(0xe10a), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd6d)));    // dot
                  s.replace(QChar(0xe10b), QString("%1%2%3%4").arg(QChar(0xd834)).arg(QChar(0xdd6d)).arg(QChar(0xd834)).arg(QChar(0xdd6d)));    // dotdot
                  s.replace(QChar(0xe167), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd0b)));    // coda
                  s.replace(QChar(0xe168), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd0c)));    // varcoda
                  s.replace(QChar(0xe169), QString("%1%2").arg(QChar(0xd834)).arg(QChar(0xdd0c)));    // segno
                  if (_doc == 0)
                        createDoc();
                  // import instrument names as unstyled html
                  if (_styleIndex != TEXT_STYLE_INSTRUMENT_SHORT
                     && _styleIndex != TEXT_STYLE_INSTRUMENT_LONG
                     && isSimpleText()) {
                        _doc->setHtml(s);
                        QString s = _doc->toPlainText();
                        delete _doc;
                        _doc = 0;
                        setText(s);
                        }
                  else {
                        setUnstyled();
                        setHtml(s);
                        }
                  }
            else {
                  setHtml(s);
                  }
            }
      else if (tag == "subtype")          // obsolete
            e.skipCurrentElement();
      else if (tag == "frameWidth") {           // obsolete
            qreal spMM = spatium() / MScore::DPMM;
            setFrameWidth(Spatium(e.readDouble() / spMM));
            }
      else if (tag == "paddingWidth") {          // obsolete
            qreal spMM = spatium() / MScore::DPMM;
            setPaddingWidth(Spatium(e.readDouble() / spMM));
            }
      else if (_textStyle.readProperties(e))
            ;
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Text::spatiumChanged(qreal oldVal, qreal newVal)
      {
      Element::spatiumChanged(oldVal, newVal);

      if (!sizeIsSpatiumDependent() || styled())
            return;
      qreal v = newVal / oldVal;

      QTextCursor c(_doc);
      QTextBlock cb = _doc->begin();
      while (cb.isValid()) {
            QTextBlock::iterator i(cb.begin());
            for (; !i.atEnd(); ++i) {
                  QTextFragment f = i.fragment();
                  if (f.isValid()) {
                        int pos = f.position();
                        int len = f.length();
                        c.setPosition(pos, QTextCursor::MoveAnchor);
                        c.setPosition(pos + len, QTextCursor::KeepAnchor);
                        QTextCharFormat cf = c.charFormat();
                        QFont font = cf.font();
                        font.setPointSizeF(font.pointSizeF() * v);
                        cf.setFont(font);
                        c.setCharFormat(cf);
                        }
                  }
            cb = cb.next();
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Text::startEdit(MuseScoreView*, const QPointF& p)
      {
      _editMode = true;
      if (styled()) {
            createDoc();
            setUnstyledText(SimpleText::getText());
            layout();
            }
      _cursor = new QTextCursor(_doc);
      _cursor->setVisualNavigation(true);
      setCursor(p);
      qreal w = 2.0; // 8.0 / view->matrix().m11();
      score()->rebuildBspTree();
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Text::edit(MuseScoreView*, int /*grip*/, int key, Qt::KeyboardModifiers modifiers, const QString& s)
      {
      if (MScore::debugMode)
            qDebug("Text::edit(%p) key 0x%x mod 0x%x\n", this, key, int(modifiers));
      if (!_editMode || !_cursor) {
            qDebug("Text::edit(%p): not in edit mode: %d %p\n", this, _editMode, _cursor);
            return false;
            }
      bool lo = type() == INSTRUMENT_NAME;
      score()->setLayoutAll(lo);
      static const qreal w = 2.0; // 8.0 / view->matrix().m11();
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));

      if (modifiers == Qt::ControlModifier) {
            switch (key) {
                  case Qt::Key_A:   // select all
                        _cursor->select(QTextCursor::Document);
                        break;
                  case Qt::Key_B:   // toggle bold face
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontWeight(f.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_I:   // toggle italic
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontItalic(!f.fontItalic());
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_U:   // toggle underline
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontUnderline(!f.fontUnderline());
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_Up:
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSubScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
                        _cursor->setCharFormat(f);
                        }
                        break;

                  case Qt::Key_Down:
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSuperScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
                        _cursor->setCharFormat(f);
                        }
                        break;
                  }
#ifndef Q_WS_MAC
            if (key != Qt::Key_Space && key != Qt::Key_Minus)
                  return true;
#endif
            }
#ifdef Q_WS_MAC
      else if (modifiers == Qt::AltModifier) {
	      if (key != Qt::Key_Space && key != Qt::Key_Minus)
                  return true;
            }
#endif
      QTextCursor::MoveMode mm = (modifiers & Qt::ShiftModifier)
         ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
      switch (key) {
            case Qt::Key_Return:
                  _cursor->insertText(QString("\r"));
                  break;

            case Qt::Key_Backspace:
                  _cursor->deletePreviousChar();
                  break;

            case Qt::Key_Delete:
                  _cursor->deleteChar();
                  break;

            case Qt::Key_Left:
                  if (!_cursor->movePosition(QTextCursor::Left, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  break;

            case Qt::Key_Right:
                  if (!_cursor->movePosition(QTextCursor::Right, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  break;

            case Qt::Key_Up:
                  _cursor->movePosition(QTextCursor::Up, mm);
                  break;

            case Qt::Key_Down:
                  _cursor->movePosition(QTextCursor::Down, mm);
                  break;

            case Qt::Key_Home:
                  _cursor->movePosition(QTextCursor::Start, mm);
                  break;

            case Qt::Key_End:
                  _cursor->movePosition(QTextCursor::End, mm);
                  break;

            case Qt::Key_Space:
                  _cursor->insertText(" ");
                  break;

            case Qt::Key_Minus:
                  _cursor->insertText("-");
                  break;

            default:
                  if (!s.isEmpty())
                        _cursor->insertText(s);
                  break;
            }
      if (key == Qt::Key_Return || key == Qt::Key_Space || key == Qt::Key_Tab) {
            replaceSpecialChars();
            }
      layoutEdit();
      return true;
      }

//---------------------------------------------------------
//   layoutEdit
//---------------------------------------------------------

void Text::layoutEdit()
      {
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
            score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
            }
      }

//---------------------------------------------------------
//   replaceSpecialChars
//---------------------------------------------------------

bool Text::replaceSpecialChars()
      {
      QTextCursor startCur = *_cursor;
      foreach (const char* s, charReplaceMap.keys()) {
            SymCode sym = *charReplaceMap.value(s);
            switch (sym.type) {
                  case SYMBOL_FRACTION:
                        if (!MScore::replaceFractions)
                              continue;
                        break;
                  default:
                        ;
                  }
            QTextCursor cur = _doc->find(s, _cursor->position() - 1 - strlen(s),
                  QTextDocument::FindWholeWords);
            if (cur.isNull())
                  continue;
            // do not go beyond the cursor
            if (cur.selectionEnd() > _cursor->selectionEnd())
                  continue;
            addSymbol(sym, &cur);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   moveCursorToEnd
//---------------------------------------------------------

void Text::moveCursorToEnd()
      {
      if (_cursor)
            _cursor->movePosition(QTextCursor::End);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void Text::moveCursor(int col)
      {
      if (_cursor)
            _cursor->setPosition(col);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Text::shape() const
      {
      if (styled())
            return SimpleText::shape();
      QPainterPath pp;

      for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
            QTextLayout* tl = tb.layout();
            int n = tl->lineCount();
            for (int i = 0; i < n; ++i) {
                  QTextLine l = tl->lineAt(i);
                  QRectF r(l.naturalTextRect().translated(tl->position()));
                  r.adjust(-l.position().x(), 0.0, 0.0, 0.0);
                  pp.addRect(r);
                  }
            }
      return pp;
      }

//---------------------------------------------------------
//   baseLine
//    returns ascent of first text line in first block
//---------------------------------------------------------

qreal Text::baseLine() const
      {
      if (styled() && !_editMode)
            return SimpleText::baseLine();
      for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
            const QTextLayout* tl = tb.layout();
            if (tl->lineCount()) {
                  return (tl->lineAt(0).ascent() + tl->lineAt(0).leading()
                     + tl->position().y());
                  }
            }
      return 0.0;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal Text::lineSpacing() const
      {
      return QFontMetricsF(textStyle().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//    HACK
//---------------------------------------------------------

qreal Text::lineHeight() const
      {
      return QFontMetricsF(textStyle().font(spatium())).height();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Text::addSymbol(const SymCode& s, QTextCursor* cur)
      {
      if (cur == 0)
            cur = _cursor;
      if (s.fontId >= 0) {
            QTextCharFormat nFormat(cur->charFormat());
            nFormat.setFontFamily(fontId2font(s.fontId).family());
            QString ss;
            if (s.code >= 0x10000) {
                  ss = QChar(QChar::highSurrogate(s.code));
                  ss += QChar(QChar::lowSurrogate(s.code));
                  }
            else
                  ss = QChar(s.code);
            cur->insertText(ss, nFormat);
            }
      else
            cur->insertText(QChar(s.code));
      score()->setLayoutAll(true);
      score()->end();
      }

//---------------------------------------------------------
//   addChar
//---------------------------------------------------------

void Text::addChar(int code, QTextCursor* cur)
      {
      if (cur == 0)
            cur = _cursor;

      QString ss;
      if (code & 0xffff0000) {
            ss = QChar(QChar::highSurrogate(code));
            ss += QChar(QChar::lowSurrogate(code));
            }
      else
            ss = QChar(code);
      cur->insertText(ss);
      score()->setLayoutAll(true);
      score()->end();
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void Text::setBlockFormat(const QTextBlockFormat& bf)
      {
      if (!_cursor)
            return;
      _cursor->setBlockFormat(bf);
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

bool Text::setCursor(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;

      int idx = _doc->documentLayout()->hitTest(pt, Qt::FuzzyHit);
      if (idx == -1)
            return true;
      if (_cursor) {
            _cursor->setPosition(idx, mode);
            if (_cursor->hasSelection())
                  QApplication::clipboard()->setText(_cursor->selectedText(), QClipboard::Selection);
            }
      return true;
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
//   paste
//---------------------------------------------------------

void Text::paste()
      {
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
      if (MScore::debugMode)
            qDebug("Text::paste() <%s>\n", qPrintable(txt));
      _cursor->insertText(txt);
      layoutEdit();
      bool lo = type() == INSTRUMENT_NAME;
      score()->setLayoutAll(lo);
      score()->setUpdateAll();
      score()->end();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Text::dragAnchor() const
      {
      QPointF p1;

      if (parent()->type() == MEASURE) {
            Measure* m     = static_cast<Measure*>(parent());
            System* system = m->system();
            qreal yp       = system->staff(staffIdx())->y() + system->y() + system->page()->pos().x();
            qreal xp       = m->canvasPos().x(); // m->tick2pos(tick()) + m->canvasPos().x();
            p1 = QPointF(xp, yp);
            }
      else {
            p1 = parent()->canvasPos(); // QPointF(parent()->canvasBoundingRect().topLeft());
            if (parent()->type() == SEGMENT) {
                  Segment* s = static_cast<Segment*>(parent());
                  p1.ry() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
                  }
            }

      QPointF p2;
      QRectF r(canvasBoundingRect());

      if (align() & ALIGN_BOTTOM)
            p2.ry() = r.bottom();
      else if (align() & ALIGN_VCENTER)
            p2.ry() = r.center().y();
      else if (align() & ALIGN_BASELINE)
            p2.ry() = canvasPos().y();
      else // ALIGN_TOP
            p2.ry() = r.top();

      if (align() & ALIGN_RIGHT)
            p2.rx() = r.right();
      else if (align() & ALIGN_HCENTER)
            p2.rx() = r.center().x();
      else  // ALIGN_LEFT
            p2.rx() = r.left();

      return QLineF(p1, p2);
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
//   sizeIsSpatiumDependent
//---------------------------------------------------------

bool Text::sizeIsSpatiumDependent() const
      {
      return textStyle().sizeIsSpatiumDependent();
      }

//---------------------------------------------------------
//   setSizeIsSpatiumDependent
//---------------------------------------------------------

void Text::setSizeIsSpatiumDependent(int v)
      {
      _textStyle.setSizeIsSpatiumDependent(v);
      }

//---------------------------------------------------------
//   xoff
//---------------------------------------------------------

qreal Text::xoff() const
      {
      return textStyle().offset().x();
      }

//---------------------------------------------------------
//   offsetType
//---------------------------------------------------------

OffsetType Text::offsetType() const
      {
      return textStyle().offsetType();
      }

//---------------------------------------------------------
//   reloff
//---------------------------------------------------------

QPointF Text::reloff() const
      {
      return textStyle().reloff();
      }

//---------------------------------------------------------
//   setAlign
//---------------------------------------------------------

void Text::setAlign(Align val)
      {
      _textStyle.setAlign(val);
      }

//---------------------------------------------------------
//   setXoff
//---------------------------------------------------------

void Text::setXoff(qreal val)
      {
      _textStyle.setXoff(val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Text::setYoff(qreal val)
      {
      _textStyle.setYoff(val);
      }

//---------------------------------------------------------
//   setOffsetType
//---------------------------------------------------------

void Text::setOffsetType(OffsetType val)
      {
      _textStyle.setOffsetType(val);
      }

//---------------------------------------------------------
//   setRxoff
//---------------------------------------------------------

void Text::setRxoff(qreal v)
      {
      _textStyle.setRxoff(v);
      }

//---------------------------------------------------------
//   setRyoff
//---------------------------------------------------------

void Text::setRyoff(qreal v)
      {
      _textStyle.setRyoff(v);
      }

//---------------------------------------------------------
//   setReloff
//---------------------------------------------------------

void Text::setReloff(const QPointF& p)
      {
      _textStyle.setReloff(p);
      }

//---------------------------------------------------------
//   yoff
//---------------------------------------------------------

qreal Text::yoff() const
      {
      return textStyle().offset().y();
      }

//---------------------------------------------------------
//   setFrameWidth
//---------------------------------------------------------

void Text::setFrameWidth(Spatium val)
      {
      _textStyle.setFrameWidth(val);
      }

//---------------------------------------------------------
//   setPaddingWidth
//---------------------------------------------------------

void Text::setPaddingWidth(Spatium val)
      {
      _textStyle.setPaddingWidth(val);
      }

//---------------------------------------------------------
//   setFrameColor
//---------------------------------------------------------

void Text::setFrameColor(const QColor& val)
      {
      _textStyle.setFrameColor(val);
      }

//---------------------------------------------------------
//   setFrameRound
//---------------------------------------------------------

void Text::setFrameRound(int val)
      {
      _textStyle.setFrameRound(val);
      }

//---------------------------------------------------------
//   setCircle
//---------------------------------------------------------

void Text::setCircle(bool val)
      {
      _textStyle.setCircle(val);
      }

//---------------------------------------------------------
//   setItalic
//---------------------------------------------------------

void Text::setItalic(bool val)
      {
      _textStyle.setItalic(val);
      }

//---------------------------------------------------------
//   setBold
//---------------------------------------------------------

void Text::setBold(bool val)
      {
      _textStyle.setBold(val);
      }

//---------------------------------------------------------
//   setSize
//---------------------------------------------------------

void Text::setSize(qreal v)
      {
      _textStyle.setSize(v);
      }

//---------------------------------------------------------
//   setHasFrame
//---------------------------------------------------------

void Text::setHasFrame(bool val)
      {
      _textStyle.setHasFrame(val);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Text::font() const
      {
      return _textStyle.font(spatium());
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Text::styleChanged()
      {
      if (styled()) {
            if (_styleIndex != TEXT_STYLE_UNKNOWN)
                  setTextStyle(score()->textStyle(_styleIndex));
            setText(getText());     // destroy formatting
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Text::setScore(Score* s)
      {
      if (s == score())
            return;
      Element::setScore(s);
      // TODO: handle custom text styles
      styleChanged();
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void Text::setFont(const QFont& f)
      {
      _textStyle.setFont(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Text::clear()
      {
      if (styled())
            SimpleText::clear();
      else
            _doc->clear();
      }

//---------------------------------------------------------
//   setTextStyleType
//---------------------------------------------------------

void Text::setTextStyleType(int st)
      {
//      if (st == _styleIndex)
//            return;
      _styleIndex = st;
      if (st != TEXT_STYLE_UNKNOWN)
            setTextStyle(score()->textStyle(st));
      if (_doc && !_doc->isEmpty() && !_editMode) {
            SimpleText::setText(_doc->toPlainText());
            delete _doc;
            _doc = 0;
            }
      }

//---------------------------------------------------------
//   setUnstyled
//---------------------------------------------------------

void Text::setUnstyled()
      {
      if (!styled())
            return;
      _styleIndex = TEXT_STYLE_UNSTYLED;
      if (_editMode)
            return;
      createDoc();
      if (!SimpleText::isEmpty())
            setUnstyledText(SimpleText::getText());
      }

//---------------------------------------------------------
//   startCursorEdit
//---------------------------------------------------------

QTextCursor* Text::startCursorEdit()
      {
      if (styled()) {
            qDebug("Text::startCursorEdit(): edit styled text\n");
            return 0;
            }
      if (_cursor) {
            qDebug("Text::startCursorEdit(): cursor already active\n");
            return 0;
            }
      _cursor = new QTextCursor(_doc);
      return _cursor;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
      {
      _editMode = false;
      endCursorEdit();
      layoutEdit();
      textChanged();
      }

//---------------------------------------------------------
//   endCursorEdit
//---------------------------------------------------------

void Text::endCursorEdit()
      {
      delete _cursor;
      _cursor = 0;
      if (styled()) {
            SimpleText::setText(_doc->toPlainText());
            delete _doc;
            _doc = 0;
            }
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Text::isEmpty() const
      {
      return styled() ? SimpleText::getText().isEmpty() : _doc->isEmpty();
      }

//---------------------------------------------------------
//   setModified
//---------------------------------------------------------

void Text::setModified(bool v)
      {
      if (!styled())
            _doc->setModified(v);
      }

//---------------------------------------------------------
//   getFragment
//---------------------------------------------------------

QTextDocumentFragment Text::getFragment() const
      {
      if (styled())
            return QTextDocumentFragment::fromPlainText(getText());
      else
            return QTextDocumentFragment(_doc);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Text::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_TEXT_STYLE:
                  return QVariant(_styleIndex);
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
      switch(propertyId) {
            case P_TEXT_STYLE:
                  _styleIndex = v.toInt();
                  setGenerated(false);
                  break;
            default:
                  rv = Element::setProperty(propertyId, v);
                  break;
            }
      score()->setLayoutAll(true);
      return rv;
      }

