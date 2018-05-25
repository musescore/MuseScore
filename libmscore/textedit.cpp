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
//   startEdit
//---------------------------------------------------------

void TextBase::startEdit(EditData& ed)
      {
      ed.grips = 0;
      TextEditData* ted = new TextEditData(this);
      ted->e      = this;
      ted->cursor.setRow(0);
      ted->cursor.setColumn(0);
      ted->cursor.clearSelection();

      ted->oldXmlText = xmlText();
      ted->startUndoIdx = score()->undoStack()->getCurIdx();

      if (!ted->cursor.set(ed.startMove))
            ted->cursor.init();
      ed.addData(ted);
      if (layoutInvalid)
            layout();
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
      setXmlText(ted->oldXmlText);                    // reset text to value before editing
      score()->startCmd();
      undoChangeProperty(Pid::TEXT, actualText);      // change property to set text to actual value again
                                                      // this also changes text of linked elements
      score()->endCmd();

      static const qreal w = 2.0;
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
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
            // printf("======%x\n", s.isEmpty() ? -1 : s[0].unicode());

            switch (ed.key) {
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
                                    score()->undo(new RemoveText(_cursor, QString(_cursor->currentCharacter())));
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
//  ChangeText::insertText
//---------------------------------------------------------

void ChangeText::insertText(EditData* ed)
      {
      TextCursor tc = c;
      c.text()->editInsertText(&tc, s);
      c.text()->triggerLayout();
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
      }

//---------------------------------------------------------
//   SplitText - undo
//---------------------------------------------------------

void SplitText::undo(EditData* ed)
      {
      TextCursor tc = c;
      tc.deleteChar();
      c.text()->triggerLayout();
      if (ed)
            *c.text()->cursor(*ed) = tc;
      }

//---------------------------------------------------------
//   SplitText - redo
//---------------------------------------------------------

void SplitText::redo(EditData* ed)
      {
      TextCursor tc       = c;
      TextBase* t         = c.text();
      int line            = c.row();
      CharFormat* charFmt = c.format();         // take current format

      t->textBlockList().insert(line + 1, tc.curLine().split(tc.column()));

      t->textBlock(line).setEol(true);
      t->setTextInvalid();
      if (t->textBlockList().last() != t->textBlock(line+1))
            t->textBlock(line+1).setEol(true);

      c.text()->triggerLayout();
      if (ed) {
            tc.setRow(line+1);
            tc.setColumn(0);
            tc.setFormat(*charFmt);             // restore orig. format at new line
            tc.clearSelection();
            TextEditData* ted = static_cast<TextEditData*>(ed->getData(t));
            ted->cursor = tc;
            }
      }

}  // namespace Ms

