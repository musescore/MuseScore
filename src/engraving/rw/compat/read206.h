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

#include "style/styledef.h"
#include "infrastructure/draw/geometry.h"
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

namespace mu::engraving::compat {
class Read206
{
public:

    //---------------------------------------------------------
    //   read206
    //    import old version > 1.3  and < 3.x files
    //---------------------------------------------------------
    static mu::engraving::Score::FileError read206(mu::engraving::MasterScore* masterScore, mu::engraving::XmlReader& e, ReadContext& ctx);

    static mu::engraving::EngravingItem* readArticulation(mu::engraving::EngravingItem*, mu::engraving::XmlReader&, const ReadContext& ctx);
    static void readAccidental206(mu::engraving::Accidental*, mu::engraving::XmlReader&);
    static void readTextStyle206(mu::engraving::MStyle* style, mu::engraving::XmlReader& e, std::map<QString, std::map<mu::engraving::Sid,
                                                                                                                       PropertyValue> >& excessStyles);
    static void readTextLine206(mu::engraving::XmlReader& e, const ReadContext& ctx, mu::engraving::TextLineBase* tlb);
    static void readTrill206(mu::engraving::XmlReader& e, mu::engraving::Trill* t);
    static void readHairpin206(mu::engraving::XmlReader& e, const ReadContext& ctx, mu::engraving::Hairpin* h);
    static void readSlur206(mu::engraving::XmlReader& e, ReadContext& ctx, mu::engraving::Slur* s);
    static void readTie206(mu::engraving::XmlReader& e, ReadContext& ctx, mu::engraving::Tie* t);

    static bool readNoteProperties206(mu::engraving::Note* note, mu::engraving::XmlReader& e, ReadContext& ctx);
    static bool readDurationProperties206(mu::engraving::XmlReader& e, const ReadContext& ctx, mu::engraving::DurationElement* de);
    static bool readTupletProperties206(mu::engraving::XmlReader& e, const ReadContext& ctx, mu::engraving::Tuplet* t);
    static bool readChordRestProperties206(mu::engraving::XmlReader& e, ReadContext& ctx, mu::engraving::ChordRest* cr);
    static bool readChordProperties206(mu::engraving::XmlReader& e, ReadContext& ctx, mu::engraving::Chord* ch);

    static mu::engraving::SymId articulationNames2SymId206(const AsciiString& s);

    static mu::engraving::NoteHeadGroup convertHeadGroup(int i);

private:
    static bool readScore206(mu::engraving::Score* score, mu::engraving::XmlReader& e, ReadContext& ctx);
    static void readPart206(mu::engraving::Part* part, mu::engraving::XmlReader& e, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_READ206_H
