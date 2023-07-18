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

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "global/allocator.h"

#include "location.h"
#include "types/types.h"

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

    const EngravingItem* _current    { 0 };
    bool _currentUpdated       { false };
    const Score* _score;

    bool finishedLeft() const;
    bool finishedRight() const;

    static int orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2);

protected:
    ElementType _type       { ElementType::INVALID };
    Location _currentLoc;
    Location _prevLoc       { Location::absolute() };
    Location _nextLoc       { Location::absolute() };

    ConnectorInfo* _prev    { 0 };
    ConnectorInfo* _next    { 0 };

    void updateLocation(const EngravingItem* e, Location& i, bool clipboardmode);
    void updateCurrentInfo(bool clipboardmode);
    bool currentUpdated() const { return _currentUpdated; }
    void setCurrentUpdated(bool v) { _currentUpdated = v; }

    ConnectorInfo* findFirst();
    const ConnectorInfo* findFirst() const;
    ConnectorInfo* findLast();
    const ConnectorInfo* findLast() const;

public:
    ConnectorInfo(const EngravingItem* current, int track = -1, Fraction = { -1, 1 });
    ConnectorInfo(const Score* score, const Location& currentLocation);
    virtual ~ConnectorInfo() = default;

    ConnectorInfo* prev() const { return _prev; }
    ConnectorInfo* next() const { return _next; }
    ConnectorInfo* start();
    ConnectorInfo* end();

    ElementType type() const { return _type; }
    const Location& location() const { return _currentLoc; }

    bool connect(ConnectorInfo* other);
    bool finished() const;

    // for reconnection of broken connectors
    int connectionDistance(const ConnectorInfo& c2) const;
    void forceConnect(ConnectorInfo* c2);

    bool hasPrevious() const { return _prevLoc.measure() != INT_MIN; }
    bool hasNext() const { return _nextLoc.measure() != INT_MIN; }
    bool isStart() const { return !hasPrevious() && hasNext(); }
    bool isMiddle() const { return hasPrevious() && hasNext(); }
    bool isEnd() const { return hasPrevious() && !hasNext(); }
};
} // namespace mu::engraving
#endif
