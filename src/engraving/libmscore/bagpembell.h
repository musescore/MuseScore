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

#ifndef __BAGPEMBELL_H__
#define __BAGPEMBELL_H__

#include <vector>

#include "engravingitem.h"

namespace mu::engraving {
typedef std::vector<int> noteList;

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name and notes for a BagpipeEmbellishment
//---------------------------------------------------------

struct BagpipeEmbellishmentInfo {
    const char* name;
    AsciiStringView notes;
};

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name, staff line and pitch for a bagpipe note
//---------------------------------------------------------

struct BagpipeNoteInfo {
    AsciiStringView name;
    int line;
    int pitch;
};

//---------------------------------------------------------
//   BagpipeEmbellishment
//    dummy element, used for drag&drop
//---------------------------------------------------------

class BagpipeEmbellishment final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BagpipeEmbellishment)
    DECLARE_CLASSOF(ElementType::BAGPIPE_EMBELLISHMENT)

    struct BEDrawingDataX;
    struct BEDrawingDataY;

    EmbellishmentType _embelType;
    void drawGraceNote(mu::draw::Painter*, const BagpipeEmbellishment::BEDrawingDataX&, const BagpipeEmbellishment::BEDrawingDataY&, SymId,
                       const double x, const bool drawFlag) const;

public:
    BagpipeEmbellishment(EngravingItem* parent)
        : EngravingItem(ElementType::BAGPIPE_EMBELLISHMENT, parent), _embelType(EmbellishmentType(0)) { }

    BagpipeEmbellishment* clone() const override { return new BagpipeEmbellishment(*this); }

    EmbellishmentType embelType() const { return _embelType; }
    void setEmbelType(EmbellishmentType val) { _embelType = val; }
    double mag() const override;

    void layout() override;
    void draw(mu::draw::Painter*) const override;
    static BagpipeNoteInfo BagpipeNoteInfoList[];
    noteList getNoteList() const;

    //---------------------------------------------------------
    //   BEDrawingDataX
    //      BagpipeEmbellishment drawing data in the x direction
    //      shared between ::draw() and ::layout()
    //---------------------------------------------------------

    struct BEDrawingDataX {
        const SymId headsym;      // grace note head symbol
        const SymId flagsym;      // grace note flag symbol
        const double mags;         // grace head magnification
        double headw;              // grace head width
        double headp;              // horizontal head pitch
        const double spatium;      // spatium
        const double lw;           // line width for stem
        double xl;                 // calc x for stem of leftmost note
        const double xcorr;        // correction to align flag with top of stem

        BEDrawingDataX(SymId hs, SymId fs, const double m, const double s, const int nn);
    };

    //---------------------------------------------------------
    //   BEDrawingDataY
    //      BagpipeEmbellishment drawing data in the y direction
    //      shared between ::draw() and ::layout()
    //---------------------------------------------------------

    struct BEDrawingDataY {
        const double y1b;          // top of all stems for beamed notes
        const double y1f;          // top of stem for note with flag
        const double y2;           // bottom of stem
        const double ycorr;        // correction to align flag with top of stem
        const double bw;           // line width for beam

        BEDrawingDataY(const int l, const double s);
    };
};
} // namespace mu::engraving

#endif
