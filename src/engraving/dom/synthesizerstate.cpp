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

#include "synthesizerstate.h"

#include "rw/xmlwriter.h"
#include "rw/xmlreader.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SynthesizerState::write(XmlWriter& xml, bool force /* = false */) const
{
    if (isDefault() && !force) {
        return;
    }

    xml.startElement("Synthesizer");
    for (const SynthesizerGroup& g : *this) {
        if (!g.name().isEmpty()) {
            muse::ByteArray ba = g.name().toAscii();
            xml.startElement(ba.constChar());
            for (const IdValue& v : g) {
                xml.tag("val", { { "id", v.id } }, v.data);
            }
            xml.endElement();
        }
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SynthesizerState::read(XmlReader& e)
{
    std::list<SynthesizerGroup> tempGroups;
    while (e.readNextStartElement()) {
        SynthesizerGroup group;
        group.setName(String::fromAscii(e.name().ascii()));

        while (e.readNextStartElement()) {
            if (e.name() == "val") {
                int id = e.intAttribute("id");
                group.push_back(IdValue(id, e.readText()));
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

SynthesizerGroup SynthesizerState::group(const String& name) const
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
    SynthesizerGroup fluid = group(u"Fluid");
    if (fluid.size() == 1) {
        if (fluid.front().data == u"MS Basic.sf3") {
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
    SynthesizerGroup g = group(u"master");

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
                LOGW("Unrecognised CCToUse index from synthesizer: %d", idVal.data.toInt());
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
    SynthesizerGroup g = group(u"master");

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
