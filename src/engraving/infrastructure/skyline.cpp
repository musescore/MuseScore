/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "skyline.h"

#include "realfn.h"
#include "draw/painter.h"

#include "../dom/arpeggio.h"
#include "../dom/beam.h"
#include "../dom/chord.h"
#include "../dom/stem.h"
#include "../dom/tremolotwochord.h"

#include "shape.h"

using namespace mu;
using namespace muse::draw;

namespace mu::engraving {
static const double MAXIMUM_Y = 1000000.0;
static const double MINIMUM_Y = -1000000.0;

// #define SKL_DEBUG

#ifdef SKL_DEBUG
#define DP(...)   printf(__VA_ARGS__)
#else
#define DP(...)
#endif

void Skyline::add(const ShapeElement& r)
{
    if (r.ignoreForLayout()) {
        return;
    }
    const EngravingItem* item = r.item();
    bool crossSouth = false;
    bool crossNorth = false;
    if (item && item->isStem()) {
        Chord* chord = toStem(item)->chord();
        if (chord) {
            Beam* beam = chord->beam();
            TremoloTwoChord* tremolo = chord->tremoloTwoChord();
            bool isCross = (beam && beam->cross())
                           || (tremolo && tremolo->chord1()->staffMove() != tremolo->chord2()->staffMove());
            if (isCross) {
                std::vector<ChordRest*> elements;
                if (beam) {
                    elements = beam->elements();
                } else if (tremolo) {
                    elements = { tremolo->chord1(), tremolo->chord2() };
                }
                int thisStaffMove = chord->staffMove();
                if (thisStaffMove < 0) {
                    crossNorth = true;
                } else if (thisStaffMove > 0) {
                    crossSouth = true;
                }
                for (ChordRest* element : elements) {
                    int staffMove = element->staffMove();
                    if (staffMove < thisStaffMove) {
                        crossNorth = true;
                    }
                    if (staffMove > thisStaffMove) {
                        crossSouth = true;
                    }
                }
            }
        }
    }
    if (item && item->isArpeggio()) {
        const Arpeggio* arpeggio = toArpeggio(item);
        if (arpeggio->crossStaff()) {
            crossSouth = true;
        }
    }
    if (!crossNorth) {
        _north.add(r);
    }
    if (!crossSouth) {
        _south.add(r);
    }
}

void SkylineLine::add(const Shape& s)
{
    for (const ShapeElement& se : s.elements()) {
        add(se);
    }
}

void SkylineLine::add(const ShapeElement& r)
{
    if (r.item() && r.item()->isStaffLines()) {
        m_staffLinesTop = std::min(m_staffLinesTop, r.top());
        m_staffLinesBottom = std::max(m_staffLinesBottom, r.bottom());
    } else if (hasValidStaffLineEdges()) {
        if (m_isNorth && muse::RealIsEqualOrMore(r.top(), m_staffLinesTop)) {
            return; // Only add if it pokes above the staff
        }
        if (!m_isNorth && muse::RealIsEqualOrLess(r.bottom(), m_staffLinesBottom)) {
            return; // Only add if it pokes below the staff
        }
    }
    m_shape.add(r);
}

void Skyline::add(const Shape& s)
{
    for (const auto& r : s.elements()) {
        add(r);
    }
}

void Skyline::clear()
{
    _north.clear();
    _south.clear();
}

void SkylineLine::clear()
{
    m_staffLinesTop = 0.0;
    m_staffLinesBottom = 0.0;
    m_shape.clear();
}

//-------------------------------------------------------------------
//   minDistance
//    a is located below this skyline.
//    Calculates the minimum distance between two skylines
//-------------------------------------------------------------------

double Skyline::minDistance(const Skyline& s) const
{
    return south().minDistance(s.north());
}

double SkylineLine::minDistance(const SkylineLine& sl) const
{
    return m_shape.minVerticalDistance(sl.m_shape);
}

void Skyline::paint(Painter& painter, double lineWidth) const
{
    painter.save();

    painter.setBrush(BrushStyle::NoBrush);
    painter.setPen(Pen(Color(144, 238, 144, 150), lineWidth));
    _north.paint(painter);
    painter.setPen(Pen(Color(144, 144, 238, 150), lineWidth));
    _south.paint(painter);

    painter.restore();
}

void SkylineLine::paint(Painter& painter) const
{
    m_shape.paint(painter);
}

bool SkylineLine::valid() const
{
    return !m_shape.empty();
}

double SkylineLine::max() const
{
    return m_isNorth ? m_shape.top() : m_shape.bottom();
}
} // namespace mu::engraving

std::string mu::engraving::dump(const Skyline& skyline)
{
    std::stringstream ss;
    ss << "Skyline dump.\n";
    dump(skyline.north(), ss);
    dump(skyline.south(), ss);
    return ss.str();
}

void mu::engraving::dump(const SkylineLine& skylineLine, std::stringstream& ss)
{
    ss << "North: " << skylineLine.isNorth();
    for (const ShapeElement& se : skylineLine.elements()) {
        dump(se, ss);
    }
}
