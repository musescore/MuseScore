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

#ifndef MU_ENGRAVING_CONNECTOR_H
#define MU_ENGRAVING_CONNECTOR_H

#include "global/allocator.h"

#include "location.h"
#include "../types/types.h"

namespace mu::engraving {
class EngravingItem;
class EngravingObject;
class Score;

//---------------------------------------------------------
//   @@ ConnectorInfo
///    Stores a general information on various connecting
///    elements (currently only spanners) including their
///    endpoints locations.
///    Base class of helper classes used to read and write
///    such elements.
//---------------------------------------------------------

class ConnectorInfo
{
    OBJECT_ALLOCATOR(engraving, ConnectorInfo)

public:
    ConnectorInfo(const EngravingItem* current, int track = -1, Fraction = { -1, 1 });
    ConnectorInfo(const Score* score, const Location& currentLocation);
    virtual ~ConnectorInfo() = default;

    ConnectorInfo* prev() const { return m_prev; }
    ConnectorInfo* next() const { return m_next; }
    ConnectorInfo* start();
    ConnectorInfo* end();

    ElementType type() const { return m_type; }
    const Location& location() const { return m_currentLoc; }

    bool connect(ConnectorInfo* other);
    bool finished() const;

    // for reconnection of broken connectors
    int connectionDistance(const ConnectorInfo& c2) const;
    void forceConnect(ConnectorInfo* c2);

    bool hasPrevious() const { return m_prevLoc.measure() != INT_MIN; }
    bool hasNext() const { return m_nextLoc.measure() != INT_MIN; }
    bool isStart() const { return !hasPrevious() && hasNext(); }
    bool isMiddle() const { return hasPrevious() && hasNext(); }
    bool isEnd() const { return hasPrevious() && !hasNext(); }

protected:

    void updateLocation(const EngravingItem* e, Location& i, bool clipboardmode);
    void updateCurrentInfo(bool clipboardmode);
    bool currentUpdated() const { return m_currentUpdated; }
    void setCurrentUpdated(bool v) { m_currentUpdated = v; }

    ConnectorInfo* findFirst();
    const ConnectorInfo* findFirst() const;
    ConnectorInfo* findLast();
    const ConnectorInfo* findLast() const;

    ElementType m_type = ElementType::INVALID;
    Location m_currentLoc;
    Location m_prevLoc = Location::absolute();
    Location m_nextLoc = Location::absolute();

    ConnectorInfo* m_prev = nullptr;
    ConnectorInfo* m_next = nullptr;

private:
    bool finishedLeft() const;
    bool finishedRight() const;

    static int orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2);

    const EngravingItem* m_current = nullptr;
    bool m_currentUpdated = false;
    const Score* m_score = nullptr;
};
} // namespace mu::engraving
#endif
