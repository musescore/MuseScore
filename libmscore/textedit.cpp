//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "textedit.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   editInsertText
//---------------------------------------------------------

void TextBase::editInsertText(TextCursor* cursor, const QString& s)
      {
      Q_ASSERT(!layoutInvalid);
      textInvalid = true;

      cursor->curLine().insert(cursor, s);
      cursor->setColumn(cursor->column() + s.size());
      cursor->clearSelection();

      triggerLayout();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void TextBase::startEdit(EditData& ed)
      {
      ed.grips = 0;
      TextEditData* ted = new TextEditData(this);
      ted->e = this;
      ted->cursor.setRow(0);
      ted->cursor.setColumn(0);
      ted->cursor.clearSelection();

      Q_ASSERT(!score()->undoStack()->active());      // make sure we are not in a Cmd

      ted->oldXmlText = xmlText();
      ted->startUndoIdx = score()->undoStack()->getCurIdx();

      if (layoutInvalid)
            layout();
      if (!ted->cursor.set(ed.startMove))
            ted->cursor.init();
      qreal _spatium = spatium();
      // refresh edit bounding box
      score()->addRefresh(canvasBoundingRect().adjusted(-_spatium, -_spatium, _spatium, _spatium));
      ed.addData(ted);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextBase::endEdit(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      score()->undoStack()->remove(ted->startUndoIdx);           // remove all undo/redo records

      // replace all undo/redo records collected during text editing with
      // one property change

      QString actualText = xmlText();
      if (ted->oldXmlText.isEmpty()) {
            UndoStack* us = score()->undoStack();
            UndoCommand* ucmd = us->last();
            if (ucmd) {
                  const QList<UndoCommand*>& cl = ucmd->commands();
                  const UndoCommand* cmd = cl.back();
                  if (strncmp(cmd->name(), "Add:", 4) == 0) {
                        const AddElement* ae = static_cast<const AddElement*>(cmd);
                        if (ae->getElement() == this) {
                              if (actualText.isEmpty()) {
                                    // we just created this empty text, rollback that operation
                                    us->rollback();
                                    score()->update();
                                    ed.element = 0;
                                    }
                              else {
                                    setXmlText(ted->oldXmlText);  // reset text to value before editing
                                    us->reopen();
                                    // combine undo records of text creation with text editing
                                    undoChangeProperty(Pid::TEXT, actualText);
                                    layout1();
                                    score()->endCmd();
                                    }
                              return;
                              }
                        }
                  }
            }
      if (actualText.isEmpty()) {
            qDebug("actual text is empty");
            score()->startCmd();
            score()->undoRemoveElement(this);
            ed.element = 0;
            score()->endCmd();
            return;
            }
      setXmlText(ted->oldXmlText);                    // reset text to value before editing
      score()->startCmd();
      undoChangeProperty(Pid::TEXT, actualText);      // change property to set text to actual value again
                                                      // this also changes text of linked elements
      layout1();
      score()->endCmd();

      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   insertSym
//---------------------------------------------------------

void TextBase::insertSym(EditData& ed, SymId id)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;

      deleteSelectedText(ed);
      QString s = score()->scoreFont()->toString(id);

      CharFormat fmt = *_cursor->format();  // save format
//      uint code = ScoreFont::fallbackFont()->sym(id).code();
      _cursor->format()->setFontFamily("ScoreText");
      _cursor->format()->setBold(false);
      _cursor->format()->setItalic(false);
      score()->undo(new InsertText(_cursor, s), &ed);
      _cursor->setFormat(fmt);  // restore format
      }

//---------------------------------------------------------
//   insertText
//---------------------------------------------------------

void TextBase::insertText(EditData& ed, const QString& s)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;
      score()->undo(new InsertText(_cursor, s), &ed);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextBase::edit(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;

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
                          case Qt::Key_0:
                          case Qt::Key_1:
                          case Qt::Key_2:
                          case Qt::Key_3:
                          case Qt::Key_4:
                          case Qt::Key_5:
                          case Qt::Key_6:
                          case Qt::Key_7:
                          case Qt::Key_8:
                          case Qt::Key_9:
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
                        case Qt::Key_A:
                        case Qt::Key_B:
                        case Qt::Key_C:
                        case Qt::Key_D:
                        case Qt::Key_E:
                        case Qt::Key_F:
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
            // printf("======%x\n", s.isEmpty() ? -1 : s[0].unicode());

            switch (ed.key) {
                  case Qt::Key_Z:         // happens when the undo stack is empty
                        return true;

                  case Qt::Key_Enter:
                  case Qt::Key_Return:
                        deleteSelectedText(ed);
                        score()->undo(new SplitText(_cursor), &ed);
                        return true;

                  case Qt::Key_Delete:
                        if (!deleteSelectedText(ed))
                              score()->undo(new RemoveText(_cursor, QString(_cursor->currentCharacter())), &ed);
                        return true;

                  case Qt::Key_Backspace:
                        if (!deleteSelectedText(ed)) {
                              if (_cursor->column() == 0 && _cursor->row() != 0)
                                    score()->undo(new JoinText(_cursor), &ed);
                              else {
                                    if (!_cursor->movePosition(QTextCursor::Left))
                                          return false;
                                    score()->undo(new RemoveText(_cursor, QString(_cursor->currentCharacter())), &ed);
                                    }
                              }
                        return true;

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
#if defined(Q_OS_MAC)
                        if (!_cursor->movePosition(QTextCursor::Up, mm))
                              _cursor->movePosition(QTextCursor::StartOfLine, mm);
#else
                        _cursor->movePosition(QTextCursor::Up, mm);
#endif
                        s.clear();
                        break;

                  case Qt::Key_Down:
#if defined(Q_OS_MAC)
                        if (!_cursor->movePosition(QTextCursor::Down, mm))
                              _cursor->movePosition(QTextCursor::EndOfLine, mm);
#else
                        _cursor->movePosition(QTextCursor::Down, mm);
#endif
                        s.clear();
                        break;

                  case Qt::Key_Home:
                        if (ctrlPressed)
                              _cursor->movePosition(QTextCursor::Start, mm);
                        else
                              _cursor->movePosition(QTextCursor::StartOfLine, mm);

                        s.clear();
                        break;

                  case Qt::Key_End:
                        if (ctrlPressed)
                              _cursor->movePosition(QTextCursor::End, mm);
                        else
                              _cursor->movePosition(QTextCursor::EndOfLine, mm);

                        s.clear();
                        break;

                  case Qt::Key_Tab:
                        s = " ";
                        ed.modifiers = 0;
                        break;

                  case Qt::Key_Space:
                        if (ed.modifiers & CONTROL_MODIFIER)
                              s = QString(QChar(0xa0)); // non-breaking space
                        else
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
                              insertSym(ed, SymId::accidentalFlat);
                              return true;
                        case Qt::Key_NumberSign:
                              insertSym(ed, SymId::accidentalSharp);
                              return true;
                        case Qt::Key_H:
                              insertSym(ed, SymId::accidentalNatural);
                              return true;
                        case Qt::Key_Space:
                              insertSym(ed, SymId::space);
                              return true;
                        case Qt::Key_F:
                              insertSym(ed, SymId::dynamicForte);
                              return true;
                        case Qt::Key_M:
                              insertSym(ed, SymId::dynamicMezzo);
                              return true;
                        case Qt::Key_N:
                              insertSym(ed, SymId::dynamicNiente);
                              return true;
                        case Qt::Key_P:
                              insertSym(ed, SymId::dynamicPiano);
                              return true;
                        case Qt::Key_S:
                              insertSym(ed, SymId::dynamicSforzando);
                              return true;
                        case Qt::Key_R:
                              insertSym(ed, SymId::dynamicRinforzando);
                              return true;
                        case Qt::Key_Z:
                              // Ctrl+Z is normally "undo"
                              // but this code gets hit even if you are also holding Shift
                              // so Shift+Ctrl+Z works
                              insertSym(ed, SymId::dynamicZ);
                              return true;
                        }
                  }
            }
      if (!s.isEmpty()) {
            deleteSelectedText(ed);
            score()->undo(new InsertText(_cursor, s), &ed);
            }
      return true;
      }

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

