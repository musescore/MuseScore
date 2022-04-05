/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "textedit.h"

#include "mscoreview.h"
#include "score.h"
#include "scorefont.h"
#include "types/symnames.h"

#include "accessibility/accessibleitem.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   ~TextEditData
//---------------------------------------------------------

TextEditData::~TextEditData()
{
    if (deleteText) {
        for (EngravingObject* se : _textBase->linkList()) {
            toTextBase(se)->deleteLater();
        }
    }
}

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

TextCursor* TextEditData::cursor() const
{
    IF_ASSERT_FAILED(_textBase) {
        return nullptr;
    }

    return _textBase->cursor();
}

//---------------------------------------------------------
//   editInsertText
//---------------------------------------------------------

void TextBase::editInsertText(TextCursor* cursor, const QString& s)
{
    Q_ASSERT(!layoutInvalid);
    textInvalid = true;

    int col = 0;
    for (const QChar& c : s) {
        if (!c.isHighSurrogate()) {
            ++col;
        }
    }
    cursor->curLine().insert(cursor, s);
    cursor->setColumn(cursor->column() + col);
    cursor->clearSelection();

    triggerLayout();
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void TextBase::startEdit(EditData& ed)
{
    std::shared_ptr<TextEditData> ted = std::make_shared<TextEditData>(this);
    ted->e = this;
    ted->cursor()->startEdit();

    Q_ASSERT(!score()->undoStack()->active());        // make sure we are not in a Cmd

    ted->oldXmlText = xmlText();
    ted->startUndoIdx = score()->undoStack()->getCurIdx();

    if (layoutInvalid) {
        layout();
    }
    if (!ted->cursor()->set(ed.startMove)) {
        resetFormatting();
    }
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
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    IF_ASSERT_FAILED(ted && ted->cursor()) {
        return;
    }

    ted->cursor()->endEdit();

    UndoStack* undo = score()->undoStack();
    IF_ASSERT_FAILED(undo) {
        return;
    }

    const QString actualXmlText = xmlText();
    const QString actualPlainText = plainText();

    // replace all undo/redo records collected during text editing with
    // one property change

    using Filter = UndoCommand::Filter;
    const bool textWasEdited = (undo->getCurIdx() != ted->startUndoIdx);

    if (textWasEdited) {
        undo->mergeCommands(ted->startUndoIdx);
        undo->last()->filterChildren(Filter::TextEdit, this);
    } else {
        // No text changes in "undo" part of undo stack,
        // hence nothing to merge and filter.
        undo->cleanRedoStack();     // prevent text editing commands from remaining in undo stack
    }

    bool newlyAdded = false;

    if (ted->oldXmlText.isEmpty()) {
        UndoCommand* ucmd = textWasEdited ? undo->prev() : undo->last();
        if (ucmd && ucmd->hasFilteredChildren(Filter::AddElement, this)) {
            // We have just added this element to a score.
            // Combine undo records of text creation with text editing.
            newlyAdded = true;
            undo->mergeCommands(ted->startUndoIdx - 1);
        }
    }

    // TBox'es manage their Text themselves and are not removed if text is empty
    const bool removeTextIfEmpty = !(explicitParent() && explicitParent()->isTBox());

    if (actualPlainText.isEmpty() && removeTextIfEmpty) {
        qDebug("actual text is empty");

        // If this assertion fails, no undo command relevant to this text
        // resides on undo stack and reopen() would corrupt the previous
        // command. Text shouldn't happen to be empty in other cases though.
        Q_ASSERT(newlyAdded || textWasEdited);

        undo->reopen();
        score()->undoRemoveElement(this);
        ed.element = 0;

        static const std::vector<Filter> filters {
            Filter::AddElementLinked,
            Filter::RemoveElementLinked,
            Filter::ChangePropertyLinked,
            Filter::Link,
        };

        if (newlyAdded && !undo->current()->hasUnfilteredChildren(filters, this)) {
            for (Filter f : filters) {
                undo->current()->filterChildren(f, this);
            }

            ted->setDeleteText(true);       // mark this text element for deletion
        }

        commitText();
        return;
    }

    if (textWasEdited) {
        setXmlText(ted->oldXmlText);                        // reset text to value before editing
        undo->reopen();
        resetFormatting();
        undoChangeProperty(Pid::TEXT, actualXmlText);       // change property to set text to actual value again
                                                            // this also changes text of linked elements
        layout1();
        triggerLayout();                                    // force relayout even if text did not change
    } else {
        triggerLayout();
    }

    static const qreal w = 2.0;
    score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));

    commitText();
}

