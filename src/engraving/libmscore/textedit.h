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
    TextCursor c;
public:
    TextEditUndoCommand(const TextCursor& tc)
        : c(tc) {}
    bool isFiltered(UndoCommand::Filter f, const Element* target) const override
    {
        return f == UndoCommand::Filter::TextEdit && c.text() == target;
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
    const TextCursor& cursor() const { return c; }
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
    UNDO_NAME("InsertText")
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
