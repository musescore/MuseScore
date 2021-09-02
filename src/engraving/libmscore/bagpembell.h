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

#include "engravingitem.h"

namespace Ms {
typedef QList<int> noteList;

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name and notes for a BagpipeEmbellishment
//---------------------------------------------------------

struct BagpipeEmbellishmentInfo {
    const char* name;
    QString notes;
};

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name, staff line and pitch for a bagpipe note
//---------------------------------------------------------

struct BagpipeNoteInfo {
    QString name;
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
    int _embelType;
    void drawGraceNote(mu::draw::Painter*, const BEDrawingDataX&, const BEDrawingDataY&, SymId, const qreal x, const bool drawFlag) const;

public:
    BagpipeEmbellishment(EngravingItem* parent)
        : EngravingItem(ElementType::BAGPIPE_EMBELLISHMENT, parent), _embelType(0) { }

    BagpipeEmbellishment* clone() const override { return new BagpipeEmbellishment(*this); }

    int embelType() const { return _embelType; }
    void setEmbelType(int val) { _embelType = val; }
    qreal mag() const override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;
    void layout() override;
    void draw(mu::draw::Painter*) const override;
    static BagpipeEmbellishmentInfo BagpipeEmbellishmentList[];
    static int nEmbellishments();
    static BagpipeNoteInfo BagpipeNoteInfoList[];
    noteList getNoteList() const;
};
}     // namespace Ms
#endif
