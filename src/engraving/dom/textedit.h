/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_TEXTEDIT_H
#define MU_ENGRAVING_TEXTEDIT_H

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
    size_t startUndoIdx = 0;

    TextCursor* cursor() const;

    bool deleteText = false;

    String selectedText;
    String selectedPlainText;
    static constexpr const char* mimeRichTextFormat = "application/musescore/richtext";

    TextEditData(TextBase* t)
        : m_textBase(t) {}
    TextEditData(const TextEditData&) = delete;
    TextEditData& operator=(const TextEditData&) = delete;
    ~TextEditData();

    virtual EditDataType type() override { return EditDataType::TextEditData; }

    void setDeleteText(bool val) { deleteText = val; }

private:
    TextBase* m_textBase = nullptr;
};

//---------------------------------------------------------
//   TextEditUndoCommand
//---------------------------------------------------------

class TextEditUndoCommand : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, TextEditUndoCommand)

public:
    TextEditUndoCommand(const TextCursor& tc)
        : m_cursor(tc) {}
    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override
    {
        return f == UndoCommand::Filter::TextEdit && m_cursor.text() == target;
    }

    TextCursor& cursor() { return m_cursor; }

protected:
    TextCursor m_cursor;
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeTextProperties : public TextEditUndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeTextProperties)

public:
    ChangeTextProperties(const TextCursor* tc, Pid propId, const PropertyValue& propVal, PropertyFlags flags);
    void undo(EditData*) override;
    void redo(EditData*) override;

private:

    void restoreSelection();

    String m_xmlText;
    Pid m_propertyId;
    PropertyValue m_propertyVal;
    FontStyle m_existingStyle;
    PropertyFlags m_flags;
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeText : public TextEditUndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeText)

public:
    ChangeText(const TextCursor* tc, const String& t)
        : TextEditUndoCommand(*tc), m_s(t), m_format(*tc->format()) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    const String& string() const { return m_s; }

protected:
    void insertText(EditData*);
    void removeText(EditData*);

private:

    String m_s;
    CharFormat m_format;
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
    UNDO_NAME("SplitText")
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
    UNDO_NAME("JoinText")
};
} // namespace mu::engraving

#endif
