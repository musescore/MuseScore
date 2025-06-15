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

#ifndef MU_ENGRAVING_STAFFTEXTBASE_H
#define MU_ENGRAVING_STAFFTEXTBASE_H

#include "textbase.h"
#include "staff.h"

namespace mu::engraving {
//---------------------------------------------------------
//   ChannelActions
//---------------------------------------------------------

struct ChannelActions {
    int channel = 0;
    StringList midiActionNames;
};

//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

class StaffTextBase : public TextBase
{
    OBJECT_ALLOCATOR(engraving, StaffTextBase)

public:
    StaffTextBase(const ElementType& type, Segment* parent, TextStyleType tid, ElementFlags = ElementFlag::NOTHING);

    void clear();

    Segment* segment() const;
    String channelName(voice_idx_t voice) const { return m_channelNames[voice]; }
    void setChannelName(voice_idx_t v, const String& s) { m_channelNames[v] = s; }
    void setSwingParameters(int unit, float ratio)
    {
        m_swingParameters.swingUnit = unit;
        m_swingParameters.swingRatio = ratio;
    }

    const std::vector<ChannelActions>& channelActions() const { return m_channelActions; }
    std::vector<ChannelActions>& channelActions() { return m_channelActions; }
    const SwingParameters& swingParameters() const { return m_swingParameters; }
    void clearAeolusStops();
    void setAeolusStop(int group, int idx, bool val);
    void setAeolusStop(int group, int val);
    bool getAeolusStop(int group, int idx) const;
    int aeolusStop(int group) const;
    void setSetAeolusStops(bool val) { m_setAeolusStops = val; }
    void setSwing(bool checked) { m_swing = checked; }
    void setCapo(int fretId) { m_capo = fretId; }
    bool setAeolusStops() const { return m_setAeolusStops; }
    bool swing() const { return m_swing; }
    int capo() const { return m_capo; }

private:

    String m_channelNames[4];
    std::vector<ChannelActions> m_channelActions;
    SwingParameters m_swingParameters;
    bool m_setAeolusStops = false;
    int m_aeolusStops[4] { 0, 0, 0, 0 };
    bool m_swing = false;
    int m_capo = 0;
};
} // namespace mu::engraving
#endif
