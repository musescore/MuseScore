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
// v0xA6 inner-grace detection: after a leading grace note (grace1 & 0x30 == 0x20),
// subsequent NORMAL notes with (grace1 & 0x30) == 0x10 and a strictly larger faceValue
// (shorter duration) are inner graces routed through the grace path in the emitters.
static void markInnerGraces(std::vector<EncMeasureElem*>& elems)
{
    quint8 leadingFv = 0;
    for (EncMeasureElem* e : elems) {
        EncNote* en = dynamic_cast<EncNote*>(e);
        if (!en || en->size != 10) {
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

// v0xA6 NOTE layouts: size=10 (pitch at +11, tuplet at +7), size=11 (same layout plus one
// articulation byte at +18, e.g. fermata 0x20), size=22 (pitch in tuplet slot),
// size<27 (artic bytes lie beyond boundary, zero them). See ENCORE_FORMAT.md §Note element.
bool EncFormatReader_V0xA6::postProcessElement(EncMeasureElem* elem,
                                               QDataStream& ds,
                                               qint64 rawElemStart) const
{
    EncNote* en = dynamic_cast<EncNote*>(elem);
    if (!en) {
        return false;
    }

    if (en->size == 10 || en->size == 11) {
        const qint64 savedPos = ds.device()->pos();
        ds.device()->seek(rawElemStart + 11);
        quint8 pitchByte;
        ds >> pitchByte;
        en->semiTonePitch = pitchByte;
        ds.device()->seek(rawElemStart + 7);
        quint8 tupByte;
        ds >> tupByte;
        en->tuplet = tupByte;
        ds.device()->seek(savedPos);
    }

    if (en->size == 22) {
        en->semiTonePitch = en->tuplet;
        en->tuplet = 0;
    }

    if (en->size < 27) {
        en->articulationUp   = 0;
        en->articulationDown = 0;
        // A size-11 note carries one articulation byte at +18 (size-10 notes never do);
        // 0x20 there is a fermata. The base read pulls articulations from beyond the
        // element boundary, so read the real slot explicitly.
        if (en->size == 11) {
            const qint64 savedPos = ds.device()->pos();
            ds.device()->seek(rawElemStart + 18);
            quint8 articByte;
            ds >> articByte;
            en->articulationUp = articByte;
            ds.device()->seek(savedPos);
        }
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
    const qint64 savedPos = ds.device()->pos();
    for (EncInstrument& instr : instruments) {
        if (instr.contentFilePos < 0) {
            continue;
        }
        if (!ds.device()->seek(instr.contentFilePos + 52)) {
            continue;
        }
        quint8 prg = 0;
        ds >> prg;
        if (prg >= 1 && prg <= 128) {
            instr.midiProgram = static_cast<int>(prg);
        }
    }
    ds.device()->seek(savedPos);
    return true;
}

void EncFormatReader_V0xA6::readKeyFromTKBlock(EncInstrument& instr,
                                               QDataStream& ds,
                                               qint64 contentStart) const
{
    if (!ds.device()->seek(contentStart + 42)) {
        return;
    }
    quint8 raw = 0;
    ds >> raw;
    const qint8 signedRaw = static_cast<qint8>(raw);
    if (signedRaw >= -33 && signedRaw <= 24) {
        instr.keyTransposeSemitones = signedRaw;
    }
    ds.device()->seek(contentStart);
}

void EncFormatReader_V0xA6::readLineStaffKeys(EncLine& line, QDataStream& ds, qint64 lineContentStart) const
{
    // v0xA6 staffPerSystem reads 0 so staffData stays empty; parse the key out of each LINE staff
    // entry directly so initial key signatures import. The 0x0E 0xFC marker bounds the run.
    // See ENCORE_FORMAT.md §v0xA6 staff size and clef.
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
        line.staffKeys.push_back(buf[14]);
    }
    dev->seek(savedPos);
}
} // namespace mu::iex::enc
