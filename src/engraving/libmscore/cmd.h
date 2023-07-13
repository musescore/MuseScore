/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "types/types.h"
#include "engravingobject.h"

namespace mu::engraving {

//---------------------------------------------------------
//   UpdateState
//---------------------------------------------------------

class UpdateState
{
public:
    mu::RectF refresh;                 ///< area to update, canvas coordinates
    bool _playNote   { false };     ///< play selected note after command
    bool _playChord  { false };     ///< play whole chord for the selected note
    bool _selectionChanged { false };
    std::list<EngravingObject*> _deleteList;
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
    UpdateMode _updateMode { UpdateMode::DoNothing };
    Fraction _startTick { -1, 1 };            // start tick for mode LayoutTick
    Fraction _endTick   { -1, 1 };              // end tick for mode LayoutTick
    staff_idx_t _startStaff = mu::nidx;
    staff_idx_t _endStaff = mu::nidx;
    const EngravingItem* _el = nullptr;
    const MeasureBase* _mb = nullptr;
    bool _oneElement = true;
    bool _oneMeasureBase = true;

    bool _locked = false;

    void setMeasureBase(const MeasureBase* mb);

public:
    LayoutFlags layoutFlags;

    bool _excerptsChanged     { false };
    bool _instrumentsChanged  { false };

    void reset();
    UpdateMode updateMode() const { return _updateMode; }
    void setUpdateMode(UpdateMode m);
    void _setUpdateMode(UpdateMode m);
    bool layoutRange() const { return _updateMode == UpdateMode::Layout; }
    bool updateAll() const { return int(_updateMode) >= int(UpdateMode::UpdateAll); }
    bool updateRange() const { return _updateMode == UpdateMode::Update; }
    void setTick(const Fraction& t);
    void setStaff(staff_idx_t staff);
    void setElement(const EngravingItem* e);
    void unsetElement(const EngravingItem* e);
    Fraction startTick() const { return _startTick; }
    Fraction endTick() const { return _endTick; }
    staff_idx_t startStaff() const { return _startStaff; }
    staff_idx_t endStaff() const { return _endStaff; }
    const EngravingItem* element() const;

    void lock() { _locked = true; }
    void unlock() { _locked = false; }
#ifndef NDEBUG
    void dump();
#endif
};

}

#endif // MU_ENGRAVING_CMD_H
