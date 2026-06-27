/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Encore 2.x (v0xA6) reader: note-layout fixups, rest dedup, inner-grace marking, TK MIDI/key/staff-key reads.

#include "readers-v0xa6.h"

#include <QDataStream>

#include "elem.h"

namespace mu::iex::enc {
// Sizes are stated in 2-byte units in this format, so these span 14, 20 and 22 bytes on disk.
static constexpr quint8 kCompactRestSize = 7;
static constexpr quint8 kCompactNoteSize = 10;
static constexpr quint8 kCompactNoteWithArticSize = 11;

// v0xA6 inner-grace detection: after a leading grace note (grace1 & 0x30 == 0x20),
// subsequent NORMAL notes with (grace1 & 0x30) == 0x10 and a strictly larger faceValue
// (shorter duration) are inner graces routed through the grace path in the emitters.
static void markInnerGraces(std::vector<EncMeasureElem*>& elems)
{
    quint8 leadingFv = 0;
    for (EncMeasureElem* e : elems) {
        EncNote* en = dynamic_cast<EncNote*>(e);
        if (!en || en->size != kCompactNoteSize) {
            leadingFv = 0;
            continue;
        }
        if (en->graceType() != EncGraceType::NORMAL) {
            if (leadingFv == 0) {
                leadingFv = en->faceValue & 0x0F;
            }
            continue;
        }
        if ((en->grace1 & 0x30) == 0x10 && leadingFv != 0) {
            const quint8 fv = en->faceValue & 0x0F;
            if (fv > leadingFv) {
                en->isInnerGrace = true;
                leadingFv = std::max(leadingFv, fv);
                continue;
            }
        }
        leadingFv = 0;
    }
}

// Maps the compact element bodies onto the fields the rest of the importer speaks.
// See ENCORE_FORMAT.md §6.3 Note, The compact note and §v0xA6 rest.
bool EncFormatReader_V0xA6::postProcessElement(EncMeasureElem* elem,
                                               QDataStream& ds,
                                               qint64 rawElemStart) const
{
    EncFormatReader::postProcessElement(elem, ds, rawElemStart);
    if (EncRest* er = dynamic_cast<EncRest*>(elem)) {
        // The compact rest has neither field: +13 is its own duration and +14 the next element.
        if (er->size == kCompactRestSize) {
            er->tuplet = 0;
            er->dotControl = 0;
        }
        return false;
    }

    EncNote* en = dynamic_cast<EncNote*>(elem);
    if (!en) {
        return false;
    }

    if (en->size == kCompactNoteSize || en->size == kCompactNoteWithArticSize) {
        en->semiTonePitch = byteAt(ds, rawElemStart + 11);
        en->tuplet = byteAt(ds, rawElemStart + 7);
        en->position = static_cast<qint8>(byteAt(ds, rawElemStart + 9));
        // The compact body ends at +19: these four fields are not in it.
        en->dotControl = 0;
        en->velocity = 0;
        en->options = 0;
        en->alterationGlyph = 0;
    }

    // The one compact form with an articulation; 0x20 there is a fermata.
    if (en->size == kCompactNoteWithArticSize) {
        en->articulationUp = byteAt(ds, rawElemStart + 18);
    }

    return false;
}

// v0xA6 back-to-back identical RESTs: Encore shows only one; duplicates break voice routing.
bool EncFormatReader_V0xA6::deduplicateRest(
    std::vector<std::unique_ptr<EncMeasureElem> >& elements,
    EncMeasureElem* candidate) const
{
    if (elements.empty()) {
        return false;
    }
    const EncRest* prevR = dynamic_cast<const EncRest*>(elements.back().get());
    const EncRest* curR  = dynamic_cast<const EncRest*>(candidate);
    if (!prevR || !curR) {
        return false;
    }
    if (prevR->tick == curR->tick
        && prevR->staffIdx == curR->staffIdx
        && prevR->voice == curR->voice
        && prevR->faceValue == curR->faceValue) {
        return true;   // drop the duplicate
    }
    return false;
}

// v0xA6 has no 4-byte sentinel; stop when < 4 bytes remain before measEnd.
bool EncFormatReader_V0xA6::isMeasureNearEnd(QDataStream& ds, qint64 measEnd) const
{
    return ds.device()->pos() >= measEnd - 4;
}

void EncFormatReader_V0xA6::postProcessVoiceGroup(
    std::vector<EncMeasureElem*>& elems, qint16) const
{
    markInnerGraces(elems);
}

// v0xA6 MIDI program: byte +52 of the TK content (block start + 60), 1-indexed GM.
bool EncFormatReader_V0xA6::readInstrumentMeta(std::vector<EncInstrument>& instruments,
                                               QDataStream& ds,
                                               const EncRoot& /*file*/) const
{
    for (EncInstrument& instr : instruments) {
        if (instr.contentFilePos < 0) {
            continue;
        }
        const quint8 prg = byteAt(ds, instr.contentFilePos + 52);
        if (prg >= 1 && prg <= 128) {
            instr.midiProgram = static_cast<int>(prg);
        }
        // The channels sit immediately before the program byte, one per voice, four of them in
        // this generation against the eight of the later ones. The first is the staff's channel.
        const quint8 chan = byteAt(ds, instr.contentFilePos + 48);
        if (chan < 16) {
            instr.midiChannel = static_cast<int>(chan) + 1;
        }
    }
    return true;
}

void EncFormatReader_V0xA6::readKeyFromTKBlock(EncInstrument& instr,
                                               QDataStream& ds,
                                               qint64 contentStart) const
{
    const qint8 signedRaw = static_cast<qint8>(byteAt(ds, contentStart + 42));
    if (signedRaw >= -33 && signedRaw <= 24) {
        instr.keyTransposeSemitones = signedRaw;
    }
}

void EncFormatReader_V0xA6::readLineStaffEntries(EncLine& line, QDataStream& ds, qint64 lineContentStart) const
{
    // This generation reports zero staves per system and writes 22-byte staff entries, so
    // EncLine::read walks nothing and staffData stays empty. Read the entries here: each one opens
    // with the display size, then the clef, then the written key, then the staff index, and the
    // 0x0E 0xFC marker two bytes later bounds the run.
    // See ENCORE_FORMAT.md §5.2 System block (LINE), Format 2.50 systems.
    QIODevice* dev = ds.device();
    const qint64 savedPos = dev->pos();
    const qint64 entriesStart = lineContentStart + 14;
    for (int i = 0; i < 64; ++i) {
        if (!dev->seek(entriesStart + static_cast<qint64>(i) * 22)) {
            break;
        }
        unsigned char buf[18];
        if (dev->read(reinterpret_cast<char*>(buf), 18) != 18) {
            break;
        }
        if (buf[16] != 0x0E || buf[17] != 0xFC) {
            break;
        }
        line.staffSizes.push_back(buf[12]);
        line.staffClefs.push_back(static_cast<EncClefType>(static_cast<qint8>(buf[13])));
        line.staffKeys.push_back(buf[14]);
    }
    dev->seek(savedPos);
}
} // namespace mu::iex::enc
