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

#ifndef MU_ENGRAVING_READ206_H
#define MU_ENGRAVING_READ206_H

#include "../ireader.h"

#include "modularity/ioc.h"
#include "iengravingfontsprovider.h"

#include "engravingerrors.h"
#include "style/styledef.h"
#include "libmscore/score.h"

namespace mu::engraving {
class XmlReader;

class EngravingItem;
class Accidental;
class MStyle;
class TextLineBase;
class Trill;
class Hairpin;
class Slur;
class Tie;
class Note;
class DurationElement;
class Tuplet;
class ChordRest;
class Chord;
class Part;
class Score;
}

namespace mu::engraving::read400 {
class ReadContext;
}

namespace mu::engraving::read206 {
class Read206 : public rw::IReader
{
    INJECT_STATIC(IEngravingFontsProvider, engravingFonts)
public:

    //---------------------------------------------------------
    //   read206
    //    import old version > 1.3  and < 3.x files
    //---------------------------------------------------------
    Err readScore(Score* score, XmlReader& e, rw::ReadInOutData* out) override;

    bool pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale) override;
    void pasteSymbols(XmlReader& e, ChordRest* dst) override;

    static EngravingItem* readArticulation(EngravingItem*, XmlReader&, read400::ReadContext& ctx);
    static void readAccidental206(Accidental*, XmlReader&, read400::ReadContext& ctx);
    static void readTextStyle206(MStyle* style, XmlReader& e, read400::ReadContext& ctx, std::map<String, std::map<Sid,
                                                                                                                   PropertyValue> >& excessStyles);
    static void readTextLine206(XmlReader& e, read400::ReadContext& ctx, TextLineBase* tlb);
    static void readTrill206(XmlReader& e, read400::ReadContext& ctx, Trill* t);
    static void readHairpin206(XmlReader& e, read400::ReadContext& ctx, Hairpin* h);
    static void readSlur206(XmlReader& e, read400::ReadContext& ctx, Slur* s);
    static void readTie206(XmlReader& e, read400::ReadContext& ctx, Tie* t);

    static bool readNoteProperties206(Note* note, XmlReader& e, read400::ReadContext& ctx);
    static bool readDurationProperties206(XmlReader& e, read400::ReadContext& ctx, DurationElement* de);
    static bool readTupletProperties206(XmlReader& e, read400::ReadContext& ctx, Tuplet* t);
    static bool readChordRestProperties206(XmlReader& e, read400::ReadContext& ctx, ChordRest* cr);
    static bool readChordProperties206(XmlReader& e, read400::ReadContext& ctx, Chord* ch);

    static SymId articulationNames2SymId206(const AsciiStringView& s);

    static NoteHeadGroup convertHeadGroup(int i);

private:
    void doReadItem(EngravingItem* item, XmlReader& xml) override;

    static bool readScore206(Score* score, XmlReader& e, read400::ReadContext& ctx);
    static void readPart206(Part* part, XmlReader& e, read400::ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_READ206_H
