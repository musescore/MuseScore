/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "mdlmigrator.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/rw/xmlreader.h"
#include "io/file.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::engraving;
using namespace muse;
using namespace muse::io;

void MdlMigrator::remapPercussion()
{
    IF_ASSERT_FAILED(m_score) {
        return;
    }

    for (Part* part : m_score->parts()) {
        const track_idx_t startTrack = part->startTrack();
        const track_idx_t endTrack = part->endTrack();
        const InstrumentList& instruments = part->instruments();

        // First instrument in list is the "default instrument".
        // It's at tick -1 but we must start at tick 0.
        auto it = instruments.cbegin();
        for (Fraction startTick(0, 1), endTick; it != instruments.cend(); startTick = endTick) {
            Instrument* instr = it->second;
            ++it; // careful, iterator now points to next instrument in the list
            endTick = (it == instruments.cend())
                      ? m_score->endTick()
                      : Fraction::fromTicks(it->first);

            if (!instr || !instr->useDrumset() || !instr->musicXmlId().startsWith(u"mdl.")) {
                continue;
            }

            RepitchFunc repitch;
            path_t drumsetPath = globalConfiguration()->appDataPath() + "templates/";

            if (!needToRemap(*instr, repitch, drumsetPath)) {
                continue;
            }

            remapPitches(startTrack, endTrack, startTick, endTick, repitch);

            Drumset* drumset = instr->drumset();

            if (!loadDrumset(drumset, drumsetPath)) {
                Drumset newDrumset;
                newDrumset.clear();
                for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
                    if (drumset->isValid(i)) {
                        newDrumset.setDrum(repitch(i), drumset->drum(i));
                    }
                }
                instr->setDrumset(&newDrumset);
                // Note: The new drumset may have fewer drums than the old one.
                // This happens if there were two pitches X and Y for which
                // repitch(X) == repitch(Y), and the old drumset had drums on
                // both X and Y. The winning drum is the higher of X and Y.
            }
        }
    }
}

bool MdlMigrator::loadDrumset(Drumset* drumset, path_t path)
{
    File file(path);
    if (!file.open(IODevice::ReadOnly)) {
        return false;
    }
    XmlReader xml(&file);
    drumset->clear();
    while (xml.readNextStartElement()) {
        if (xml.name() == "museScore") {
            bool ok = true;
            const String verString = xml.attribute("version");
            const int version = String(verString).remove(u".").toInt(&ok);
            IF_ASSERT_FAILED(ok && version <= Constants::MSC_VERSION) {
                LOGW("Loading drumset from unrecognized MuseScore version <%s>", verString.toAscii().constData());
            }
            while (xml.readNextStartElement()) {
                if (xml.name() == "Drum") {
                    drumset->loadDrum(xml);
                } else {
                    xml.unknown();
                }
            }
        }
    }
    file.close();
    return true;
}

void MdlMigrator::remapPitches(track_idx_t startTrack, track_idx_t endTrack, Fraction startTick, Fraction endTick,
                               const RepitchFunc& repitch)
{
    const auto repitchChord = [repitch] (Chord* chord) {
        for (Note* note : chord->notes()) {
            note->setPitch(repitch(note->pitch()));
            note->setTpcFromPitch(Prefer::SHARPS); // required for playback
        }
    };

    for (
        Segment* seg = m_score->tick2segment(startTick, true, SegmentType::ChordRest);
        seg && seg->tick() < endTick;
        seg = seg->next1(SegmentType::ChordRest)
        ) {
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* el = seg->element(track);
            if (!el || !el->isChord()) {
                continue;
            }
            Chord* ch = toChord(el);
            repitchChord(ch);
            for (Chord* gch : ch->graceNotes()) {
                repitchChord(gch);
            }
        }
    }
}