void TextBase::movePosition(EditData& ed, QTextCursor::MoveOperation op)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;
      _cursor->movePosition(op);
      score()->addRefresh(canvasBoundingRect());
      score()->update();
      }

//---------------------------------------------------------
//  ChangeText::insertText
//---------------------------------------------------------

void ChangeText::insertText(EditData* ed)
      {
      TextCursor tc = c;
      c.text()->editInsertText(&tc, s);
      if (ed) {
            TextCursor* ttc = c.text()->cursor(*ed);
            *ttc = tc;
            }
      }

//---------------------------------------------------------
//  ChangeText::removeText
//---------------------------------------------------------

void ChangeText::removeText(EditData* ed)
      {
      TextCursor tc = c;
      TextBlock& l  = c.curLine();
      int column    = c.column();

      for (int n = 0; n < s.size(); ++n)
            l.remove(column);
      c.text()->triggerLayout();
      if (ed)
            *c.text()->cursor(*ed) = tc;
      c.text()->setTextInvalid();
      }

//---------------------------------------------------------
//   SplitJoinText
//---------------------------------------------------------

void SplitJoinText::join(EditData* ed)
      {
      TextBase* t   = c.text();
      int line      = c.row();
      t->setTextInvalid();
      t->triggerLayout();

      CharFormat* charFmt = c.format();         // take current format
      int col             = t->textBlock(line-1).columns();
      int eol             = t->textBlock(line).eol();
      t->textBlock(line-1).fragments().append(t->textBlock(line).fragments());
      int lines = t->rows();
      if (line < lines)
            t->textBlock(line).setEol(eol);
      t->textBlockList().removeAt(line);
      c.setRow(line-1);
      c.setColumn(col);
      c.setFormat(*charFmt);             // restore orig. format at new line
      c.clearSelection();
      if (ed)
            *t->cursor(*ed) = c;
      c.text()->setTextInvalid();
      }

