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

#include "synthesizerstate.h"
#include "rw/xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SynthesizerState::write(XmlWriter& xml, bool force /* = false */) const
{
    if (isDefault() && !force) {
        return;
    }

    xml.startObject("Synthesizer");
    for (const SynthesizerGroup& g : *this) {
        if (!g.name().isEmpty()) {
            xml.startObject(g.name());
            for (const IdValue& v : g) {
                xml.tag(QString("val id=\"%1\"").arg(v.id), v.data);
            }
            xml.endObject();
        }
    }
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SynthesizerState::read(XmlReader& e)
{
    std::list<SynthesizerGroup> tempGroups;
    while (e.readNextStartElement()) {
        SynthesizerGroup group;
        group.setName(e.name().toString());

        while (e.readNextStartElement()) {
            if (e.name() == "val") {
                int id = e.intAttribute("id");
                group.push_back(IdValue(id, e.readElementText()));
            } else {
                e.unknown();
            }
        }
        tempGroups.push_back(group);
    }

    if (!tempGroups.empty()) {
        // Replace any previously set state if we have read a new state
        swap(tempGroups);
        setIsDefault(false);
    }
}

//---------------------------------------------------------
//   group
///  Get SynthesizerGroup by name
//---------------------------------------------------------

SynthesizerGroup SynthesizerState::group(const QString& name) const
{
    for (const SynthesizerGroup& g : *this) {
        if (g.name() == name) {
            return g;
        }
    }
    SynthesizerGroup sg;
    return sg;
}

//---------------------------------------------------------
//   isDefaultSynthSoundfont
///  check if synthesizer state uses default synth and
///  default font only
//---------------------------------------------------------

bool SynthesizerState::isDefaultSynthSoundfont()
{
    SynthesizerGroup fluid = group("Fluid");
    if (fluid.size() == 1) {
        if (fluid.front().data == "MuseScore_General.sf3") {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   ccToUse
//---------------------------------------------------------

int SynthesizerState::ccToUse() const
{
    SynthesizerGroup g = group("master");

    int method = 1;
    int cc = -1;

    for (const IdValue& idVal : g) {
        if (idVal.id == 4) {
            method = idVal.data.toInt();
        } else if (idVal.id == 5) {
            switch (idVal.data.toInt()) {
            case 0:
                cc = 1;
                break;
            case 1:
                cc = 2;
                break;
            case 2:
                cc = 4;
                break;
            case 3:
                cc = 11;
                break;
            default:
                qWarning("Unrecognised CCToUse index from synthesizer: %d", idVal.data.toInt());
            }
        }
    }

    if (method == 0) {        // velocity only
        return -1;
    }

    return cc;
}

//---------------------------------------------------------
//   method
//---------------------------------------------------------

int SynthesizerState::method() const
{
    SynthesizerGroup g = group("master");

    int method = -1;

    for (const IdValue& idVal : g) {
        if (idVal.id == 4) {
            method = idVal.data.toInt();
            break;
        }
    }

    return method;
}
}
