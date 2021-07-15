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

#ifndef __READ206_H__
#define __READ206_H__

#include "draw/geometry.h"

namespace Ms {
class MStyle;

extern Element* readArticulation(Element*, XmlReader&);
extern void readAccidental206(Accidental*, XmlReader&);
extern void readTextStyle206(MStyle* style, XmlReader& e, std::map<QString, std::map<Sid, QVariant> >& excessStyles);
//extern void readText206(XmlReader& e, TextBase* t, Element* be);
// extern void readVolta206(XmlReader& e, Volta* volta);
extern void readTextLine206(XmlReader& e, TextLineBase* tlb);
extern void readTrill206(XmlReader& e, Trill* t);
extern void readHairpin206(XmlReader& e, Hairpin* h);
extern void readSlur206(XmlReader& e, Slur* s);
extern void readTie206(XmlReader& e, Tie* t);

extern bool readNoteProperties206(Note* note, XmlReader& e);
extern bool readDurationProperties206(XmlReader& e, DurationElement* de);
extern bool readTupletProperties206(XmlReader& e, Tuplet* t);
extern bool readChordRestProperties206(XmlReader& e, ChordRest* cr);
extern bool readChordProperties206(XmlReader& e, Chord* ch);
}

#endif
