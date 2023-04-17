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

struct BEDrawingDataX;
struct BEDrawingDataY;

//---------------------------------------------------------
//   BagpipeEmbellishment
//    dummy element, used for drag&drop
//---------------------------------------------------------

class BagpipeEmbellishment final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BagpipeEmbellishment)
    DECLARE_CLASSOF(ElementType::BAGPIPE_EMBELLISHMENT)

    EmbellishmentType _embelType;
    void drawGraceNote(mu::draw::Painter*, const BEDrawingDataX&, const BEDrawingDataY&, SymId, const double x, const bool drawFlag) const;

public:
    BagpipeEmbellishment(EngravingItem* parent)
        : EngravingItem(ElementType::BAGPIPE_EMBELLISHMENT, parent), _embelType(EmbellishmentType(0)) { }

    BagpipeEmbellishment* clone() const override { return new BagpipeEmbellishment(*this); }

    EmbellishmentType embelType() const { return _embelType; }
    void setEmbelType(EmbellishmentType val) { _embelType = val; }
    double mag() const override;
    void write(XmlWriter&) const override;
    void layout() override;
    void draw(mu::draw::Painter*) const override;
    static BagpipeNoteInfo BagpipeNoteInfoList[];
    noteList getNoteList() const;
};
} // namespace mu::engraving

#endif