void TextBase::commitText()
{
    score()->endCmd();
}

//---------------------------------------------------------
//   insertSym
//---------------------------------------------------------

void TextBase::insertSym(EditData& ed, SymId id)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();

    deleteSelectedText(ed);
    QString s = score()->scoreFont()->toString(id);
    CharFormat fmt = *cursor->format();    // save format
    cursor->format()->setFontFamily("ScoreText");
    score()->undo(new InsertText(_cursor, s), &ed);
    cursor->setFormat(fmt);    // restore format
}

//---------------------------------------------------------
//   insertText
//---------------------------------------------------------

void TextBase::insertText(EditData& ed, const QString& s)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    score()->undo(new InsertText(cursor, s), &ed);
}

bool TextBase::isEditAllowed(EditData& ed) const
{
    if (ed.key == Qt::Key_Shift || ed.key == Qt::Key_Escape) {
        return false;
    }

    bool ctrlPressed  = ed.modifiers & Qt::ControlModifier;
    bool shiftPressed = ed.modifiers & Qt::ShiftModifier;

    if (ctrlPressed && !shiftPressed) {
        static QSet<int> ignore = {
            Qt::Key_B,
            Qt::Key_C,
            Qt::Key_I,
            Qt::Key_U,
            Qt::Key_V,
            Qt::Key_X,
            Qt::Key_Y,
            Qt::Key_Z
        };

        return !ignore.contains(ed.key);
    }

    return true;
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextBase::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    if (!ted) {
        return false;
    }
    TextCursor* cursor = ted->cursor();

    QString s         = ed.s;
    bool ctrlPressed  = ed.modifiers & Qt::ControlModifier;
    bool shiftPressed = ed.modifiers & Qt::ShiftModifier;
    bool altPressed = ed.modifiers & Qt::AltModifier;

    TextCursor::MoveMode mm = shiftPressed ? TextCursor::MoveMode::KeepAnchor : TextCursor::MoveMode::MoveAnchor;

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
        } else if (ed.modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
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
//printf("======%x\n", s.isEmpty() ? -1 : s[0].unicode());

        switch (ed.key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            deleteSelectedText(ed);
            score()->undo(new SplitText(cursor), &ed);

            notifyAboutTextCursorChanged();

            return true;

        case Qt::Key_Delete:
            if (!deleteSelectedText(ed)) {
                // check for move down
                if (cursor->column() == cursor->columns()) {               // if you are on the end of the line, delete the newline char
                    int cursorRow = cursor->row();
                    cursor->movePosition(TextCursor::MoveOperation::Down);
                    if (cursor->row() != cursorRow) {
                        cursor->movePosition(TextCursor::MoveOperation::StartOfLine);
                        score()->undo(new JoinText(cursor), &ed);
                    }
                } else {
                    score()->undo(new RemoveText(cursor, QString(cursor->currentCharacter())), &ed);
                }

//                notifyAboutTextRemoved() // todo
            }
            return true;

        case Qt::Key_Backspace: {
            int startPosition = cursor->currentPosition();

            if (ctrlPressed) {
                // delete last word
                cursor->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::KeepAnchor);
                int endPosition = cursor->currentPosition();
                QString text = cursor->selectedText();

                s.clear();

                if (deleteSelectedText(ed)) {
                    notifyAboutTextRemoved(startPosition, endPosition, text);
                }
            } else {
                QString text = cursor->selectedText();

                if (!deleteSelectedText(ed)) {
                    if (cursor->column() == 0 && _cursor->row() != 0) {
                        score()->undo(new JoinText(cursor), &ed);
                    } else {
                        if (!cursor->movePosition(TextCursor::MoveOperation::Left)) {
                            return false;
                        }
                        score()->undo(new RemoveText(cursor, QString(cursor->currentCharacter())), &ed);
                    }
                } else {
                    notifyAboutTextRemoved(startPosition, startPosition - 1, text);
                }
            }
            return true;
        }

        case Qt::Key_Left:
            if (!_cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::WordLeft : TextCursor::MoveOperation::Left,
                                       mm) && type() == ElementType::LYRICS) {
                return false;
            }
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_Right:
            if (!_cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::NextWord : TextCursor::MoveOperation::Right,
                                       mm) && type() == ElementType::LYRICS) {
                return false;
            }
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_Up:
#if defined(Q_OS_MAC)
            if (!cursor->movePosition(TextCursor::MoveOperation::Up, mm)) {
                cursor->movePosition(TextCursor::MoveOperation::StartOfLine, mm);
            }
