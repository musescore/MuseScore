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
#include "noterw.h"

#include "../../libmscore/note.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/accidental.h"
#include "../../libmscore/spanner.h"
#include "../../libmscore/fingering.h"
#include "../../libmscore/image.h"
#include "../../libmscore/bend.h"
#include "../../libmscore/notedot.h"
#include "../../libmscore/chord.h"
#include "../../libmscore/chordline.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "propertyrw.h"
#include "symbolrw.h"
#include "imagerw.h"
#include "engravingitemrw.h"
#include "accidentalrw.h"
#include "bendrw.h"
#include "notedotrw.h"
#include "chordlinerw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void NoteRW::read(Note* n, XmlReader& e, ReadContext& ctx)
{
    n->setTpc1(Tpc::TPC_INVALID);
    n->setTpc2(Tpc::TPC_INVALID);

    while (e.readNextStartElement()) {
        if (readProperties(n, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    n->setupAfterRead(ctx.tick(), ctx.pasteMode());
}

bool NoteRW::readProperties(Note* n, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "pitch") {
        n->setPitch(std::clamp(e.readInt(), 0, 127), false);
    } else if (tag == "tpc") {
        int tcp = e.readInt();
        n->setTpc1(tcp);
        n->setTpc2(tcp);
    } else if (tag == "track") {          // for performance
        n->setTrack(e.readInt());
    } else if (tag == "Accidental") {
        Accidental* a = Factory::createAccidental(n);
        a->setTrack(n->track());
        AccidentalRW::read(a, e, ctx);
        n->add(a);
    } else if (tag == "Spanner") {
        Spanner::readSpanner(e, n, n->track());
    } else if (tag == "tpc2") {
        n->setTpc2(e.readInt());
    } else if (tag == "small") {
        n->setSmall(e.readInt());
    } else if (tag == "mirror") {
        PropertyRW::readProperty(n, e, ctx, Pid::MIRROR_HEAD);
    } else if (tag == "dotPosition") {
        PropertyRW::readProperty(n, e, ctx, Pid::DOT_POSITION);
    } else if (tag == "fixed") {
        n->setFixed(e.readBool());
    } else if (tag == "fixedLine") {
        n->setFixedLine(e.readInt());
    } else if (tag == "headScheme") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_SCHEME);
    } else if (tag == "head") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_GROUP);
    } else if (tag == "velocity") {
        n->setUserVelocity(e.readInt());
    } else if (tag == "play") {
        n->setPlay(e.readInt());
    } else if (tag == "tuning") {
        n->setTuning(e.readDouble());
    } else if (tag == "fret") {
        n->setFret(e.readInt());
    } else if (tag == "string") {
        n->setString(e.readInt());
    } else if (tag == "ghost") {
        n->setGhost(e.readInt());
    } else if (tag == "dead") {
        n->setDeadNote(e.readInt());
    } else if (tag == "headType") {
        PropertyRW::readProperty(n, e, ctx, Pid::HEAD_TYPE);
    } else if (tag == "veloType") {
        PropertyRW::readProperty(n, e, ctx, Pid::VELO_TYPE);
    } else if (tag == "line") {
        n->setLine(e.readInt());
    } else if (tag == "Fingering") {
        Fingering* f = Factory::createFingering(n);
        f->setTrack(n->track());
        f->read(e);
        n->add(f);
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(n);
        s->setTrack(n->track());
        SymbolRW::read(s, e, ctx);
        n->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(n);
            image->setTrack(n->track());
            ImageRW::read(image, e, ctx);
            n->add(image);
        }
    } else if (tag == "Bend") {
        Bend* b = Factory::createBend(n);
        b->setTrack(n->track());
        rw400::BendRW::read(b, e, ctx);
        n->add(b);
    } else if (tag == "NoteDot") {
        NoteDot* dot = Factory::createNoteDot(n);
        NoteDotRW::read(dot, e, ctx);
        n->add(dot);
    } else if (tag == "Events") {
        NoteEventList playEvents;
        while (e.readNextStartElement()) {
            const AsciiStringView t(e.name());
            if (t == "Event") {
                NoteEvent ne;
                ne.read(e);
                playEvents.push_back(ne);
            } else {
                e.unknown();
            }
        }
        n->setPlayEvents(playEvents);
        if (n->chord()) {
            n->chord()->setPlayEventType(PlayEventType::User);
        }
    } else if (tag == "offset") {
        EngravingItemRW::readProperties(n, e, ctx);
    } else if (tag == "ChordLine" && n->chord()) {
        ChordLine* cl = Factory::createChordLine(n->chord());
        cl->setNote(n);
        ChordLineRW::read(cl, e, ctx);
        n->chord()->add(cl);
    } else if (EngravingItemRW::readProperties(n, e, ctx)) {
    } else {
        return false;
    }
    return true;
}
