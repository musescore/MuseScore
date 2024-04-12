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

#ifndef MU_ENGRAVING_SYNTHESIZERSTATE_H
#define MU_ENGRAVING_SYNTHESIZERSTATE_H

#include <list>

#include "global/allocator.h"
#include "types/string.h"

namespace mu::engraving {
class SynthesizerState;

class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   IdValue
//---------------------------------------------------------

struct IdValue {
    int id = 0;
    muse::String data;

    IdValue() {}
    IdValue(int _id, const muse::String& _data)
        : id(_id), data(_data) {}
};

//---------------------------------------------------------
//   SynthesizerGroup
//---------------------------------------------------------

class SynthesizerGroup : public std::list<IdValue>
{
    OBJECT_ALLOCATOR(engraving, SynthesizerGroup)

public:
    const muse::String& name() const { return m_name; }
    void setName(const muse::String& s) { m_name = s; }

    SynthesizerGroup()
        : std::list<IdValue>() {}
    SynthesizerGroup(const char* n, std::list<IdValue> l)
        : std::list<IdValue>(l), m_name(muse::String::fromUtf8(n)) {}

private:
    muse::String m_name;
};

//---------------------------------------------------------
//   SynthesizerState
//---------------------------------------------------------

class SynthesizerState : public std::list<SynthesizerGroup>
{
    OBJECT_ALLOCATOR(engraving, SynthesizerState)

public:
    SynthesizerState(std::initializer_list<SynthesizerGroup> l)
    {
        insert(end(), l.begin(), l.end());
    }

    SynthesizerState()
        : std::list<SynthesizerGroup>() {}

    void write(XmlWriter&, bool force = false) const;
    void read(XmlReader&);
    SynthesizerGroup group(const muse::String& name) const;
    bool isDefaultSynthSoundfont();
    int ccToUse() const;
    int method() const;
    bool isDefault() const { return m_isDefault; }
    void setIsDefault(bool val) { m_isDefault = val; }

private:

    bool m_isDefault = true;
};
} // namespace mu::engraving
#endif