#else
            cursor->movePosition(TextCursor::MoveOperation::Up, mm);
#endif
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_Down:
#if defined(Q_OS_MAC)
            if (!cursor->movePosition(TextCursor::MoveOperation::Down, mm)) {
                cursor->movePosition(TextCursor::MoveOperation::EndOfLine, mm);
            }
#else
            cursor->movePosition(TextCursor::MoveOperation::Down, mm);
#endif
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_Home:
            if (ctrlPressed) {
                cursor->movePosition(TextCursor::MoveOperation::Start, mm);
            } else {
                cursor->movePosition(TextCursor::MoveOperation::StartOfLine, mm);
            }

            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_End:
            if (ctrlPressed) {
                cursor->movePosition(TextCursor::MoveOperation::End, mm);
            } else {
                cursor->movePosition(TextCursor::MoveOperation::EndOfLine, mm);
            }

            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Qt::Key_Tab:
            s = " ";
            ed.modifiers = {};
            break;

        case Qt::Key_Space:
            if (ed.modifiers & CONTROL_MODIFIER) {
                s = QString(QChar(0xa0));               // non-breaking space
            } else {
                if (isFingering() && ed.view()) {
                    score()->endCmd();
                    ed.view()->textTab(ed.modifiers & Qt::ShiftModifier);
                    return true;
                }
                s = " ";
            }
            ed.modifiers = {};
            break;

        case Qt::Key_Minus:
            if (ed.modifiers == 0) {
                s = "-";
            }
            break;

        case Qt::Key_Underscore:
            if (ed.modifiers == 0) {
                s = "_";
            }
            break;

        case Qt::Key_A:
            if (ctrlPressed && !shiftPressed) {
                cursor->movePosition(TextCursor::MoveOperation::Start, TextCursor::MoveMode::MoveAnchor);
                cursor->movePosition(TextCursor::MoveOperation::End, TextCursor::MoveMode::KeepAnchor);
                s.clear();

                notifyAboutTextCursorChanged();
            }
            break;
        case Qt::Key_B:
        case Qt::Key_C:
        case Qt::Key_I:
        case Qt::Key_U:
        case Qt::Key_V:
        case Qt::Key_X:
        case Qt::Key_Y:
        case Qt::Key_Z:
            if (ctrlPressed && !shiftPressed) {
                return false; // handled at application level
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
                s = "\u266d";               // Unicode flat
                break;
            case Qt::Key_NumberSign: // e.g. QWERTY (US)
            case Qt::Key_AsciiTilde: // e.g. QWERTY (GB)
            case Qt::Key_Apostrophe: // e.g. QWERTZ (DE)
            case Qt::Key_periodcentered: // e.g. QWERTY (ES)
            case Qt::Key_3: // e.g. AZERTY (FR, BE)
                s = "\u266f";               // Unicode sharp
                break;
            case Qt::Key_H:
                s = "\u266e";               // Unicode natural
                break;
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
            if (ctrlPressed && altPressed) {
                if (ed.key == Qt::Key_Minus) {
                    insertSym(ed, SymId::lyricsElision);
                    return true;
                }
            }
        }
    }
    if (!s.isEmpty()) {
        deleteSelectedText(ed);
        score()->undo(new InsertText(_cursor, s), &ed);

        int startPosition = cursor->currentPosition();
        notifyAboutTextInserted(startPosition, startPosition + s.size(), s);
    }
    return true;
}

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

