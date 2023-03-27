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
#include "chordrw.h"

#include "../../libmscore/chord.h"
#include "../../libmscore/score.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/note.h"
#include "../../libmscore/stem.h"
#include "../../libmscore/stemslash.h"
#include "../../libmscore/hook.h"
#include "../../libmscore/arpeggio.h"
#include "../../libmscore/tremolo.h"
#include "../../libmscore/chordline.h"

#include "../xmlreader.h"

#include "propertyrw.h"
#include "chordrestrw.h"
#include "noterw.h"
#include "stemrw.h"
#include "hookrw.h"
#include "stemslashrw.h"
#include "arpeggiorw.h"
#include "tremolorw.h"
#include "chordlinerw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void ChordRW::read(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (readProperties(ch, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    // Reset horizontal offset of grace notes when migrating from before 4.0
    if (ch->isGrace() && mscVersion < 400) {
        ch->rxoffset() = 0;
    }
}

bool ChordRW::readProperties(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "Note") {
        Note* note = Factory::createNote(ch);
        // the note needs to know the properties of the track it belongs to
        note->setTrack(ch->track());
        note->setParent(ch);
        NoteRW::read(note, e, ctx);
        ch->add(note);
    } else if (ChordRestRW::readProperties(ch, e, ctx)) {
    } else if (tag == "Stem") {
        Stem* s = Factory::createStem(ch);
        StemRW::read(s, e, ctx);
        ch->add(s);
    } else if (tag == "Hook") {
        Hook* hook = new Hook(ch);
        HookRW::read(hook, e, ctx);
        ch->setHook(hook);
        ch->add(hook);
    } else if (tag == "appoggiatura") {
        ch->setNoteType(NoteType::APPOGGIATURA);
        e.readNext();
    } else if (tag == "acciaccatura") {
        ch->setNoteType(NoteType::ACCIACCATURA);
        e.readNext();
    } else if (tag == "grace4") {
        ch->setNoteType(NoteType::GRACE4);
        e.readNext();
    } else if (tag == "grace16") {
        ch->setNoteType(NoteType::GRACE16);
        e.readNext();
    } else if (tag == "grace32") {
        ch->setNoteType(NoteType::GRACE32);
        e.readNext();
    } else if (tag == "grace8after") {
        ch->setNoteType(NoteType::GRACE8_AFTER);
        e.readNext();
    } else if (tag == "grace16after") {
        ch->setNoteType(NoteType::GRACE16_AFTER);
        e.readNext();
    } else if (tag == "grace32after") {
        ch->setNoteType(NoteType::GRACE32_AFTER);
        e.readNext();
    } else if (tag == "StemSlash") {
        StemSlash* ss = Factory::createStemSlash(ch);
        StemSlashRW::read(ss, e, ctx);
        ch->add(ss);
    } else if (PropertyRW::readProperty(ch, tag, e, ctx, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        ch->setNoStem(e.readInt());
    } else if (tag == "Arpeggio") {
        Arpeggio* arpeggio = Factory::createArpeggio(ch);
        arpeggio->setTrack(ch->track());
        ArpeggioRW::read(arpeggio, e, ctx);
        arpeggio->setParent(ch);
        ch->setArpeggio(arpeggio);
    } else if (tag == "Tremolo") {
        Tremolo* tremolo = Factory::createTremolo(ch);
        tremolo->setTrack(ch->track());
        TremoloRW::read(tremolo, e, ctx);
        tremolo->setParent(ch);
        tremolo->setDurationType(ch->durationType());
        ch->setTremolo(tremolo, false);
    } else if (tag == "tickOffset") {     // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = Factory::createChordLine(ch);
        ChordLineRW::read(cl, e, ctx);
        ch->add(cl);
    } else {
        return false;
    }
    return true;
}
