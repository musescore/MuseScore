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

#ifndef __STAFFTEXTBASE_H__
#define __STAFFTEXTBASE_H__

#include "textbase.h"
#include "staff.h"

namespace mu::engraving {
//---------------------------------------------------------
//   ChannelActions
//---------------------------------------------------------

struct ChannelActions {
    int channel;
    StringList midiActionNames;
};

//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

class StaffTextBase : public TextBase
{
    OBJECT_ALLOCATOR(engraving, StaffTextBase)

    String _channelNames[4];
    std::vector<ChannelActions> _channelActions;
    SwingParameters _swingParameters;
    bool _setAeolusStops { false };
    int m_aeolusStops[4]   { 0, 0, 0, 0 };
    bool _swing          { false };
    int _capo            { 0 };

public:
    StaffTextBase(const ElementType& type, Segment* parent, TextStyleType tid, ElementFlags = ElementFlag::NOTHING);

    void clear();

    virtual void write(XmlWriter& xml) const override;

    Segment* segment() const;
    String channelName(voice_idx_t voice) const { return _channelNames[voice]; }
    void setChannelName(voice_idx_t v, const String& s) { _channelNames[v] = s; }
    void setSwingParameters(int unit, int ratio)
    {
        _swingParameters.swingUnit = unit;
        _swingParameters.swingRatio = ratio;
    }

    const std::vector<ChannelActions>& channelActions() const { return _channelActions; }
    std::vector<ChannelActions>& channelActions() { return _channelActions; }
    const SwingParameters& swingParameters() const { return _swingParameters; }
    void clearAeolusStops();
    void setAeolusStop(int group, int idx, bool val);
    void setAeolusStop(int group, int val);
    bool getAeolusStop(int group, int idx) const;
    int aeolusStop(int group) const;
    void setSetAeolusStops(bool val) { _setAeolusStops = val; }
    void setSwing(bool checked) { _swing = checked; }
    void setCapo(int fretId) { _capo = fretId; }
    bool setAeolusStops() const { return _setAeolusStops; }
    bool swing() const { return _swing; }
    int capo() const { return _capo; }
};
} // namespace mu::engraving
#endif
