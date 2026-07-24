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

// EncFormatReader interface plus shared block-skip and duration-resolution helpers used by all format readers.

#ifndef MU_IMPORTEXPORT_ENC_PARSER_READER_H
#define MU_IMPORTEXPORT_ENC_PARSER_READER_H

#include <memory>
#include <vector>

#include <QtGlobal>
#include "elem.h"

class QDataStream;

namespace mu::iex::enc {
struct EncMeasureElem;
struct EncInstrument;
struct EncRoot;
struct EncLine;

// Skip an untrusted file-supplied block size. Returns false (so the caller stops) when the size is
// negative or past EOF; chunks the skip because skipRawData takes int and a >INT_MAX size wraps.
bool skipBlock(QDataStream& ds, qint64 size);

// Skip from the cursor to (blockStartPos + declaredLen), where declaredLen is an untrusted varSize.
// Returns false when the cursor is already at/past that end or the end lies past EOF.
bool skipToBlockEnd(QDataStream& ds, qint64 blockStartPos, qint64 declaredLen);

// Device position just past a MEAS element stream, clamped to deviceSize so an oversized varsize
// cannot push the element loop past EOF. Pure so it can be unit-tested with synthetic sizes.
qint64 clampMeasureEnd(qint64 measStart, quint32 varsize, qint64 elemBlockOffset, qint64 deviceSize);

// Phase 1 of duration resolution: set each element's realDuration from the gap to the next event
// (skipping same-tick chord members and near-simultaneous cluster notes), capping the gap at any
// boundaryTicks (mid-measure CLEF/KEYCHANGE) and applying v0xA6 grace time-borrowing when enabled.
// Declared here so the decision core can be unit-tested with synthetic element lists.
void computeElementDurations(std::vector<EncMeasureElem*>& elems, int durTicks, bool hasGraceTimeBorrowing,
                             const std::vector<qint16>& boundaryTicks = {});

// EncFormatReader: per-format binary parsing strategy. Register a new version in EncFormatReader::create().
struct EncFormatReader
{
    // Byte offset where element block begins in a MEAS block. See ENCORE_FORMAT.md §Known quirks.
    virtual quint32 elemBlockOffset() const = 0;

    // Apply format-specific fixups; return true to drop the element (duplicate suppression).
    virtual bool postProcessElement(EncMeasureElem* elem,
                                    QDataStream& ds,
                                    qint64 rawElemStart) const
    {
        (void)elem;
        (void)ds;
        (void)rawElemStart;
        return false;
    }

    // True if the candidate REST is a duplicate and should be dropped.
    virtual bool deduplicateRest(std::vector<std::unique_ptr<EncMeasureElem> >& elements,
                                 EncMeasureElem* candidate) const
    {
        (void)elements;
        (void)candidate;
        return false;
    }

    // Byte stride past one element block.
    virtual qint64 elemSpacing(qint64 rawSize) const { return rawSize; }

    // True when the stream is too close to measEnd for another element.
    virtual bool isMeasureNearEnd(QDataStream& ds, qint64 measEnd) const
    {
        (void)ds;
        (void)measEnd;
        return false;
    }

    // Byte offset where the file header ends; first block starts here.
    // See ENCORE_FORMAT.md §Known quirks for per-version values.
    virtual qint64 headerEnd() const { return 0xC2; }

    // Read MIDI program, Key, and name metadata stored outside TK blocks.
    virtual bool readInstrumentMeta(std::vector<EncInstrument>& instruments,
                                    QDataStream& ds,
                                    const EncRoot& file) const
    {
        (void)instruments;
        (void)ds;
        (void)file;
        return true;
    }

    // True when TK instrument names need UTF-16 probe.
    virtual bool probeInstrumentEncoding() const { return false; }

    // Header byte offset of the global staff-size selector (1-4). v0xC2/C4/C5 use 0x52;
    // v0xA6 uses 0x8D. See ENCORE_FORMAT.md §Header.
    virtual qint64 scoreSizeOffset() const { return 0x52; }

    // Reads Key transposition from TK content (v0xA6). See ENCORE_FORMAT.md §Instrument block.
    virtual void readKeyFromTKBlock(EncInstrument& /*instr*/,
                                    QDataStream& /*ds*/,
                                    qint64 /*contentStart*/) const {}

