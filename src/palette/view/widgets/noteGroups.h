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

#ifndef __NOTE_GROUPS_H__
#define __NOTE_GROUPS_H__

#include "ui_note_groups.h"

#include "libmscore/groups.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"

namespace Ms {
class Chord;
class Score;

//---------------------------------------------------------
//   NoteGroups
//---------------------------------------------------------

class NoteGroups : public QGroupBox, Ui::NoteGroups
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, paletteConfiguration)

    std::vector<Chord*> chords8;
    std::vector<Chord*> chords16;
    std::vector<Chord*> chords32;
    Groups _groups;
    Fraction _sig;
    QString _z, _n;

    Score* createScore(int n, DurationType t, std::vector<Chord*>* chords);
    void updateBeams(Chord*, BeamMode);

private slots:
    void resetClicked();
    void noteClicked(Note*);
    void beamPropertyDropped(Chord*, ActionIcon*);

public:
    NoteGroups(QWidget* parent);
    void setSig(Fraction sig, const Groups&, const QString& zText, const QString& nText);
    Groups groups();
};
} // namespace Ms
#endif
