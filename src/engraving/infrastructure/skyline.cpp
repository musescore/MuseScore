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

SkylineLine SkylineLine::getFilteredCopy(std::function<bool(const ShapeElement&)> filterOut) const
{
    SkylineLine newSkylineLine(*this);

    newSkylineLine.m_shape.clear();

    for (const ShapeElement& shapeEl : m_shape.elements()) {
        if (filterOut(shapeEl)) {
            continue;
        }
        newSkylineLine.m_shape.add(shapeEl);
    }

    return newSkylineLine;
}

void SkylineLine::add(const ShapeElement& r)
{
    if (r.ignoreForLayout()) {
        return;
    }

    double x = r.x();
    double top = r.top();
    double bottom = r.bottom();
    double staffLinesTop = staffLinesTopAtX(x);
    double staffLinesBottom = staffLinesBottomAtX(x);

    if (r.item() && r.item()->isStaffLines()) {
        if (!(muse::RealIsEqual(top, staffLinesTop) && muse::RealIsEqual(bottom, staffLinesBottom))) {
            m_staffLineEdges.emplace(round(x), StaffLineEdge(top, bottom));
        }
    } else if (hasValidStaffLineEdges()) {
        if (m_isNorth && muse::RealIsEqualOrMore(top, staffLinesTop)) {
            return; // Only add if it pokes above the staff
        }
        if (!m_isNorth && muse::RealIsEqualOrLess(bottom, staffLinesBottom)) {
            return; // Only add if it pokes below the staff
        }
    }

    m_shape.add(r);
}

double SkylineLine::staffLinesTopAtX(double x) const
{
    if (m_staffLineEdges.empty()) {
        return 0.0;
    }

    auto upperBound = m_staffLineEdges.upper_bound(std::round(x)); // iterator to first element in map with key larger than x
    if (upperBound != m_staffLineEdges.begin()) {
        --upperBound; // back by one (i.e. get last element with key smaller than x)
    }

    return (*upperBound).second.top;
}

double SkylineLine::staffLinesBottomAtX(double x) const
{
    if (m_staffLineEdges.empty()) {
        return 0.0;
    }

    auto upperBound = m_staffLineEdges.upper_bound(std::round(x)); // iterator to first element in map with key larger than x
    if (upperBound != m_staffLineEdges.begin()) {
        --upperBound; // back by one (i.e. get last element with key smaller than x)
    }

    return (*upperBound).second.bottom;
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
    m_staffLineEdges.clear();
    m_shape.clear();
}

//-------------------------------------------------------------------
//   minDistance
//    a is located below this skyline.
//    Calculates the minimum distance between two skylines
//-------------------------------------------------------------------

double Skyline::minDistance(const Skyline& s, double minHorizontalClearance) const
{
    return south().minDistance(s.north(), minHorizontalClearance);
}

double SkylineLine::minDistance(const SkylineLine& sl, double minHorizontalClearance) const
{
    return m_shape.minVerticalDistance(sl.m_shape, minHorizontalClearance);
}

double SkylineLine::minDistanceToShapeAbove(const Shape& shapeAbove, double minHorizontalClearance) const
{
    return shapeAbove.minVerticalDistance(m_shape, minHorizontalClearance);
}

double SkylineLine::minDistanceToShapeBelow(const Shape& shapeBelow, double minHorizontalClearance) const
{
    return m_shape.minVerticalDistance(shapeBelow, minHorizontalClearance);
}

double SkylineLine::verticalClearanceAbove(const Shape& shapeAbove) const
{
    return shapeAbove.verticalClearance(m_shape);
}

double SkylineLine::verticalClaranceBelow(const Shape& shapeBelow) const
{
    return m_shape.verticalClearance(shapeBelow);
}

void Skyline::paint(Painter& painter, double lineWidth) const // DEBUG only
{
    painter.save();

    painter.setBrush(BrushStyle::NoBrush);

    for (const ShapeElement& northShapeEl : _north.elements()) {
        painter.setPen(Pen(Color(144, 238, 144), lineWidth));
        for (const ShapeElement& southShapeEl : _south.elements()) {
            if (northShapeEl.item() == southShapeEl.item()) {
                // Paint in different color items that belong to both north and south skyline
                painter.setPen(Pen(Color(144, 191, 191), lineWidth));
                break;
            }
        }
        painter.drawRect(northShapeEl);
    }

    painter.setPen(Pen(Color(144, 144, 238), lineWidth));
    for (const ShapeElement& southShapeEl : _south.elements()) {
        bool alreadyPainted = false;
        for (const ShapeElement& northShapeEl : _north.elements()) {
            if (northShapeEl.item() == southShapeEl.item()) {
                alreadyPainted = true;
                break;
            }
        }
        if (!alreadyPainted) {
            painter.drawRect(southShapeEl);
        }
    }

    painter.restore();
}

bool SkylineLine::valid() const
{
    return !m_shape.empty();
}

SkylineLine& SkylineLine::translateY(double y)
{
    m_shape.translateY(y);
    return *this;
}

double SkylineLine::top(double startX, double endX)
{
    double top = DBL_MAX;
    for (const ShapeElement& element : m_shape.elements()) {
        if (element.right() > startX && element.left() < endX) {
            top = std::min(top, element.top());
        }
    }

    if (top == DBL_MAX) {
        top = 0.0;
    }

    return top;
}

double SkylineLine::bottom(double startX, double endX)
{
    double bottom = -DBL_MAX;
    for (const ShapeElement& element : m_shape.elements()) {
        if (element.right() > startX && element.left() < endX) {
            bottom = std::max(bottom, element.bottom());
        }
    }

    if (bottom == -DBL_MAX) {
        bottom = 0.0;
    }

    return bottom;
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
