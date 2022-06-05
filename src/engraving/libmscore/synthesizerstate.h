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
#include <QString>

namespace mu::engraving {
class XmlWriter;
class XmlReader;
class SynthesizerState;

//---------------------------------------------------------
//   IdValue
//---------------------------------------------------------

struct IdValue {
    int id = 0;
    QString data;

    IdValue() {}
    IdValue(int _id, const QString& _data)
        : id(_id), data(_data) {}
};

//---------------------------------------------------------
//   SynthesizerGroup
//---------------------------------------------------------

class SynthesizerGroup : public std::list<IdValue>
{
    QString _name;

public:
    const QString& name() const { return _name; }
    void setName(const QString& s) { _name = s; }

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
    SynthesizerGroup group(const QString& name) const;
    bool isDefaultSynthSoundfont();
    int ccToUse() const;
    int method() const;
    bool isDefault() const { return _isDefault; }
    void setIsDefault(bool val) { _isDefault = val; }
};

//---------------------------------------------------------
//   default builtin SynthesizerState
//    used if synthesizer.xml does not exist or is not
//    readable
//---------------------------------------------------------

static SynthesizerState defaultState = {
    { "master", {
          {
              2, "0.1"
          },
          { 3, "440" },
          { 4, "1" },
          { 5, "1" }
      },
    },
    { "Fluid", {
          { 0, "MuseScore_General.sf3" },
      },
    }
};
} // namespace mu::engraving
#endif
