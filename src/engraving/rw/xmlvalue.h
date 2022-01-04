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
#ifndef MU_ENGRAVING_XMLVALUE_H
#define MU_ENGRAVING_XMLVALUE_H

#include <QString>

#include "types/types.h"

//! NOTE This is class for conversion from/to xml values
namespace mu::engraving::rw {
class XmlValue
{
public:

    static QString toXml(OrnamentStyle v);
    static OrnamentStyle fromXml(const QString& str, OrnamentStyle def);

    static QString toXml(PlacementV v);
    static PlacementV fromXml(const QString& str, PlacementV def);
    static QString toXml(PlacementH v);
    static PlacementH fromXml(const QString& str, PlacementH def);

    static QString toXml(TextPlace v);
    static TextPlace fromXml(const QString& str, TextPlace def);

    static QString toXml(DirectionV v);
    static DirectionV fromXml(const QString& str, DirectionV def);
    static QString toXml(DirectionH v);
    static DirectionH fromXml(const QString& str, DirectionH def);

    static QString toXml(LayoutBreakType v);
    static LayoutBreakType fromXml(const QString& str, LayoutBreakType def);

    static QString toXml(VeloType v);
    static VeloType fromXml(const QString& str, VeloType def);

    static QString toXml(BeamMode v);
    static BeamMode fromXml(const QString& str, BeamMode def);

    static QString toXml(GlissandoStyle v);
    static GlissandoStyle fromXml(const QString& str, GlissandoStyle def);

    static QString toXml(BarLineType v);
    static BarLineType fromXml(const QString& str, BarLineType def);
};
}

#endif // MU_ENGRAVING_XMLVALUE_H
