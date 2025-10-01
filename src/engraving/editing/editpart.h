/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "undo.h"

#include "../compat/midi/midipatch.h"
#include "../dom/drumset.h"
#include "../dom/instrument.h"
#include "../dom/part.h"
#include "../dom/score.h"

namespace mu::engraving {
class InsertPart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertPart)

    Part* m_part = nullptr;
    size_t m_targetPartIdx = 0;

public:
    InsertPart(Part* p, size_t targetPartIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::InsertPart)
    UNDO_NAME("InsertPart")
    UNDO_CHANGED_OBJECTS({ m_part })
};

class RemovePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemovePart)

    Part* m_part = nullptr;
    size_t m_partIdx = muse::nidx;

public:
    RemovePart(Part*, size_t partIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::RemovePart)
    UNDO_NAME("RemovePart")
    UNDO_CHANGED_OBJECTS({ m_part })
};

class SetSoloist : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetSoloist)

    Part* part = nullptr;
    bool soloist = false;

public:
    SetSoloist(Part* p, bool b);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::SetSoloist)
    UNDO_NAME("SetSoloist")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangePart : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePart)

    Part* part = nullptr;
    Instrument* instrument = nullptr;
    String partName;

    void flip(EditData*) override;

public:
    ChangePart(Part*, Instrument*, const String& name);

    UNDO_TYPE(CommandType::ChangePart)
    UNDO_NAME("ChangePart")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangeInstrumentLong : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentLong)

    Part* part = nullptr;
    Fraction tick;
    StaffNameList text;

    void flip(EditData*) override;

public:
    ChangeInstrumentLong(const Fraction&, Part*, const StaffNameList&);

    UNDO_TYPE(CommandType::ChangeInstrumentLong)
    UNDO_NAME("ChangeInstrumentLong")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangeInstrumentShort : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeInstrumentShort)

    Part* part = nullptr;
    Fraction tick;
    StaffNameList text;

    void flip(EditData*) override;

public:
    ChangeInstrumentShort(const Fraction&, Part*, const StaffNameList&);

    UNDO_TYPE(CommandType::ChangeInstrumentShort)
    UNDO_NAME("ChangeInstrumentShort")
    UNDO_CHANGED_OBJECTS({ part })
};

class ChangeDrumset : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeDrumset)

    Instrument* instrument = nullptr;
    Drumset drumset;
    Part* part = nullptr;

    void flip(EditData*) override;

public:
    ChangeDrumset(Instrument* i, const Drumset& d, Part* p)
        : instrument(i), drumset(d), part(p) {}

    UNDO_TYPE(CommandType::ChangeDrumset)
    UNDO_NAME("ChangeDrumset")
};

class ChangeStringData : public UndoCommand
{
    Instrument* m_instrument = nullptr;
    StringTunings* m_stringTunings = nullptr;
    StringData m_stringData;

public:
    ChangeStringData(StringTunings* stringTunings, const StringData& stringData)
        : m_stringTunings(stringTunings), m_stringData(stringData) {}
    ChangeStringData(Instrument* instrument, const StringData& stringData)
        : m_instrument(instrument), m_stringData(stringData) {}

    void flip(EditData*) override;
    UNDO_NAME("ChangeStringData")
};

class ChangePatch : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePatch)

    Score* score = nullptr;
    InstrChannel* channel = nullptr;
    MidiPatch patch;

    void flip(EditData*) override;

public:
    ChangePatch(Score* s, InstrChannel* c, const MidiPatch& pt)
        : score(s), channel(c), patch(pt) {}
    UNDO_NAME("ChangePatch")
    UNDO_CHANGED_OBJECTS({ score })
};

class SetUserBankController : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SetUserBankController)

    InstrChannel* channel = nullptr;
    bool val = false;

    void flip(EditData*) override;

public:
    SetUserBankController(InstrChannel* c, bool v)
        : channel(c), val(v) {}
    UNDO_NAME("SetUserBankController")
};
}
