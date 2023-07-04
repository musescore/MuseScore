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

#include "iengravingfont.h"
#include "types/symnames.h"

#include "mscoreview.h"
#include "navigate.h"
#include "score.h"
#include "lyrics.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
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

void TextBase::editInsertText(TextCursor* cursor, const String& s)
{
    assert(!m_layoutInvalid);
    m_textInvalid = true;

    int col = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (!s.at(i).isHighSurrogate()) {
            ++col;
        }
    }

    TextBlock& block = m_blocks[cursor->row()];
    const CharFormat* previousFormat = block.formatAt(std::max(int(cursor->column()) - 1, 0));
    if (previousFormat && previousFormat->fontFamily() == "ScoreText" && s == " ") {
        // This space would be ignored by the xml parser (see #15629)
        // We must use the nonBreaking space character instead
        String nonBreakingSpace = String(Char(0xa0));
        cursor->curLine().insert(cursor, nonBreakingSpace);
    } else {
        cursor->curLine().insert(cursor, s);
    }

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

    assert(!score()->undoStack()->active());        // make sure we are not in a Cmd

    ted->oldXmlText = xmlText();
    ted->startUndoIdx = score()->undoStack()->getCurIdx();

    if (m_layoutInvalid) {
        layout()->layoutItem(this);
    }
    if (!ted->cursor()->set(ed.startMove)) {
        resetFormatting();
    }
    double _spatium = spatium();
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

    const String actualXmlText = xmlText();
    const String actualPlainText = plainText();

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
        LOGD("actual text is empty");

        // If this assertion fails, no undo command relevant to this text
        // resides on undo stack and reopen() would corrupt the previous
        // command. Text shouldn't happen to be empty in other cases though.
        assert(newlyAdded || textWasEdited);

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
        if (isLyrics()) {
            Lyrics* prev = prevLyrics(toLyrics(this));
            if (prev) {
                prev->setRemoveInvalidSegments();
                layout()->layoutItem(prev);
            }
        }
        return;
    }

    if (textWasEdited) {
        setXmlText(ted->oldXmlText);                        // reset text to value before editing
        undo->reopen();
        resetFormatting();
        undoChangeProperty(Pid::TEXT, actualXmlText);       // change property to set text to actual value again
                                                            // this also changes text of linked elements
        layout()->layoutText1(this);
        triggerLayout();                                    // force relayout even if text did not change
    } else {
        triggerLayout();
    }

    static const double w = 2.0;
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
    String s = score()->engravingFont()->toString(id);
    cursor->format()->setFontFamily(u"ScoreText");
    score()->undo(new InsertText(m_cursor, s), &ed);
}

//---------------------------------------------------------
//   insertText
//---------------------------------------------------------

void TextBase::insertText(EditData& ed, const String& s)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    score()->undo(new InsertText(cursor, s), &ed);
}

