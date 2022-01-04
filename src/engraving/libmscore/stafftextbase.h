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

#include "text.h"
#include "part.h"
#include "staff.h"

namespace Ms {
//---------------------------------------------------------
//   ChannelActions
//---------------------------------------------------------

struct ChannelActions {
    int channel;
    QStringList midiActionNames;
};

//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

class StaffTextBase : public TextBase
{
    QString _channelNames[4];
    QList<ChannelActions> _channelActions;
    SwingParameters _swingParameters;
    bool _setAeolusStops { false };
    int aeolusStops[4]   { 0, 0, 0, 0 };
    bool _swing          { false };
    int _capo            { 0 };

public:
    StaffTextBase(const ElementType& type, Segment* parent, TextStyleType tid, ElementFlags = ElementFlag::NOTHING);

    virtual void write(XmlWriter& xml) const override;
    virtual void read(XmlReader&) override;
    virtual bool readProperties(XmlReader&) override;

    Segment* segment() const;
    QString channelName(int voice) const { return _channelNames[voice]; }
    void setChannelName(int v, const QString& s) { _channelNames[v] = s; }
    void setSwingParameters(int unit, int ratio)
    {
        _swingParameters.swingUnit = unit;
        _swingParameters.swingRatio = ratio;
    }

    const QList<ChannelActions>* channelActions() const { return &_channelActions; }
    QList<ChannelActions>* channelActions() { return &_channelActions; }
    const SwingParameters* swingParameters() const { return &_swingParameters; }
    void clearAeolusStops();
    void setAeolusStop(int group, int idx, bool val);
    bool getAeolusStop(int group, int idx) const;
    void setSetAeolusStops(bool val) { _setAeolusStops = val; }
    void setSwing(bool checked) { _swing = checked; }
    void setCapo(int fretId) { _capo = fretId; }
    bool setAeolusStops() const { return _setAeolusStops; }
    bool swing() const { return _swing; }
    int capo() const { return _capo; }
};
}     // namespace Ms
#endif
