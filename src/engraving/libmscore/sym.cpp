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

#include "sym.h"
#include "translation.h"

using namespace mu;
using namespace Ms;

//---------------------------------------------------------
//   commonScoreSymbols
//    subset for use in text palette, possible translations, etc
//---------------------------------------------------------

const std::vector<SymId> Sym::commonScoreSymbols = {
    SymId::accidentalFlat,
    SymId::accidentalNatural,
    SymId::accidentalSharp,
    SymId::accidentalDoubleFlat,
    SymId::accidentalDoubleSharp,
    SymId::metNoteWhole,
    SymId::metNoteHalfUp,
    SymId::metNoteQuarterUp,
    SymId::metNote8thUp,
    SymId::metNote16thUp,
    SymId::metNote32ndUp,
    SymId::metNote64thUp,
    SymId::metNote128thUp,
    SymId::metAugmentationDot,
    SymId::restWholeLegerLine,
    SymId::restHalfLegerLine,
    SymId::restQuarter,
    SymId::rest8th,
    SymId::rest16th,
    SymId::rest32nd,
    SymId::rest64th,
    SymId::rest128th,
    SymId::segno,
    SymId::coda,
    SymId::segnoSerpent1,
    SymId::codaSquare,
    SymId::repeat1Bar,
    SymId::repeat2Bars,
    SymId::repeat4Bars,
    SymId::gClef,
    SymId::fClef,
    SymId::cClef,
    SymId::lyricsElisionNarrow,
    SymId::lyricsElision,
    SymId::lyricsElisionWide,
    SymId::dynamicPiano,
    SymId::dynamicMezzo,
    SymId::dynamicForte,
    SymId::dynamicNiente,
    SymId::dynamicRinforzando,
    SymId::dynamicSforzando,
    SymId::dynamicZ,
    SymId::space
};

// =============================================
// Properties
// =============================================

bool Sym::isValid() const
{
    return m_code != -1 && m_bbox.isValid();
}

const std::vector<SymId>& Sym::subSymbols() const
{
    return m_subSymbolIds;
}

void Sym::setSubSymbols(const std::vector<SymId>& subSymbols)
{
    m_subSymbolIds = subSymbols;
}

int Sym::code() const
{
    return m_code;
}

void Sym::setCode(int val)
{
    m_code = val;
}

// =============================================
// Metrics
// =============================================

RectF Sym::bbox() const
{
    return m_bbox;
}

void Sym::setBbox(RectF val)
{
    m_bbox = val;
}

qreal Sym::advance() const
{
    return m_advance;
}

void Sym::setAdvance(qreal val)
{
    m_advance = val;
}

PointF Sym::smuflAnchor(SmuflAnchorId anchorId)
{
    return m_smuflAnchors[anchorId];
}

void Sym::setSmuflAnchor(SmuflAnchorId anchorId, const PointF& newValue)
{
    m_smuflAnchors[anchorId] = newValue;
}

// =============================================
// Name <-> SymId conversions
// =============================================

SymId Sym::name2id(const QString& s)
{
    return nameToSymIdHash.value(s, SymId::noSym);
}

SymId Sym::oldName2id(const QString s)
{
    return oldNameToSymIdHash.value(s, SymId::noSym);
}

SymId Sym::userName2id(const QString& userName)
{
    int idx = 0;
    for (const char* a : symUserNames) {
        if (a && strcmp(a, qPrintable(userName)) == 0) {
            return SymId(idx);
        }
    }

    return SymId::noSym;
}

const char* Sym::id2name(SymId id)
{
    return symNames[int(id)];
}

QString Sym::id2userName(SymId id)
{
    return mu::qtrc("symUserNames", symUserNames[int(id)]);
}