void SplitJoinText::split(EditData* ed)
      {
      TextBase* t   = c.text();
      int line      = c.row();
      t->setTextInvalid();
      t->triggerLayout();

      CharFormat* charFmt = c.format();         // take current format
      t->textBlockList().insert(line + 1, c.curLine().split(c.column()));
      c.curLine().setEol(true);

      c.setRow(line+1);
      c.setColumn(0);
      c.setFormat(*charFmt);             // restore orig. format at new line
      c.clearSelection();

      if (ed)
            *t->cursor(*ed) = c;
      c.text()->setTextInvalid();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TextBase::drop(EditData& ed)
      {
      TextCursor* _cursor = cursor(ed);

      Element* e = ed.element;
      switch (e->type()) {
            case ElementType::SYMBOL:
                  {
                  SymId id = toSymbol(e)->sym();
                  delete e;

                  deleteSelectedText(ed);
                  insertSym(ed, id);
                  }
                  break;

            case ElementType::FSYMBOL:
                  {
                  uint code = toFSymbol(e)->code();
                  delete e;

                  QString s = QString::fromUcs4(&code, 1);

                  deleteSelectedText(ed);
                  score()->undo(new InsertText(_cursor, s), &ed);
                  }
                  break;

            default:
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void TextBase::paste(EditData& ed)
      {
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
      if (MScore::debugMode)
            qDebug("<%s>", qPrintable(txt));

      int state = 0;
      QString token;
      QString sym;
      bool symState = false;

      score()->startCmd();
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
                              deleteSelectedText(ed);
                              if (c.isHighSurrogate()) {
                                    QChar highSurrogate = c;
                                    Q_ASSERT(i + 1 < txt.length());
                                    i++;
                                    QChar lowSurrogate = txt[i];
                                    insertText(ed, QString(QChar::surrogateToUcs4(highSurrogate, lowSurrogate)));
                                    }
                              else {
                                    insertText(ed, QString(QChar(c.unicode())));
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
                              insertSym(ed, Sym::name2id(sym));
                              }
                        }
                  else
                        token += c;
                  }
            else if (state == 2) {
                  if (c == ';') {
                        state = 0;
                        if (token == "lt")
                              insertText(ed, "<");
                        else if (token == "gt")
                              insertText(ed, ">");
                        else if (token == "amp")
                              insertText(ed, "&");
                        else if (token == "quot")
                              insertText(ed, "\"");
                        else
                              insertSym(ed, Sym::name2id(token));
                        }
                  else if (!c.isLetter()) {
                        state = 0;
                        insertText(ed, "&");
                        insertText(ed, token);
                        insertText(ed, c);
                        }
                  else
                        token += c;
                  }
            }
      if (state == 2) {
            insertText(ed, "&");
            insertText(ed, token);
            }
      score()->endCmd();
      }

