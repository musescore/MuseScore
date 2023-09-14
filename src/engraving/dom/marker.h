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

#ifndef __MARKER_H__
#define __MARKER_H__

#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ Marker
//
//   @P label       string
//   @P markerType  enum (Marker.CODA, .CODETTA, .FINE, .SEGNO, .TOCODA, .USER, .VARCODA, .VARSEGNO)
//---------------------------------------------------------

class Marker final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Marker)
    DECLARE_CLASSOF(ElementType::MARKER)

public:
    Marker(EngravingItem* parent);
    Marker(EngravingItem* parent, TextStyleType);

    void setMarkerType(MarkerType t);
    MarkerType markerType() const { return _markerType; }
    String markerTypeUserName() const;

    Marker* clone() const override { return new Marker(*this); }

    int subtype() const override { return int(_markerType); }

    Measure* measure() const { return (Measure*)explicitParent(); }

    String label() const { return _label; }
    void setLabel(const String& s) { _label = s; }
    void undoSetLabel(const String& s);
    void undoSetMarkerType(MarkerType t);

    void styleChanged() override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    void setLayoutToParentWidth(bool v) { m_layoutToParentWidth = v; }

private:
    MarkerType _markerType;
    String _label;                 ///< referenced from Jump() element
};
} // namespace mu::engraving

#endif
