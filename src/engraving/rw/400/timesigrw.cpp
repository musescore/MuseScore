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
#include "timesigrw.h"

#include "../../libmscore/timesig.h"
#include "../../libmscore/score.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"
#include "propertyrw.h"
#include "tread.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void TimeSigRW::read(TimeSig* s, XmlReader& e, ReadContext& ctx)
{
    int n=0, z1=0, z2=0, z3=0, z4=0;
    bool old = false;

    TimeSigType timeSigType = TimeSigType::NORMAL;
    Fraction sig;
    Fraction stretch(1, 1);
    String numeratorString;
    String denominatorString;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "den") {
            old = true;
            n = e.readInt();
        } else if (tag == "nom1") {
            old = true;
            z1 = e.readInt();
        } else if (tag == "nom2") {
            old = true;
            z2 = e.readInt();
        } else if (tag == "nom3") {
            old = true;
            z3 = e.readInt();
        } else if (tag == "nom4") {
            old = true;
            z4 = e.readInt();
        } else if (tag == "subtype") {
            int i = e.readInt();
            if (s->score()->mscVersion() <= 114) {
                if (i == 0x40000104) {
                    timeSigType = TimeSigType::FOUR_FOUR;
                } else if (i == 0x40002084) {
                    timeSigType = TimeSigType::ALLA_BREVE;
                } else {
                    timeSigType = TimeSigType::NORMAL;
                }
            } else {
                timeSigType = TimeSigType(i);
            }
        } else if (tag == "showCourtesySig") {
            s->setShowCourtesySig(e.readInt());
        } else if (tag == "sigN") {
            sig.setNumerator(e.readInt());
        } else if (tag == "sigD") {
            sig.setDenominator(e.readInt());
        } else if (tag == "stretchN") {
            stretch.setNumerator(e.readInt());
        } else if (tag == "stretchD") {
            stretch.setDenominator(e.readInt());
        } else if (tag == "textN") {
            numeratorString = e.readText();
        } else if (tag == "textD") {
            denominatorString = e.readText();
        } else if (tag == "Groups") {
            Groups groups;
            TRead::read(&groups, e, ctx);
            s->setGroups(groups);
        } else if (PropertyRW::readStyledProperty(s, tag, e, ctx)) {
        } else if (!EngravingItemRW::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
    if (old) {
        sig.set(z1 + z2 + z3 + z4, n);
    }
    stretch.reduce();

    // HACK: handle time signatures from scores before 3.5 differently on some special occasions.
    // See https://musescore.org/node/308139.
    String version = s->score()->mscoreVersion();
    if (!version.isEmpty() && (version >= u"3.0") && (version < u"3.5")) {
        if ((timeSigType == TimeSigType::NORMAL) && !numeratorString.isEmpty() && denominatorString.isEmpty()) {
            if (numeratorString == String::number(sig.numerator())) {
                numeratorString.clear();
            } else {
                denominatorString = String::number(sig.denominator());
            }
        }
    }

    s->setSig(sig, timeSigType);
    s->setStretch(stretch);
    s->setNumeratorString(numeratorString);
    s->setDenominatorString(denominatorString);
}
