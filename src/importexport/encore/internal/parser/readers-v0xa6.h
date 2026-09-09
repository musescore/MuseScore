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

#ifndef MU_IMPORTEXPORT_ENC_PARSER_READER_V0XA6_H
#define MU_IMPORTEXPORT_ENC_PARSER_READER_V0XA6_H

#include "readers.h"

namespace mu::iex::enc {
// Encore 2.x (v0xA6) format reader. See ENCORE_FORMAT.md §8.1 Per-generation differences at a glance.
struct EncFormatReader_V0xA6 final : EncFormatReader
{
    qint64 headerEnd() const override { return 0xA6; }

    quint32 elemBlockOffset() const override { return 0x1A; }

    bool postProcessElement(EncMeasureElem* elem, QDataStream& ds, qint64 rawElemStart) const override;

    bool deduplicateRest(std::vector<std::unique_ptr<EncMeasureElem> >& elements, EncMeasureElem* candidate) const override;

    qint64 elemSpacing(qint64 rawSize) const override { return rawSize * 2; }

    bool isMeasureNearEnd(QDataStream& ds, qint64 measEnd) const override;

    bool probeInstrumentEncoding() const override { return false; }

    qint64 scoreSizeOffset() const override { return 0x8D; }

    bool readInstrumentMeta(std::vector<EncInstrument>& instruments, QDataStream& ds, const EncRoot& file) const override;

    void readKeyFromTKBlock(EncInstrument& instr, QDataStream& ds, qint64 contentStart) const override;

    void readLineStaffEntries(EncLine& line, QDataStream& ds, qint64 lineContentStart) const override;

    // This generation draws a cross where the later ones draw a square: a MusicTime score whose
    // percussion staves carry nibble 3 throughout opens in Encore with crosses on every note.
    // Only that value is measured; the rest are assumed to follow the listed vocabulary.
    // See ENCORE_FORMAT.md §6.4 Note.
    quint8 canonicalNoteHeadNibble(quint8 nibble) const override { return nibble == 3 ? 4 : nibble; }

    bool hasGraceTimeBorrowing() const override { return true; }
    // Compact lyric: kie byte immediately after rawStaff (+5), text at +6, no gap.
    quint8 lyricPreKieSkip() const override { return 0; }
    quint8 lyricTextGapAfterKie() const override { return 0; }
    // TEXT entries carry no per-entry header. The compact ornament keeps three fields outside the
    // v0xC4 field order: the staff-text index at +28, a signed-byte y at +9 and the forward measure
    // count at +14. See ENCORE_FORMAT.md §6.8 Ornament.
    quint8 textBlockEntryTextOffset() const override { return 0; }
    bool textBlockEntryHasRunHeader() const override { return false; }
    int staffTextTindOffset() const override { return 28; }
    int ornamentYoffsetOffset() const override { return 9; }
    int ornamentMeasureCountOffset() const override { return 14; }
    const char* formatName() const override { return "v0xA6"; }

    void postProcessVoiceGroup(std::vector<EncMeasureElem*>& elems, qint16 durTicks) const override;
};
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_PARSER_READER_V0XA6_H
