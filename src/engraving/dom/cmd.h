/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_CMD_H
#define MU_ENGRAVING_CMD_H

#include <list>

#include "../types/types.h"
#include "engravingobject.h"

namespace mu::engraving {
//---------------------------------------------------------
//   UpdateState
//---------------------------------------------------------

struct UpdateState
{
    RectF refresh;           ///< area to update, canvas coordinates
    bool playNote = false;       ///< play selected note after command
    bool playChord = false;      ///< play whole chord for the selected note
    bool selectionChanged = false;
    std::list<EngravingObject*> deleteList;
};

//---------------------------------------------------------
//   CmdState
//
//    the following variables are reset on startCmd()
//    modified during cmd processing and used in endCmd() to
//    determine what to layout and what to repaint:
//---------------------------------------------------------

class CmdState
{
public:
    LayoutFlags layoutFlags;

    bool excerptsChanged = false;
    bool instrumentsChanged = false;

    void reset();
    UpdateMode updateMode() const { return m_updateMode; }
    void setUpdateMode(UpdateMode m);
    void _setUpdateMode(UpdateMode m);
    bool layoutRange() const { return m_updateMode == UpdateMode::Layout; }
    bool updateAll() const { return int(m_updateMode) >= int(UpdateMode::UpdateAll); }
    bool updateRange() const { return m_updateMode == UpdateMode::Update; }
    void setTick(const Fraction& t);
    void setStaff(staff_idx_t staff);
    void setElement(const EngravingItem* e);
    void unsetElement(const EngravingItem* e);
    Fraction startTick() const { return m_startTick; }
    Fraction endTick() const { return m_endTick; }
    staff_idx_t startStaff() const { return m_startStaff; }
    staff_idx_t endStaff() const { return m_endStaff; }
    const EngravingItem* element() const;

    void lock() { m_locked = true; }
    void unlock() { m_locked = false; }
#ifndef NDEBUG
    void dump();
#endif

private:
    void setMeasureBase(const MeasureBase* mb);

    UpdateMode m_updateMode = UpdateMode::DoNothing;
    Fraction m_startTick { -1, 1 };            // start tick for mode LayoutTick
    Fraction m_endTick   { -1, 1 };              // end tick for mode LayoutTick
    staff_idx_t m_startStaff = muse::nidx;
    staff_idx_t m_endStaff = muse::nidx;
    const EngravingItem* m_el = nullptr;
    const MeasureBase* m_mb = nullptr;
    bool m_oneElement = true;
    bool m_oneMeasureBase = true;

    bool m_locked = false;
};
}

#endif // MU_ENGRAVING_CMD_H
