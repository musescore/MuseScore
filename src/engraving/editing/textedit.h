/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include "elementeditdata.h"
#include "transaction/undoablecommand.h"

#include "../dom/textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   TextEditData
//---------------------------------------------------------

struct TextEditData : public ElementEditData {
    OBJECT_ALLOCATOR(engraving, TextEditData)
public:

    String oldXmlText;
    size_t startUndoIdx = 0;

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
//   TextEditUndoableCommand
//---------------------------------------------------------

class TextEditUndoableCommand : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, TextEditUndoableCommand)

public:
    UNDO_TYPE(CommandType::TextEdit)
    UNDO_NAME("TextEdit")
    UNDO_CHANGED_OBJECTS({ m_cursor.text() })

    TextEditUndoableCommand(const TextCursor& tc)
        : m_cursor(tc) {}

    bool matchesFilter(UndoableCommandFilter f, const EngravingItem* target) const override
    {
        return f == UndoableCommandFilter::TextEdit && m_cursor.text() == target;
    }

    TextCursor& cursor() { return m_cursor; }
    const TextCursor& cursor() const { return m_cursor; }

protected:
    TextCursor m_cursor;
};

//---------------------------------------------------------
//   ChangeText
//---------------------------------------------------------

class ChangeTextProperties : public TextEditUndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeTextProperties)

public:
    ChangeTextProperties(const TextCursor* tc, Pid propId, const PropertyValue& propVal, PropertyFlags flags);
    void undo() override;
    void redo() override;

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

class ChangeText : public TextEditUndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeText)

public:
    ChangeText(const TextCursor* tc, const String& t)
        : TextEditUndoableCommand(*tc), m_s(t), m_format(*tc->format()) {}
    virtual void undo() override = 0;
    virtual void redo() override = 0;
    const String& string() const { return m_s; }

protected:
    void insertText();
    void removeText();

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
    virtual void redo() override { insertText(); }
    virtual void undo() override { removeText(); }
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
    virtual void redo() override { removeText(); }
    virtual void undo() override { insertText(); }
    UNDO_NAME("RemoveText")
};

//---------------------------------------------------------
//   SplitJoinText
//---------------------------------------------------------

class SplitJoinText : public TextEditUndoableCommand
{
    OBJECT_ALLOCATOR(engraving, SplitJoinText)
protected:
    virtual void split();
    virtual void join();

public:
    SplitJoinText(const TextCursor* tc)
        : TextEditUndoableCommand(*tc) {}
};

//---------------------------------------------------------
//   SplitText
//---------------------------------------------------------

class SplitText : public SplitJoinText
{
    OBJECT_ALLOCATOR(engraving, SplitText)

    virtual void undo() override { join(); }
    virtual void redo() override { split(); }

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

    virtual void undo() override { split(); }
    virtual void redo() override { join(); }

public:
    JoinText(const TextCursor* tc)
        : SplitJoinText(tc) {}
    UNDO_NAME("JoinText")
};
} // namespace mu::engraving