bool TextBase::isEditAllowed(EditData& ed) const
{
    // Keep this method closely in sync with TextBase::edit()!

    if (ed.key == Key_Shift || ed.key == Key_Escape) {
        return false;
    }

    bool ctrlPressed  = ed.modifiers & ControlModifier;
    bool shiftPressed = ed.modifiers & ShiftModifier;
    bool altPressed = ed.modifiers & AltModifier;

    // Hex
    if (ed.modifiers == (ControlModifier | ShiftModifier | KeypadModifier)) {
        switch (ed.key) {
        case Key_0:
        case Key_1:
        case Key_2:
        case Key_3:
        case Key_4:
        case Key_5:
        case Key_6:
        case Key_7:
        case Key_8:
        case Key_9:
            return true;
        default: break;
        }
    } else if (ed.modifiers == (ControlModifier | ShiftModifier)) {
        switch (ed.key) {
        case Key_A:
        case Key_B:
        case Key_C:
        case Key_D:
        case Key_E:
        case Key_F:
            return true;
        default: break;
        }
    }

    switch (ed.key) {
    // Basic editing / navigation
    case Key_Enter:
    case Key_Return:
    case Key_Delete:
    case Key_Backspace:
    case Key_Left:
    case Key_Right:
    case Key_Up:
    case Key_Down:
    case Key_Home:
    case Key_End:
    case Key_Tab:
    case Key_Space:
    case Key_Minus:
    case Key_Underscore:
        return true;
    // Select all is handled by us
    case Key_A:
        if (ctrlPressed && !shiftPressed) {
            return true;
        }
        break;
    // Several commands
    // TODO: looks like we're hard-coding keyboard shortcuts here, but what if user has edited those?
    case Key_B:
    case Key_C:
    case Key_I:
    case Key_U:
    case Key_V:
    case Key_X:
    case Key_Y:
    case Key_Z:
        if (ctrlPressed && !shiftPressed) {
            return false; // handled at application level
        }
        break;
    default:
        break;
    }

    // Insert special characters
    if (ctrlPressed && shiftPressed) {
        switch (ed.key) {
        case Key_U:
        case Key_B:
        case Key_NumberSign: // e.g. QWERTY (US)
        case Key_AsciiTilde: // e.g. QWERTY (GB)
        case Key_Apostrophe: // e.g. QWERTZ (DE)
        case Key_periodcentered: // e.g. QWERTY (ES)
        case Key_3: // e.g. AZERTY (FR, BE)
        case Key_H:
        case Key_Space:
        case Key_F:
        case Key_M:
        case Key_N:
        case Key_P:
        case Key_S:
        case Key_R:
        case Key_Z:
            return true;
        }
    }
    if (ctrlPressed && altPressed) {
        if (ed.key == Key_Minus) {
            return true;
        }
    }

    // At least on non-macOS, sometimes ed.s is not empty even if Ctrl is pressed
    return !ctrlPressed && !ed.s.empty();
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextBase::edit(EditData& ed)
{
    // Keep this method closely in sync with TextBase::isEditAllowed()!

    if (!isEditAllowed(ed)) {
        return false;
    }

    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    if (!ted) {
        return false;
    }
    TextCursor* cursor = ted->cursor();
    CharFormat* currentFormat = cursor->format();

    String s         = ed.s;
    bool ctrlPressed  = ed.modifiers & ControlModifier;
    bool shiftPressed = ed.modifiers & ShiftModifier;
    bool altPressed = ed.modifiers & AltModifier;

    TextCursor::MoveMode mm = shiftPressed ? TextCursor::MoveMode::KeepAnchor : TextCursor::MoveMode::MoveAnchor;

    bool wasHex = false;
    if (m_hexState >= 0) {
        if (ed.modifiers == (ControlModifier | ShiftModifier | KeypadModifier)) {
            switch (ed.key) {
            case Key_0:
            case Key_1:
            case Key_2:
            case Key_3:
            case Key_4:
            case Key_5:
            case Key_6:
            case Key_7:
            case Key_8:
            case Key_9:
                s = Char::fromAscii(ed.key);
                ++m_hexState;
                wasHex = true;
                break;
            default:
                break;
            }
        } else if (ed.modifiers == (ControlModifier | ShiftModifier)) {
            switch (ed.key) {
            case Key_A:
            case Key_B:
            case Key_C:
            case Key_D:
            case Key_E:
            case Key_F:
                s = Char::fromAscii(ed.key);
                ++m_hexState;
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
        case Key_Enter:
        case Key_Return:
            deleteSelectedText(ed);
            score()->undo(new SplitText(cursor), &ed);

            notifyAboutTextCursorChanged();

            return true;

        case Key_Delete:
            if (!deleteSelectedText(ed)) {
                // check for move down
                if (cursor->column() == cursor->columns()) {               // if you are on the end of the line, delete the newline char
                    size_t cursorRow = cursor->row();
                    cursor->movePosition(TextCursor::MoveOperation::Down);
                    if (cursor->row() != cursorRow) {
                        cursor->movePosition(TextCursor::MoveOperation::StartOfLine);
                        score()->undo(new JoinText(cursor), &ed);
                    }
                } else {
                    score()->undo(new RemoveText(cursor, String(cursor->currentCharacter())), &ed);
                }

//                notifyAboutTextRemoved() // todo
            }
            return true;

        case Key_Backspace: {
            int startPosition = cursor->currentPosition();

            if (ctrlPressed) {
                // delete last word
                cursor->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::KeepAnchor);
                int endPosition = cursor->currentPosition();
                String text = cursor->selectedText();

                s.clear();

                if (deleteSelectedText(ed)) {
                    notifyAboutTextRemoved(startPosition, endPosition, text);
                }
            } else {
                String text = cursor->selectedText();

                if (!deleteSelectedText(ed)) {
                    if (cursor->column() == 0 && m_cursor->row() != 0) {
                        score()->undo(new JoinText(cursor), &ed);
                    } else {
                        if (!cursor->movePosition(TextCursor::MoveOperation::Left)) {
                            return false;
                        }
                        score()->undo(new RemoveText(cursor, String(cursor->currentCharacter())), &ed);
                    }
                } else {
                    notifyAboutTextRemoved(startPosition, startPosition - 1, text);
                }
            }
            return true;
        }

        case Key_Left:
            if (!m_cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::WordLeft : TextCursor::MoveOperation::Left,
                                        mm) && type() == ElementType::LYRICS) {
                return false;
            }
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Key_Right:
            if (!m_cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::NextWord : TextCursor::MoveOperation::Right,
                                        mm) && type() == ElementType::LYRICS) {
                return false;
            }
            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Key_Up:
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

        case Key_Down:
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

        case Key_Home:
            if (ctrlPressed) {
                cursor->movePosition(TextCursor::MoveOperation::Start, mm);
            } else {
                cursor->movePosition(TextCursor::MoveOperation::StartOfLine, mm);
            }

            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Key_End:
            if (ctrlPressed) {
                cursor->movePosition(TextCursor::MoveOperation::End, mm);
            } else {
                cursor->movePosition(TextCursor::MoveOperation::EndOfLine, mm);
            }

            s.clear();

            notifyAboutTextCursorChanged();

            break;

        case Key_Tab:
            s = u" ";
            ed.modifiers = {};
            break;

        case Key_Space:
            if ((ed.modifiers & TextEditingControlModifier) || currentFormat->fontFamily() == u"ScoreText") {
                s = String(Char(0xa0)); // non-breaking space
            } else {
                if (isFingering() && ed.view()) {
                    score()->endCmd();
                    ed.view()->textTab(ed.modifiers & ShiftModifier);
                    return true;
                }
                s = u" ";
            }
            ed.modifiers = {};
            break;

        case Key_Minus:
            if (ed.modifiers == 0) {
                s = u"-";
            }
            break;

        case Key_Underscore:
            if (ed.modifiers == 0) {
                s = u"_";
            }
            break;

        case Key_A:
            if (ctrlPressed && !shiftPressed) {
                cursor->movePosition(TextCursor::MoveOperation::Start, TextCursor::MoveMode::MoveAnchor);
                cursor->movePosition(TextCursor::MoveOperation::End, TextCursor::MoveMode::KeepAnchor);
                s.clear();

                notifyAboutTextCursorChanged();
            }
            break;
        case Key_B:
        case Key_C:
        case Key_I:
        case Key_U:
        case Key_V:
        case Key_X:
        case Key_Y:
        case Key_Z:
            if (ctrlPressed && !shiftPressed) {
                return false; // handled at application level
            }
            break;
        default:
            break;
        }
        if (ctrlPressed && shiftPressed) {
            switch (ed.key) {
            case Key_U:
                if (m_hexState == -1) {
                    m_hexState = 0;
                    s = u"u";
                }
                break;
            case Key_B:
                s = u"\u266d";               // Unicode flat
                break;
            case Key_NumberSign: // e.g. QWERTY (US)
            case Key_AsciiTilde: // e.g. QWERTY (GB)
            case Key_Apostrophe: // e.g. QWERTZ (DE)
            case Key_periodcentered: // e.g. QWERTY (ES)
            case Key_3: // e.g. AZERTY (FR, BE)
                s = u"\u266f";               // Unicode sharp
                break;
            case Key_H:
                s = u"\u266e";               // Unicode natural
                break;
            case Key_Space:
                insertSym(ed, SymId::space);
                return true;
            case Key_F:
                insertSym(ed, SymId::dynamicForte);
                return true;
            case Key_M:
                insertSym(ed, SymId::dynamicMezzo);
                return true;
            case Key_N:
                insertSym(ed, SymId::dynamicNiente);
                return true;
            case Key_P:
                insertSym(ed, SymId::dynamicPiano);
                return true;
            case Key_S:
                insertSym(ed, SymId::dynamicSforzando);
                return true;
            case Key_R:
                insertSym(ed, SymId::dynamicRinforzando);
                return true;
            case Key_Z:
                // Ctrl+Z is normally "undo"
                // but this code gets hit even if you are also holding Shift
                // so Shift+Ctrl+Z works
                insertSym(ed, SymId::dynamicZ);
                return true;
            }
        }
        if (ctrlPressed && altPressed) {
            if (ed.key == Key_Minus) {
                insertSym(ed, SymId::lyricsElision);
                return true;
            }
        }
    }
    if (!s.isEmpty()) {
        if (currentFormat->fontFamily() == u"ScoreText") {
            currentFormat->setFontFamily(propertyDefault(Pid::FONT_FACE).value<String>());
        }
        deleteSelectedText(ed);
        score()->undo(new InsertText(m_cursor, s), &ed);

        int startPosition = cursor->currentPosition();
        notifyAboutTextInserted(startPosition, startPosition + static_cast<int>(s.size()), s);
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
    tc.setFormat(format);                              // To undo TextCursor::updateCursorFormat()
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
    size_t column = _cursor.column();
    format = *l.formatAt(static_cast<int>(column + s.size()) - 1);

    for (size_t n = 0; n < s.size(); ++n) {
        l.remove(static_cast<int>(column), &_cursor);
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
    size_t line   = _cursor.row();
    t->setTextInvalid();
    t->triggerLayout();

    CharFormat* charFmt = _cursor.format();         // take current format
    size_t col          = t->textBlock(static_cast<int>(line) - 1).columns();
    int eol             = t->textBlock(static_cast<int>(line)).eol();
    auto fragmentsList = t->textBlock(static_cast<int>(line)).fragmentsWithoutEmpty();

    if (fragmentsList.size() > 0) {
        t->textBlock(static_cast<int>(line) - 1).removeEmptyFragment();
    }
    mu::join(t->textBlock(static_cast<int>(line) - 1).fragments(), fragmentsList);

    t->textBlockList().erase(t->textBlockList().begin() + line);

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
    size_t line   = _cursor.row();
    bool eol      = _cursor.curLine().eol();
    t->setTextInvalid();
    t->triggerLayout();

    CharFormat* charFmt = _cursor.format();           // take current format
    t->textBlockList().insert(t->textBlockList().begin() + line + 1,
                              _cursor.curLine().split(static_cast<int>(_cursor.column()), t->cursorFromEditData(*ed)));
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
        String s = toFSymbol(e)->toString();
        delete e;

        CharFormat* currentFormat = cursor->format();
        if (currentFormat->fontFamily() == u"ScoreText") {
            currentFormat->setFontFamily(propertyDefault(Pid::FONT_FACE).value<String>());
        }

        deleteSelectedText(ed);
        score()->undo(new InsertText(cursor, s), &ed);
    }
    break;

    default:
        LOGD("drop <%s> not handled", e->typeName());
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void TextBase::paste(EditData& ed, const String& txt)
{
    if (MScore::debugMode) {
        LOGD("<%s>", muPrintable(txt));
    }

    int state = 0;
    String token;
    String sym;
    bool symState = false;
    CharFormat format = *static_cast<TextEditData*>(ed.getData(this).get())->cursor()->format();

    score()->startCmd();
    for (size_t i = 0; i < txt.size(); i++) {
        Char c = txt.at(i);
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
                        Char highSurrogate = c;
                        assert(i + 1 < txt.size());
                        i++;
                        Char lowSurrogate = txt.at(i);
                        insertText(ed, String::fromUcs4(Char::surrogateToUcs4(highSurrogate, lowSurrogate)));
                    } else {
                        insertText(ed, String(c));
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
                    insertText(ed, u"<");
                } else if (token == "gt") {
                    insertText(ed, u">");
                } else if (token == "amp") {
                    insertText(ed, u"&");
                } else if (token == "quot") {
                    insertText(ed, u"\"");
                } else {
                    insertSym(ed, SymNames::symIdByName(token));
                }
            } else if (!c.isLetter()) {
                state = 0;
                insertText(ed, u"&");
                insertText(ed, token);
                insertText(ed, c);
            } else {
                token += c;
            }
        }
    }
    if (state == 2) {
        insertText(ed, u"&");
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

    if (m_hexState >= 0) {
        if (m_hexState > 0) {
            size_t c2 = cursor->column();
            size_t c1 = c2 - (m_hexState + 1);

            TextBlock& t = m_blocks[cursor->row()];
            String ss   = t.remove(static_cast<int>(c1), m_hexState + 1, cursor);
            bool ok;
            char16_t code = ss.mid(1).toInt(&ok, 16);
            cursor->setColumn(c1);
            cursor->clearSelection();
            if (ok) {
                editInsertText(cursor, String(code));
            } else {
                LOGD("cannot convert hex string <%s>, state %d (%zu-%zu)",
                     muPrintable(ss.mid(1)), m_hexState, c1, c2);
            }
        }
        m_hexState = -1;
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

    size_t r1 = cursor->selectLine();
    size_t c1 = cursor->selectColumn();

    if (r1 > cursor->row() || (r1 == cursor->row() && c1 > cursor->column())) {
        // swap start end of selection
        r1 = cursor->row();
        c1 = cursor->column();
        cursor->setRow(m_cursor->selectLine());
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
            if (!m_cursor->movePosition(TextCursor::MoveOperation::Left)) {
                break;
            }
            TextCursor undoCursor(*m_cursor);
            score()->undo(new RemoveText(&undoCursor, String(m_cursor->currentCharacter())), &ed);
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

ChangeTextProperties::ChangeTextProperties(const TextCursor* tc, Pid propId, const PropertyValue& propVal, PropertyFlags flags_)
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
    EngravingItem::layout()->layoutText1(cursor().text());
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
} // namespace mu::engraving