//---------------------------------------------------------
//   inputTransition
//    - preedit string should not influence then undo/redo stack
//    - commit string goes onto the undo/redo stack
//---------------------------------------------------------

void TextBase::inputTransition(EditData& ed, QInputMethodEvent* ie)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;

      // remove preedit string
      int n = preEdit.size();
      while (n--) {
            if (_cursor->movePosition(QTextCursor::Left)) {
                  TextBlock& l  = _cursor->curLine();
                   l.remove(_cursor->column());
                  _cursor->text()->triggerLayout();
                  _cursor->text()->setTextInvalid();
                  }
            }

      qDebug("<%s><%s> len %d start %d, preEdit size %d",
         qPrintable(ie->commitString()),
         qPrintable(ie->preeditString()),
         ie->replacementLength(), ie->replacementStart(), preEdit.size());

      if (!ie->commitString().isEmpty()) {
            _cursor->format()->setPreedit(false);
            score()->startCmd();
            insertText(ed, ie->commitString());
            score()->endCmd();
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
                                    qDebug("   attribute TextFormat: %d-%d", a.start, a.length);
                                    QTextFormat tf = a.value.value<QTextFormat>();
                                    }
                                    break;
                              case QInputMethodEvent::Cursor:
                                    qDebug("   attribute Cursor at %d", a.start);
                                    break;
                              default:
                                    qDebug("   attribute %d", a.type);
                              }
                        }
#endif
                  _cursor->format()->setPreedit(true);
                  _cursor->updateCursorFormat();
                  editInsertText(_cursor, preEdit);
                  setTextInvalid();
                  layout1();
                  }
            }
      ie->accept();
      }

//---------------------------------------------------------
//   endHexState
//---------------------------------------------------------

void TextBase::endHexState(EditData& ed)
      {
      TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
      TextCursor* _cursor = &ted->cursor;

      if (hexState >= 0) {
            if (hexState > 0) {
                  int c2 = _cursor->column();
                  int c1 = c2 - (hexState + 1);

                  TextBlock& t = _layout[_cursor->row()];
                  QString ss   = t.remove(c1, hexState + 1);
                  bool ok;
                  int code     = ss.mid(1).toInt(&ok, 16);
                  _cursor->setColumn(c1);
                  _cursor->clearSelection();
                  if (ok)
                        editInsertText(_cursor, QString(code));
                  else
                        qDebug("cannot convert hex string <%s>, state %d (%d-%d)",
                           qPrintable(ss.mid(1)), hexState, c1, c2);
                  }
            hexState = -1;
            }
      }

//---------------------------------------------------------
//   deleteSelectedText
//---------------------------------------------------------

bool TextBase::deleteSelectedText(EditData& ed)
      {
      TextCursor* _cursor = cursor(ed);

      if (!_cursor->hasSelection())
            return false;

      int r1 = _cursor->selectLine();
      int c1 = _cursor->selectColumn();

      if (r1 > _cursor->row() || (r1 == _cursor->row() && c1 > _cursor->column())) {
            // swap start end of selection
            r1 = _cursor->row();
            c1 = _cursor->column();
            _cursor->setRow(_cursor->selectLine());
            _cursor->setColumn(_cursor->selectColumn());
            }

      _cursor->clearSelection();
      for (;;) {
            if (r1 == _cursor->row() && c1 == _cursor->column())
                  break;
            if (_cursor->column() == 0 && _cursor->row() != 0)
                  score()->undo(new JoinText(_cursor), &ed);
            else {
                  // move cursor left:
                  if (!_cursor->movePosition(QTextCursor::Left))
                        break;
                  score()->undo(new RemoveText(_cursor, QString(_cursor->currentCharacter())), &ed);
                  }
            }
      return true;
      }

}  // namespace Ms

