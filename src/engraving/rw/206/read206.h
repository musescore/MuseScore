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

#include "../iscorereader.h"

#include "modularity/ioc.h"
#include "iengravingfontsprovider.h"

#include "engravingerrors.h"
#include "style/styledef.h"
#include "libmscore/score.h"

namespace mu::engraving {
class ReadContext;
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

namespace mu::engraving::compat {
class Read206 : public IScoreReader
{
    INJECT_STATIC(IEngravingFontsProvider, engravingFonts)
public:

    //---------------------------------------------------------
    //   read206
    //    import old version > 1.3  and < 3.x files
    //---------------------------------------------------------
    Err read(Score* score, XmlReader& e, ReadInOutData* out) override;

    static EngravingItem* readArticulation(EngravingItem*, XmlReader&, ReadContext& ctx);
    static void readAccidental206(Accidental*, XmlReader&);
    static void readTextStyle206(MStyle* style, XmlReader& e, std::map<String, std::map<Sid, PropertyValue> >& excessStyles);
    static void readTextLine206(XmlReader& e, ReadContext& ctx, TextLineBase* tlb);
    static void readTrill206(XmlReader& e, Trill* t);
    static void readHairpin206(XmlReader& e, ReadContext& ctx, Hairpin* h);
    static void readSlur206(XmlReader& e, ReadContext& ctx, Slur* s);
    static void readTie206(XmlReader& e, ReadContext& ctx, Tie* t);

    static bool readNoteProperties206(Note* note, XmlReader& e, ReadContext& ctx);
    static bool readDurationProperties206(XmlReader& e, ReadContext& ctx, DurationElement* de);
    static bool readTupletProperties206(XmlReader& e, ReadContext& ctx, Tuplet* t);
    static bool readChordRestProperties206(XmlReader& e, ReadContext& ctx, ChordRest* cr);
    static bool readChordProperties206(XmlReader& e, ReadContext& ctx, Chord* ch);

    static SymId articulationNames2SymId206(const AsciiStringView& s);

    static NoteHeadGroup convertHeadGroup(int i);

private:
    static bool readScore206(Score* score, XmlReader& e, ReadContext& ctx);
    static void readPart206(Part* part, XmlReader& e, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_READ206_H
