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

#ifndef __SLIDE_H__
#define __SLIDE_H__

#include "chordline.h"
#include "note.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class Chord;

//---------------------------------------------------------
//   @@ Slide
///    straight line attached to the note
///    implements slide-in-above, slide-in-below, slide-out-up, slide-out-down
//---------------------------------------------------------

class Slide final : public ChordLine
{
    Slide(Chord* parent);
    Slide(const Slide&);

    bool onTheRight() const { return _chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT; }  // chordlines to the right of the note
    bool onTheLeft() const { return _chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::SCOOP; }  // chordlines to the left of the note
    bool topToBottom() const { return _chordLineType == ChordLineType::DOIT || _chordLineType == ChordLineType::PLOP; }
    bool bottomToTop() const { return _chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::SCOOP; }

    Note* _note = nullptr;

public:

    friend class mu::engraving::Factory;

    Slide* clone() const override { return new Slide(*this); }

    void layout() override;
    void draw(mu::draw::Painter*) const override;
    void setNote(Note* note) { _note = note; }
    Note* note() const { return _note; }
};
}     // namespace Ms
#endif