void TextBase::movePosition(EditData& ed, TextCursor::MoveOperation op)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    cursor->movePosition(op);
    score()->addRefresh(canvasBoundingRect());
    score()->update();
}

//---------------------------------------------------------
//  ChangeText::insertText
//---------------------------------------------------------

void ChangeText::insertText(EditData* ed)
{
    TextCursor tc = _cursor;
    tc.text()->editInsertText(&tc, s);
    if (ed) {
        TextCursor* ttc = tc.text()->cursorFromEditData(*ed);
        *ttc = tc;
    }
}

//---------------------------------------------------------
//  ChangeText::removeText
//---------------------------------------------------------

void ChangeText::removeText(EditData* ed)
{
    TextCursor tc = _cursor;
    TextBlock& l  = _cursor.curLine();
    int column    = _cursor.column();

    for (int n = 0; n < s.size(); ++n) {
        l.remove(column, &_cursor);
    }
    _cursor.text()->triggerLayout();
    if (ed) {
        *_cursor.text()->cursorFromEditData(*ed) = tc;
    }
    _cursor.text()->setTextInvalid();
}

//---------------------------------------------------------
//   SplitJoinText
//---------------------------------------------------------

void SplitJoinText::join(EditData* ed)
{
    TextBase* t   = _cursor.text();
    int line      = _cursor.row();
    t->setTextInvalid();
    t->triggerLayout();

    CharFormat* charFmt = _cursor.format();         // take current format
    int col             = t->textBlock(line - 1).columns();
    int eol             = t->textBlock(line).eol();
    auto fragmentsList = t->textBlock(line).fragmentsWithoutEmpty();

    if (fragmentsList->size() > 0) {
        t->textBlock(line - 1).removeEmptyFragment();
    }
    t->textBlock(line - 1).fragments().append(*fragmentsList);
    delete fragmentsList;

    t->textBlockList().removeAt(line);

    _cursor.setRow(line - 1);
    _cursor.curLine().setEol(eol);
    _cursor.setColumn(col);
    _cursor.setFormat(*charFmt);             // restore orig. format at new line
    _cursor.clearSelection();

    if (ed) {
        *t->cursorFromEditData(*ed) = _cursor;
    }
    _cursor.text()->setTextInvalid();
}

