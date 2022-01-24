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

#include "text.h"

namespace Ms {
//---------------------------------------------------------
//   @@ Marker
//
//   @P label       string
//   @P markerType  enum (Marker.CODA, .CODETTA, .FINE, .SEGNO, .TOCODA, .USER, .VARCODA, .VARSEGNO)
//---------------------------------------------------------

class Marker final : public TextBase
{
public:
    enum class Type : char {
        SEGNO,
        VARSEGNO,
        CODA,
        VARCODA,
        CODETTA, // not in SMuFL, but still needed for 1.x compatibility, rendered as a double coda
        FINE,
        TOCODA,
        TOCODASYM,
        USER
    };

private:
    Type _markerType;
    QString _label;                 ///< referenced from Jump() element

public:
    Marker(EngravingItem* parent);
    Marker(EngravingItem* parent, TextStyleType);

    void setMarkerType(Type t);
    Type markerType() const { return _markerType; }
    QString markerTypeUserName() const;
    Type markerType(const QString&) const;

    Marker* clone() const override { return new Marker(*this); }

    int subtype() const override { return int(_markerType); }

    Measure* measure() const { return (Measure*)explicitParent(); }

    void layout() override;
    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    QString label() const { return _label; }
    void setLabel(const QString& s) { _label = s; }
    void undoSetLabel(const QString& s);
    void undoSetMarkerType(Type t);

    void styleChanged() override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    QString accessibleInfo() const override;
};

//---------------------------------------------------------
//   MarkerTypeItem
//---------------------------------------------------------

struct MarkerTypeItem {
    Marker::Type type;
    QString name;
};

extern const std::vector<MarkerTypeItem> markerTypeTable;
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Marker::Type);

#endif
