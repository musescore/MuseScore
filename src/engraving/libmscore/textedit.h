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

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "element.h"
#include "text.h"
#include "undo.h"

namespace Ms {
//---------------------------------------------------------
//   TextEditData
//---------------------------------------------------------

struct TextEditData : public ElementEditData {
    QString oldXmlText;
    int startUndoIdx { 0 };

    TextCursor* cursor() const;
    TextBase* _textBase = nullptr;
    bool deleteText = false;

    QString selectedText;

    TextEditData(TextBase* t)
        : _textBase(t) {}
    TextEditData(const TextEditData&) = delete;
    TextEditData& operator=(const TextEditData&) = delete;
    ~TextEditData();

    virtual EditDataType type() override { return EditDataType::TextEditData; }

    void setDeleteText(bool val) { deleteText = val; }
};

//---------------------------------------------------------
//   TextEditUndoCommand
//---------------------------------------------------------

class TextEditUndoCommand : public UndoCommand
{
protected:
    TextCursor _cursor;
public:
    TextEditUndoCommand(const TextCursor& tc)
        : _cursor(tc) {}
    bool isFiltered(UndoCommand::Filter f, const Element* target) const override
    {
        return f == UndoCommand::Filter::TextEdit && _cursor.text() == target;
    }

    TextCursor& cursor() { return _cursor; }
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeTextProperties : public TextEditUndoCommand
{
    QString xmlText;
    Pid propertyId;
    QVariant propertyVal;
    FontStyle existingStyle;

    void restoreSelection()
    {
        TextCursor& tc = cursor();
        tc.text()->cursor()->setSelectLine(tc.selectLine());
        tc.text()->cursor()->setSelectColumn(tc.selectColumn());
        tc.text()->cursor()->setRow(tc.row());
        tc.text()->cursor()->setColumn(tc.column());
    }

public:
    ChangeTextProperties(const TextCursor* tc, Ms::Pid propId, const QVariant& propVal)
        : TextEditUndoCommand(*tc)
    {
        propertyId = propId;
        propertyVal = propVal;
        if (propertyId == Pid::FONT_STYLE) {
            existingStyle = static_cast<FontStyle>(cursor().text()->getProperty(propId).toInt());
        }
    }

    virtual void undo(EditData*) override
    {
        cursor().text()->resetFormatting();
        cursor().text()->setXmlText(xmlText);
        restoreSelection();
        cursor().text()->layout();
    }

    virtual void redo(EditData*) override
    {
        xmlText = cursor().text()->xmlText();
        restoreSelection();
        if (propertyId == Pid::FONT_STYLE) {
            FontStyle setStyle = static_cast<FontStyle>(propertyVal.toInt());
            if ((setStyle& FontStyle::Bold) != (existingStyle & FontStyle::Bold)) {
                cursor().setFormat(FormatId::Bold, setStyle & FontStyle::Bold);
            }
            if ((setStyle& FontStyle::Italic) != (existingStyle & FontStyle::Italic)) {
                cursor().setFormat(FormatId::Italic, setStyle & FontStyle::Italic);
            }
            if ((setStyle& FontStyle::Underline) != (existingStyle & FontStyle::Underline)) {
                cursor().setFormat(FormatId::Underline, setStyle & FontStyle::Underline);
            }
        } else {
            cursor().text()->setProperty(propertyId, propertyVal);
        }
    }
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeText : public TextEditUndoCommand
{
    QString s;

protected:
    void insertText(EditData*);
    void removeText(EditData*);

public:
    ChangeText(const TextCursor* tc, const QString& t)
        : TextEditUndoCommand(*tc), s(t) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    const QString& string() const { return s; }
};

//---------------------------------------------------------
//   InsertText
//---------------------------------------------------------

class InsertText : public ChangeText
{
public:
    InsertText(const TextCursor* tc, const QString& t)
        : ChangeText(tc, t) {}
    virtual void redo(EditData* ed) override { insertText(ed); }
    virtual void undo(EditData* ed) override { removeText(ed); }
    UNDO_NAME("InsertText")
};

//---------------------------------------------------------
//   RemoveText
//---------------------------------------------------------

class RemoveText : public ChangeText
{
public:
    RemoveText(const TextCursor* tc, const QString& t)
        : ChangeText(tc, t) {}
    virtual void redo(EditData* ed) override { removeText(ed); }
    virtual void undo(EditData* ed) override { insertText(ed); }
    UNDO_NAME("RemoveText")
};

//---------------------------------------------------------
//   SplitJoinText
//---------------------------------------------------------

class SplitJoinText : public TextEditUndoCommand
{
protected:
    virtual void split(EditData*);
    virtual void join(EditData*);

public:
    SplitJoinText(const TextCursor* tc)
        : TextEditUndoCommand(*tc) {}
};

//---------------------------------------------------------
//   SplitText
//---------------------------------------------------------

class SplitText : public SplitJoinText
{
    virtual void undo(EditData* data) override { join(data); }
    virtual void redo(EditData* data) override { split(data); }

public:
    SplitText(const TextCursor* tc)
        : SplitJoinText(tc) {}
    UNDO_NAME("SplitText");
};

//---------------------------------------------------------
//   JoinText
//---------------------------------------------------------

class JoinText : public SplitJoinText
{
    virtual void undo(EditData* data) override { split(data); }
    virtual void redo(EditData* data) override { join(data); }

public:
    JoinText(const TextCursor* tc)
        : SplitJoinText(tc) {}
    UNDO_NAME("JoinText");
};
}     // namespace Ms

#endif
