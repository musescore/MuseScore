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

#include "editsystemlocks.h"

#include "../dom/measurebase.h"
#include "../dom/score.h"
#include "../dom/systemlock.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddSystemLock
//---------------------------------------------------------

AddSystemLock::AddSystemLock(const SystemLock* systemLock)
    : m_systemLock(systemLock) {}

void AddSystemLock::undo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->removeSystemLock(m_systemLock);
}

void AddSystemLock::redo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->addSystemLock(m_systemLock);
}

void AddSystemLock::cleanup(bool undo)
{
    if (!undo) {
        delete m_systemLock;
        m_systemLock = nullptr;
    }
}

std::vector<EngravingObject*> AddSystemLock::objectItems() const
{
    return { m_systemLock->startMB(), m_systemLock->endMB() };
}

//---------------------------------------------------------
//   RemoveSystemLock
//---------------------------------------------------------

RemoveSystemLock::RemoveSystemLock(const SystemLock* systemLock)
    : m_systemLock(systemLock) {}

void RemoveSystemLock::undo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->addSystemLock(m_systemLock);
}

void RemoveSystemLock::redo(EditData*)
{
    Score* score = m_systemLock->startMB()->score();
    score->removeSystemLock(m_systemLock);
}

void RemoveSystemLock::cleanup(bool undo)
{
    if (undo) {
        delete m_systemLock;
        m_systemLock = nullptr;
    }
}

std::vector<EngravingObject*> RemoveSystemLock::objectItems() const
{
    return { m_systemLock->startMB(), m_systemLock->endMB() };
}
