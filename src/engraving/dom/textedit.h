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

#include "engravingitem.h"
#include "undo.h"

namespace mu::engraving {
class TextBase;

//---------------------------------------------------------
//   TextEditData
//---------------------------------------------------------

struct TextEditData : public ElementEditData {
    OBJECT_ALLOCATOR(engraving, TextEditData)
public:

    String oldXmlText;
    size_t startUndoIdx { 0 };

    TextCursor* cursor() const;
    TextBase* _textBase = nullptr;
    bool deleteText = false;

    String selectedText;
    String selectedPlainText;
    static constexpr const char* mimeRichTextFormat = "application/musescore/richtext";

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
    OBJECT_ALLOCATOR(engraving, TextEditUndoCommand)
protected:
    TextCursor _cursor;
public:
    TextEditUndoCommand(const TextCursor& tc)
        : _cursor(tc) {}
    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override
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
    OBJECT_ALLOCATOR(engraving, ChangeTextProperties)

    String xmlText;
    Pid propertyId;
    PropertyValue propertyVal;
    FontStyle existingStyle;
    PropertyFlags flags;

    void restoreSelection();

public:
    ChangeTextProperties(const TextCursor* tc, Pid propId, const PropertyValue& propVal, PropertyFlags flags);
    void undo(EditData*) override;
    void redo(EditData*) override;
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeText : public TextEditUndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeText)

    String s;
    CharFormat format;

protected:
    void insertText(EditData*);
    void removeText(EditData*);

public:
    ChangeText(const TextCursor* tc, const String& t)
        : TextEditUndoCommand(*tc), s(t), format(*tc->format()) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    const String& string() const { return s; }
};

//---------------------------------------------------------
//   InsertText
//---------------------------------------------------------

class InsertText : public ChangeText
{
    OBJECT_ALLOCATOR(engraving, InsertText)
public:
    InsertText(const TextCursor* tc, const String& t)
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
    OBJECT_ALLOCATOR(engraving, RemoveText)
public:
    RemoveText(const TextCursor* tc, const String& t)
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
    OBJECT_ALLOCATOR(engraving, SplitJoinText)
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
    OBJECT_ALLOCATOR(engraving, SplitText)

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
    OBJECT_ALLOCATOR(engraving, JoinText)

    virtual void undo(EditData* data) override { split(data); }
    virtual void redo(EditData* data) override { join(data); }

public:
    JoinText(const TextCursor* tc)
        : SplitJoinText(tc) {}
    UNDO_NAME("JoinText");
};
} // namespace mu::engraving

#endif
