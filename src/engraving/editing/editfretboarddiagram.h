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

#include "../dom/fret.h"

namespace mu::engraving {
class FretUndoData
{
public:
    FretUndoData() = default;
    FretUndoData(FretDiagram* fd);

    void updateDiagram();

private:
    FretDiagram* m_diagram = nullptr;
    BarreMap m_barres;
    MarkerMap m_markers;
    DotMap m_dots;

    int m_strings = 0;
    int m_frets = 0;
    int m_fretOffset = 0;
    int m_maxFrets = 0;
    bool m_showNut = true;
    bool m_showFingering = false;
    Orientation m_orientation = Orientation::VERTICAL;
    double m_userMag = 1.0;
};

class FretDataChange : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretDataChange)

    FretDiagram* m_diagram = nullptr;
    FretUndoData m_undoData;
    String m_harmonyName;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretDataChange(FretDiagram* d, const String& harmonyName)
        : m_diagram(d), m_harmonyName(harmonyName) {}

    UNDO_TYPE(CommandType::FretDataChange)

    UNDO_NAME("FretDataChange")

    UNDO_CHANGED_OBJECTS({ m_diagram })
};

class FretDot : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretDot)

    FretDiagram* diagram = nullptr;
    int string = 0;
    int fret = 0;
    bool add = 0;
    FretDotType dtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretDot(FretDiagram* d, int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL)
        : diagram(d), string(_string), fret(_fret), add(_add), dtype(_dtype) {}

    UNDO_TYPE(CommandType::FretDot)
    UNDO_NAME("FretDot")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretMarker : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretMarker)

    FretDiagram* diagram = nullptr;
    int string = 0;
    FretMarkerType mtype;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretMarker(FretDiagram* d, int _string, FretMarkerType _mtype)
        : diagram(d), string(_string), mtype(_mtype) {}

    UNDO_TYPE(CommandType::FretMarker)
    UNDO_NAME("FretMarker")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretBarre : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretBarre)

    FretDiagram* diagram = nullptr;
    int string = 0;
    int fret = 0;
    bool add = 0;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretBarre(FretDiagram* d, int _string, int _fret, bool _add = false)
        : diagram(d), string(_string), fret(_fret), add(_add) {}

    UNDO_TYPE(CommandType::FretBarre)
    UNDO_NAME("FretBarre")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class FretClear : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, FretClear)

    FretDiagram* diagram = nullptr;
    FretUndoData undoData;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    FretClear(FretDiagram* d)
        : diagram(d) {}

    UNDO_TYPE(CommandType::FretClear)
    UNDO_NAME("FretClear")
    UNDO_CHANGED_OBJECTS({ diagram })
};

class AddFretDiagramToFretBox : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddFretDiagramToFretBox)

    FretDiagram* m_fretDiagram = nullptr;
    size_t m_idx = muse::nidx;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    AddFretDiagramToFretBox(FretDiagram* f, size_t idx);

    UNDO_TYPE(CommandType::AddFretDiagramToFretBox)
    UNDO_NAME("AddFretDiagramToFretBox")
    UNDO_CHANGED_OBJECTS({ m_fretDiagram })
};

class RemoveFretDiagramFromFretBox : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveFretDiagramFromFretBox)

    FretDiagram* m_fretDiagram = nullptr;
    size_t m_idx = muse::nidx;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    RemoveFretDiagramFromFretBox(FretDiagram* f);

    UNDO_TYPE(CommandType::RemoveFretDiagramFromFretBox)
    UNDO_NAME("RemoveFretDiagramFromFretBox")
    UNDO_CHANGED_OBJECTS({ m_fretDiagram })
};
}
