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
using BagpipeNoteList = std::vector<int>;

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name and notes for a BagpipeEmbellishment
//---------------------------------------------------------

struct BagpipeEmbellishmentInfo {
    const char* name = 0;
    AsciiStringView notes;
};

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name, staff line and pitch for a bagpipe note
//---------------------------------------------------------

struct BagpipeNoteInfo {
    AsciiStringView name;
    int line = 0;
    int pitch = 0;
};

//---------------------------------------------------------
//   BagpipeEmbellishment
//    dummy element, used for drag&drop
//---------------------------------------------------------

class BagpipeEmbellishment final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BagpipeEmbellishment)
    DECLARE_CLASSOF(ElementType::BAGPIPE_EMBELLISHMENT)

public:
    BagpipeEmbellishment(EngravingItem* parent)
        : EngravingItem(ElementType::BAGPIPE_EMBELLISHMENT, parent), m_embelType(EmbellishmentType(0)) { }

    BagpipeEmbellishment* clone() const override { return new BagpipeEmbellishment(*this); }

    EmbellishmentType embelType() const { return m_embelType; }
    void setEmbelType(EmbellishmentType val) { m_embelType = val; }
    double mag() const override;

    void draw(mu::draw::Painter*) const override;

    static const BagpipeNoteInfo BAGPIPE_NOTEINFO_LIST[];
    BagpipeNoteList resolveNoteList() const;

    struct LayoutData {
        struct NoteData {
            PointF headXY;
            LineF stemLine;
            PointF flagXY;
            LineF ledgerLine;
        };

        struct BeamData {
            double width = 0.0;         // line width for beam
            double y = 0.0;
            double x1 = 0.0;
            double x2 = 0.0;
        };

        bool isDrawBeam = false;
        bool isDrawFlag = false;
        double spatium = 1.0;           // spatium
        SymId headsym = SymId::noSym;   // grace note head symbol
        SymId flagsym = SymId::noSym;   // grace note flag symbol
        double stemLineW = 0.0;         // line width for stem
        std::map<size_t /*note index*/, NoteData> notesData;
        BeamData beamData;

        RectF bbox;

        bool isValid() const { return bbox.isValid(); }
    };

    const LayoutData& layoutData() const { return m_layoutData; }
    void setLayoutData(const LayoutData& data);

    //! -- Old interface --
    //---------------------------------------------------------
    //   BEDrawingDataX
    //      BagpipeEmbellishment drawing data in the x direction
    //      shared between ::draw() and ::layout()
    //---------------------------------------------------------

    struct BEDrawingDataX {
        const SymId headsym = SymId::noSym; // grace note head symbol
        const SymId flagsym = SymId::noSym; // grace note flag symbol
        const double mags = 1.0;            // grace head magnification
        double headw = 0.0;                 // grace head width
        double headp = 0.0;                 // horizontal head pitch
        const double spatium = 1.0;         // spatium
        const double lw = 0.0;              // line width for stem
        double xl = 0.0;                    // calc x for stem of leftmost note
        const double xcorr = 0.0;           // correction to align flag with top of stem

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

    void oldDraw(mu::draw::Painter* painter) const;
    void drawGraceNote(mu::draw::Painter* painter, const BEDrawingDataX& dx, const BEDrawingDataY& dy, SymId flagsym, const double x,
                       const bool drawFlag) const;
    //! -------------------

private:

    EmbellishmentType m_embelType;
    LayoutData m_layoutData;
};
} // namespace mu::engraving

#endif
