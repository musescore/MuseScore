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
#ifndef MU_ENGRAVING_CONNECTORINFOREADER_H
#define MU_ENGRAVING_CONNECTORINFOREADER_H

#include "../../libmscore/connector.h"

//---------------------------------------------------------
//   @@ ConnectorInfoReader
///    Helper class for reading beams, tuplets and spanners.
//---------------------------------------------------------

namespace mu::engraving {
class XmlReader;
class ChordRest;
class Measure;
class Note;
}

namespace mu::engraving::rw400 {
class ConnectorInfoReader final : public ConnectorInfo
{
    OBJECT_ALLOCATOR(engraving, ConnectorInfoReader)

    XmlReader* _reader;
    EngravingItem* _connector;
    EngravingObject* _connectorReceiver;

    void readEndpointLocation(Location& l);

public:
    ConnectorInfoReader(XmlReader& e, EngravingItem* current, int track = -1);
    ConnectorInfoReader(XmlReader& e, Score* current, int track = -1);

    ConnectorInfoReader* prev() const { return static_cast<ConnectorInfoReader*>(_prev); }
    ConnectorInfoReader* next() const { return static_cast<ConnectorInfoReader*>(_next); }

    EngravingItem* connector();
    const EngravingItem* connector() const;
    EngravingItem* releaseConnector();   // returns connector and "forgets" it by
    // setting an internal pointer to it to zero

    bool read();
    void update();
    void addToScore(bool pasteMode);

    static void readConnector(std::unique_ptr<ConnectorInfoReader> info, XmlReader& e);

    static void readAddConnector(ChordRest* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Measure* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Note* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Score* item, ConnectorInfoReader* info, bool pasteMode);
};
}

#endif // MU_ENGRAVING_CONNECTORINFOREADER_H