    // Reads per-staff written key indices from a LINE block into EncLine::staffKeys.
    // v0xA6 stores them in its 22-byte staff entries; other formats fill staffData during
    // EncLine::read and leave staffKeys empty. The override seeks within the stream and must
    // restore the position before returning. See ENCORE_FORMAT.md §v0xA6 staff size and clef.
    virtual void readLineStaffKeys(EncLine& /*line*/,
                                   QDataStream& /*ds*/,
                                   qint64 /*lineContentStart*/) const {}

    // True when a slur's stored xoffset2 lives in a stale coordinate origin and the endpoint
    // must be anchored explicitly (to the target measure / next note) instead of by coordinate
    // search. v0xC2 only. See ENCORE_FORMAT.md §Slur.
    virtual bool slurXoffset2Stale() const { return false; }

    // True when the file stores no importable document margins and a uniform 0.25" default is
    // preferred over MuseScore's A4-tuned defaults. SCO5 (macOS Encore 5) only.
    virtual bool usesUniformPageMargins() const { return false; }

    // Format capability queries, see ENCORE_FORMAT.md §Known quirks for per-version details.
    virtual bool hasGraceTimeBorrowing() const { return false; }  // v0xA6: grace borrows rdur from next note
    virtual const char* formatName() const { return "v0xC4"; }    // for logging

    // True when a chord's notes are recorded with staggered playback ticks (a per-chord "strum")
    // but share one notated horizontal column (the note xoffset byte). When set, a run of notes
    // sharing the same nonzero xoffset and face value is collapsed to one tick before duration
    // computation so they form a single chord. See ENCORE_FORMAT.md §Chord column (xoffset).
    virtual bool clustersChordsByXoffset() const { return false; }

    // Called once per (staffIdx, voice) element group after computeElementDurations().
    // Override to perform format-specific per-voice post-processing:
    //   v0xA6: marks inner-grace notes (isInnerGrace)
    //   v0xC2: fixes dotted-eighth placement and marks implied tuplet members
    virtual void postProcessVoiceGroup(std::vector<EncMeasureElem*>& /*elems*/,
                                       qint16 /*durTicks*/) const {}
    // Bytes to skip between kie (byte +10) and text. v0xC4=9 (text at +20), v0xC2=7 (text at +18).
    // v0xA6 uses 0 (compact lyric: kie at +5, text at +6). See ENCORE_FORMAT.md §Lyric element.
    virtual quint8 lyricTextGapAfterKie() const { return 9; }

    // Bytes from the cursor after the element header (size + rawStaff) to the kie byte. v0xC4/v0xC2
    // use 5 (kie at element +10); v0xA6 uses 0 (kie at +5). See ENCORE_FORMAT.md §Lyric element.
    virtual quint8 lyricPreKieSkip() const { return 5; }

    // Byte offset of the text payload within a TEXT-block entry. v0xC4/v0xC2 use 14 (a per-entry
    // header precedes the text); v0xA6 uses 0 (text starts at the entry). See ENCORE_FORMAT.md §TEXT block.
    virtual quint8 textBlockEntryTextOffset() const { return 14; }

    // True when a TEXT-block entry begins with Encore's rich-text run header: a uint16 run count,
    // a flags word, a run-offset table (run count * uint32), then a 6-byte descriptor before the
    // text. The text offset is then variable (14 is only the single-run case). v0xA6 has no such
    // header. See ENCORE_FORMAT.md §TEXT block.
    virtual bool textBlockEntryHasRunHeader() const { return true; }

    // Byte offset of a STAFFTEXT ornament's TEXT-entry index (tind) measured from the type/voice
    // byte, or -1 to use the size-based location read inline. v0xA6 stores it at +26 in its compact
    // ornament; other formats return -1. See ENCORE_FORMAT.md §Ornament subtypes.
    virtual int staffTextTindOffset() const { return -1; }

    // Byte offset of a STAFFTEXT ornament's vertical placement value (signed Cartesian y, positive =
    // above, negative = below) measured from the type/voice byte, or -1 to use the offset read inline.
    // v0xA6 stores it at +6 in its compact ornament; other formats return -1. See ENCORE_FORMAT.md
    // §Ornament subtypes.
    virtual int staffTextYoffsetOffset() const { return -1; }

    virtual ~EncFormatReader() = default;

    // Factory: returns the reader for the file. The 4-char magic string is needed because some
    // formats are not distinguished by chuMagio (SCO5/macOS Encore 5 shares the v0xC4 format but
    // does not carry chuMagio 0xC4). See create() in readers.cpp.
    static std::unique_ptr<EncFormatReader> create(quint8 chuMagio, const QString& magic);
};
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_PARSER_READER_H