void SplitJoinText::split(EditData* ed)
{
    TextBase* t   = _cursor.text();
    int line      = _cursor.row();
    bool eol      = _cursor.curLine().eol();
    t->setTextInvalid();
    t->triggerLayout();

    CharFormat* charFmt = _cursor.format();           // take current format
    t->textBlockList().insert(line + 1, _cursor.curLine().split(_cursor.column(), t->cursorFromEditData(*ed)));
    _cursor.curLine().setEol(true);

    _cursor.setRow(line + 1);
    _cursor.curLine().setEol(eol);
    _cursor.setColumn(0);
    _cursor.setFormat(*charFmt);               // restore orig. format at new line
    _cursor.clearSelection();

    if (ed) {
        *t->cursorFromEditData(*ed) = _cursor;
    }
    _cursor.text()->setTextInvalid();
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* TextBase::drop(EditData& ed)
{
    TextCursor* cursor = cursorFromEditData(ed);

    EngravingItem* e = ed.dropElement;

    switch (e->type()) {
    case ElementType::SYMBOL:
    {
        SymId id = toSymbol(e)->sym();
        delete e;

        insertSym(ed, id);
    }
    break;

    case ElementType::FSYMBOL:
    {
        uint code = toFSymbol(e)->code();
        QString s = QString::fromUcs4(&code, 1);
        delete e;

        deleteSelectedText(ed);
        score()->undo(new InsertText(cursor, s), &ed);
    }
    break;

    default:
        qDebug("drop <%s> not handled", e->typeName());
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void TextBase::paste(EditData& ed, const QString& txt)
{
    if (MScore::debugMode) {
        qDebug("<%s>", qPrintable(txt));
    }

    int state = 0;
    QString token;
    QString sym;
    bool symState = false;
    Ms::CharFormat format = *static_cast<TextEditData*>(ed.getData(this).get())->cursor()->format();

    score()->startCmd();
    for (int i = 0; i < txt.length(); i++) {
        QChar c = txt[i];
        if (state == 0) {
            if (c == '<') {
                state = 1;
                token.clear();
            } else if (c == '&') {
                state = 2;
                token.clear();
            } else {
                if (symState) {
                    sym += c;
                } else {
                    deleteSelectedText(ed);
                    static_cast<TextEditData*>(ed.getData(this).get())->cursor()->setFormat(format);
                    if (c.isHighSurrogate()) {
                        QChar highSurrogate = c;
                        Q_ASSERT(i + 1 < txt.length());
                        i++;
                        QChar lowSurrogate = txt[i];
                        insertText(ed, QString(QChar::surrogateToUcs4(highSurrogate, lowSurrogate)));
                    } else {
                        insertText(ed, QString(QChar(c.unicode())));
                    }
                }
            }
        } else if (state == 1) {
            if (c == '>') {
                state = 0;
                if (token == "sym") {
                    symState = true;
                    sym.clear();
                } else if (token == "/sym") {
                    symState = false;
                    insertSym(ed, SymNames::symIdByName(sym));
                } else {
                    prepareFormat(token, format);
                }
            } else {
                token += c;
            }
        } else if (state == 2) {
            if (c == ';') {
                state = 0;
                if (token == "lt") {
                    insertText(ed, "<");
                } else if (token == "gt") {
                    insertText(ed, ">");
                } else if (token == "amp") {
                    insertText(ed, "&");
                } else if (token == "quot") {
                    insertText(ed, "\"");
                } else {
                    insertSym(ed, SymNames::symIdByName(token));
                }
            } else if (!c.isLetter()) {
                state = 0;
                insertText(ed, "&");
                insertText(ed, token);
                insertText(ed, c);
            } else {
                token += c;
            }
        }
    }
    if (state == 2) {
        insertText(ed, "&");
        insertText(ed, token);
    }
    score()->endCmd();
}

//---------------------------------------------------------
//   endHexState
//---------------------------------------------------------

void TextBase::endHexState(EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();

    if (hexState >= 0) {
        if (hexState > 0) {
            int c2 = cursor->column();
            int c1 = c2 - (hexState + 1);

            TextBlock& t = _layout[cursor->row()];
            QString ss   = t.remove(c1, hexState + 1, cursor);
            bool ok;
            int code     = ss.mid(1).toInt(&ok, 16);
            cursor->setColumn(c1);
            cursor->clearSelection();
            if (ok) {
                editInsertText(cursor, QString(code));
            } else {
                qDebug("cannot convert hex string <%s>, state %d (%d-%d)",
                       qPrintable(ss.mid(1)), hexState, c1, c2);
            }
        }
        hexState = -1;
    }
}

//---------------------------------------------------------
//   deleteSelectedText
//---------------------------------------------------------

bool TextBase::deleteSelectedText(EditData& ed)
{
    TextCursor* cursor = cursorFromEditData(ed);

    if (!cursor->hasSelection()) {
        return false;
    }

    int r1 = cursor->selectLine();
    int c1 = cursor->selectColumn();

    if (r1 > cursor->row() || (r1 == cursor->row() && c1 > cursor->column())) {
        // swap start end of selection
        r1 = cursor->row();
        c1 = cursor->column();
        cursor->setRow(_cursor->selectLine());
        cursor->setColumn(cursor->selectColumn());
    }

    cursor->clearSelection();
    for (;;) {
        if (r1 == cursor->row() && c1 == cursor->column()) {
            break;
        }
        if (cursor->column() == 0 && cursor->row() != 0) {
            score()->undo(new JoinText(cursor), &ed);
        } else {
            // move cursor left:
            if (!_cursor->movePosition(TextCursor::MoveOperation::Left)) {
                break;
            }
            Ms::TextCursor undoCursor(*_cursor);
            // can't rely on the cursor's current format as it doesn't preserve the special font "ScoreText"
            undoCursor.setFormat(*_layout[_cursor->row()].formatAt(_cursor->column()));
            score()->undo(new RemoveText(&undoCursor, QString(_cursor->currentCharacter())), &ed);
        }
    }
    return true;
}

//---------------------------------------------------------
//   ChangeTextProperties
//---------------------------------------------------------

void ChangeTextProperties::restoreSelection()
{
    TextCursor& tc = cursor();
    tc.text()->cursor()->setSelectLine(tc.selectLine());
    tc.text()->cursor()->setSelectColumn(tc.selectColumn());
    tc.text()->cursor()->setRow(tc.row());
    tc.text()->cursor()->setColumn(tc.column());
}

ChangeTextProperties::ChangeTextProperties(const TextCursor* tc, Ms::Pid propId, const PropertyValue& propVal, PropertyFlags flags_)
    : TextEditUndoCommand(*tc)
{
    propertyId = propId;
    propertyVal = propVal;
    flags = flags_;
    if (propertyId == Pid::FONT_STYLE) {
        existingStyle = static_cast<FontStyle>(cursor().text()->getProperty(propId).toInt());
    }
}

void ChangeTextProperties::undo(EditData*)
{
    cursor().text()->resetFormatting();
    cursor().text()->setXmlText(xmlText);
    restoreSelection();
    cursor().text()->layout1();
}

void ChangeTextProperties::redo(EditData*)
{
    xmlText = cursor().text()->xmlText();
    restoreSelection();
    cursor().text()->setPropertyFlags(propertyId, flags);

    if (propertyId == Pid::FONT_STYLE) {
        FontStyle setStyle = static_cast<FontStyle>(propertyVal.toInt());
        TextCursor* tc = cursor().text()->cursor();
        // user turned on bold/italic/underline/strike for text where it's not set, or turned it off for text where it is set,
        // note this logic only works because the user can only click one at a time
        if ((setStyle& FontStyle::Bold) != (existingStyle & FontStyle::Bold)) {
            tc->setFormat(FormatId::Bold, setStyle & FontStyle::Bold);
        }
        if ((setStyle& FontStyle::Italic) != (existingStyle & FontStyle::Italic)) {
            tc->setFormat(FormatId::Italic, setStyle & FontStyle::Italic);
        }
        if ((setStyle& FontStyle::Underline) != (existingStyle & FontStyle::Underline)) {
            tc->setFormat(FormatId::Underline, setStyle & FontStyle::Underline);
        }
        if ((setStyle& FontStyle::Strike) != (existingStyle & FontStyle::Strike)) {
            tc->setFormat(FormatId::Strike, setStyle & FontStyle::Strike);
        }
    } else {
        cursor().text()->setProperty(propertyId, propertyVal);
    }
}
}  // namespace Ms
