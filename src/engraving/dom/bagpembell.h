/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_BAGPEMBELL_H
#define MU_ENGRAVING_BAGPEMBELL_H

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

    static const BagpipeNoteInfo BAGPIPE_NOTEINFO_LIST[];
    BagpipeNoteList resolveNoteList() const;

    struct LayoutData : public EngravingItem::LayoutData {
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
    };
    DECLARE_LAYOUTDATA_METHODS(BagpipeEmbellishment)

private:

    EmbellishmentType m_embelType;
};
} // namespace mu::engraving

#endif
