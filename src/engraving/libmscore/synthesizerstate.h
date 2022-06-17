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

#ifndef __SYNTHESIZERSTATE_H__
#define __SYNTHESIZERSTATE_H__

#include <list>
#include "types/string.h"

namespace mu::engraving {
class XmlWriter;
class XmlReader;
class SynthesizerState;

//---------------------------------------------------------
//   IdValue
//---------------------------------------------------------

struct IdValue {
    int id = 0;
    String data;

    IdValue() {}
    IdValue(int _id, const String& _data)
        : id(_id), data(_data) {}
};

//---------------------------------------------------------
//   SynthesizerGroup
//---------------------------------------------------------

class SynthesizerGroup : public std::list<IdValue>
{
    String _name;

public:
    const String& name() const { return _name; }
    void setName(const String& s) { _name = s; }

    SynthesizerGroup()
        : std::list<IdValue>() {}
    SynthesizerGroup(const char* n, std::list<IdValue> l)
        : std::list<IdValue>(l), _name(n) {}
};

//---------------------------------------------------------
//   SynthesizerState
//---------------------------------------------------------

class SynthesizerState : public std::list<SynthesizerGroup>
{
    bool _isDefault        { true };

public:
    SynthesizerState(std::initializer_list<SynthesizerGroup> l)
    {
        insert(end(), l.begin(), l.end());
    }

    SynthesizerState()
        : std::list<SynthesizerGroup>() {}

    void write(XmlWriter&, bool force = false) const;
    void read(XmlReader&);
    SynthesizerGroup group(const String& name) const;
    bool isDefaultSynthSoundfont();
    int ccToUse() const;
    int method() const;
    bool isDefault() const { return _isDefault; }
    void setIsDefault(bool val) { _isDefault = val; }
};
} // namespace mu::engraving
#endif
