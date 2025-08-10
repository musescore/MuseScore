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
#pragma once

#include "../xmlreader.h"
#include "../../dom/connector.h"

//---------------------------------------------------------
//   @@ ConnectorInfoReader
///    Helper class for reading beams, tuplets and spanners.
//---------------------------------------------------------

namespace mu::engraving {
class ChordRest;
class Measure;
class Note;
}

namespace mu::engraving::read460 {
class ReadContext;
class ConnectorInfoReader final : public ConnectorInfo
{
    OBJECT_ALLOCATOR(engraving, ConnectorInfoReader)

public:
    ConnectorInfoReader(XmlReader& e, ReadContext* ctx, EngravingItem* current, int track = -1);
    ConnectorInfoReader(XmlReader& e, ReadContext* ctx, Score* current, int track = -1);

    ConnectorInfoReader* prev() const { return static_cast<ConnectorInfoReader*>(m_prev); }
    ConnectorInfoReader* next() const { return static_cast<ConnectorInfoReader*>(m_next); }

    EngravingItem* connector();
    const EngravingItem* connector() const;
    EngravingItem* releaseConnector();   // returns connector and "forgets" it by
    // setting an internal pointer to it to zero

    bool read();
    void update();
    void addToScore(bool pasteMode);

    static void readConnector(std::shared_ptr<ConnectorInfoReader> info, XmlReader& e, ReadContext& ctx);

    static void readAddConnector(ChordRest* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Measure* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Note* item, ConnectorInfoReader* info, bool pasteMode);
    static void readAddConnector(Score* item, ConnectorInfoReader* info, bool pasteMode);

private:
    void readEndpointLocation(Location& l);
    Fraction curTick() const;

    XmlReader* m_reader = nullptr;
    ReadContext* m_ctx = nullptr;
    EngravingItem* m_connector = nullptr;
    EngravingObject* m_connectorReceiver = nullptr;
};
}
