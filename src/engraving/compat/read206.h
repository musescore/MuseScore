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

namespace Ms {
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
    static Ms::Score::FileError read206(Ms::MasterScore* masterScore, Ms::XmlReader& e);

    static Ms::EngravingItem* readArticulation(Ms::EngravingItem*, Ms::XmlReader&);
    static void readAccidental206(Ms::Accidental*, Ms::XmlReader&);
    static void readTextStyle206(Ms::MStyle* style, Ms::XmlReader& e, std::map<QString, std::map<Ms::Sid, QVariant> >& excessStyles);
    static void readTextLine206(Ms::XmlReader& e, Ms::TextLineBase* tlb);
    static void readTrill206(Ms::XmlReader& e, Ms::Trill* t);
    static void readHairpin206(Ms::XmlReader& e, Ms::Hairpin* h);
    static void readSlur206(Ms::XmlReader& e, Ms::Slur* s);
    static void readTie206(Ms::XmlReader& e, Ms::Tie* t);

    static bool readNoteProperties206(Ms::Note* note, Ms::XmlReader& e);
    static bool readDurationProperties206(Ms::XmlReader& e, Ms::DurationElement* de);
    static bool readTupletProperties206(Ms::XmlReader& e, Ms::Tuplet* t);
    static bool readChordRestProperties206(Ms::XmlReader& e, Ms::ChordRest* cr);
    static bool readChordProperties206(Ms::XmlReader& e, Ms::Chord* ch);

    static Ms::SymId articulationNames2SymId206(const QString& s);

private:
    static bool readScore206(Ms::Score* score, Ms::XmlReader& e);
    static void readPart206(Ms::Part* part, Ms::XmlReader& e);
};
}

#endif // MU_ENGRAVING_READ206_H
