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

#include "../../dom/connector.h"

//---------------------------------------------------------
//   @@ ConnectorInfoWriter
///    Helper class for writing connecting elements.
///    Subclasses should fill _prevInfo and _nextInfo with
///    the proper information on the connector's endpoints.
//---------------------------------------------------------

namespace mu::engraving {
class XmlWriter;
class Spanner;
class MeasureBase;
}

namespace mu::engraving::write {
class WriteContext;
class ConnectorInfoWriter : public ConnectorInfo
{
    OBJECT_ALLOCATOR(engraving, ConnectorInfoWriter)

public:
    ConnectorInfoWriter(XmlWriter& xml, WriteContext* ctx, const EngravingItem* current, const EngravingItem* connector, int track = -1,
                        Fraction = { -1, 1 });
    virtual ~ConnectorInfoWriter() = default;

    void write();

protected:
    const EngravingItem* _connector;

    virtual const char* tagName() const = 0;

private:
    XmlWriter* m_xml = nullptr;
    WriteContext* m_ctx = nullptr;
};

//-----------------------------------------------------------------------------
//   @@ SpannerWriter
///   Helper class for writing Spanners
//-----------------------------------------------------------------------------

class SpannerWriter : public ConnectorInfoWriter
{
    OBJECT_ALLOCATOR(engraving, SpannerWriter)
protected:
    const char* tagName() const override { return "Spanner"; }
public:
    SpannerWriter(XmlWriter& xml, WriteContext* ctx, const EngravingItem* current, const Spanner* spanner, int track, Fraction frac,
                  bool start);

    static void fillSpannerPosition(Location& l, const MeasureBase* endpoint, const Fraction& tick, bool clipboardmode);
};
}