bool MdlMigrator::needToRemap(const Instrument& instr, RepitchFunc& repitch, path_t& drumsetPath)
{
    const Drumset* drumset = instr.drumset();
    const InstrChannel* channel = instr.channel(0);

    if (!drumset || !channel || channel->synti() != u"Zerberus") {
        return false;
    }

    const int program = channel->program();
    const String musicXmlId = instr.musicXmlId();

    if (musicXmlId == u"mdl.drum.snare-drum" && (program == 5 || program == 6)) {
        // MDL Snare Line   (id="mdl-snareline")
        // MDL Snare Line A (id="mdl-snareline-a")
        // MDL Snare        (id="mdl-snaresolo")
        // MDL Snare A      (id="mdl-snaresolo-a")
        drumsetPath += "Marching_Snare_Drums.drm";
        repitch = repitchMdlSnares;
    } else if (musicXmlId == u"mdl.drum.tenor-drum" && (program == 7 || program == 8)) {
        // MDL Tenor Line   (id="mdl-tenorline")
        // MDL Tenors       (id="mdl-tenorsolo")
        drumsetPath += "Marching_Tenors.drm";
        repitch = repitchMdlTenors;
    } else if (musicXmlId == u"mdl.drum.bass-drum" && program == 0) {
        drumsetPath += "Marching_Bass_Drums.drm";
        if (drumset->isValid(61)) {
            // MDL Bass Line (10)   (id="mdl-bassline-10")
            repitch = repitchMdlBassline10;
        } else {
            // MDL Bass Line (5)    (id="mdl-bassline-5")
            repitch = repitchMdlBassline5;
        }
    } else if (musicXmlId == u"mdl.metal.cymbal.crash" && program == 1) {
        // MDL Cymbal Line  (id="mdl-cymballine")
        drumsetPath += "Marching_Cymbals.drm";
        repitch = repitchMdlCymballine;
    } else {
        return false;
    }
    return true;
}

int MdlMigrator::repitchMdlSnares(int pitch)
{
    switch (pitch) {
    case 23: return 55;
    case 27: return 25;
    case 29: return 92;
    case 30: return 91;
    case 31: return 89;
    case 32: return 58;
    case 33: return 71;
    case 36: return 76;
    case 38: return 40;
    case 39: return 38;
    case 40: return 36;
    case 60: return 50;
    case 61: return 51;
    case 62: return 56;
    case 63: return 53;
    case 64: return 49;
    case 65: return 57;
    case 67: return 60;
    case 68: return 60;
    case 72: return 65;
    case 73: return 67;
    case 74: return 59;
    case 76: return 55;
    case 77: return 24;
    }
    return pitch;
}

int MdlMigrator::repitchMdlTenors(int pitch)
{
    switch (pitch) {
    case 47: return 40;
    case 60: return 36;
    case 61: return 48;
    case 62: return 60;
    case 63: return 72;
    case 64: return 84;
    case 65: return 96;
    case 72: return 41;
    case 73: return 53;
    case 74: return 65;
    case 75: return 77;
    case 76: return 89;
    case 77: return 101;
    case 78: return 37;
    case 79: return 49;
    case 80: return 61;
    case 81: return 73;
    case 82: return 85;
    case 83: return 97;
    case 96: return 37;
    case 97: return 49;
    case 98: return 61;
    case 107: return 36;
    }
    return pitch;
}

int MdlMigrator::repitchMdlBassline10(int pitch)
{
    switch (pitch) {
    case 60: return 90;
    case 61: return 37;
    case 62: return 37;
    case 63: return 49;
    case 64: return 49;
    case 65: return 61;
    case 66: return 61;
    case 67: return 73;
    case 68: return 73;
    case 69: return 85;
    case 70: return 85;
    case 72: return 92;
    case 73: return 39;
    case 74: return 39;
    case 75: return 51;
    case 76: return 51;
    case 77: return 63;
    case 78: return 63;
    case 79: return 75;
    case 80: return 75;
    case 81: return 87;
    case 82: return 87;
    }
    return pitch;
}

int MdlMigrator::repitchMdlBassline5(int pitch)
{
    switch (pitch) {
    case 60: return 90;
    case 62: return 37;
    case 64: return 49;
    case 67: return 61;
    case 68: return 73;
    case 70: return 85;
    case 72: return 92;
    case 74: return 39;
    case 76: return 51;
    case 79: return 63;
    case 80: return 75;
    case 82: return 87;
    }
    return pitch;
}

int MdlMigrator::repitchMdlCymballine(int pitch)
{
    switch (pitch) {
    case 38: return 72;
    case 43: return 81;
    case 45: return 76;
    case 47: return 77;
    case 50: return 84;
    case 51: return 91;
    case 62: return 72;
    case 67: return 81;
    case 69: return 76;
    case 71: return 77;
    case 74: return 84;
    case 75: return 91;
    case 86: return 72;
    case 91: return 81;
    case 93: return 76;
    case 95: return 77;
    case 98: return 84;
    case 99: return 91;
    }
    return pitch;
}
