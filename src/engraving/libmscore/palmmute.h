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

#ifndef __PALM_MUTE_H__
#define __PALM_MUTE_H__

#include "textlinebase.h"

namespace Ms {
class PalmMute;

//---------------------------------------------------------
//   @@ PalmMuteSegment
//---------------------------------------------------------

class PalmMuteSegment final : public TextLineBaseSegment
{
    Sid getPropertyStyle(Pid) const override;

public:
    PalmMuteSegment(PalmMute* sp, System* parent);

    PalmMuteSegment* clone() const override { return new PalmMuteSegment(*this); }

    PalmMute* palmMute() const { return (PalmMute*)spanner(); }

    void layout() override;

    friend class PalmMute;
};

//---------------------------------------------------------
//   @@ PalmMute
//---------------------------------------------------------

class PalmMute final : public TextLineBase
{
    Sid getPropertyStyle(Pid) const override;

protected:
    mu::PointF linePos(Grip, System**) const override;

public:
    PalmMute(EngravingItem* parent);

    PalmMute* clone() const override { return new PalmMute(*this); }

    void read(XmlReader&) override;
//      virtual void write(XmlWriter& xml) const override;

    LineSegment* createLineSegment(System* parent) override;
    mu::engraving::PropertyValue propertyDefault(Pid propertyId) const override;

    friend class PalmMuteLine;
};
}     // namespace Ms
#endif
