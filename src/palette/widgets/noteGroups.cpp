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

#include "noteGroups.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/mcursor.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/key.h"
#include "engraving/dom/actionicon.h"
#include "engraving/dom/staff.h"

#include "translation.h"

using namespace mu::palette;
using namespace mu::engraving;

Score* NoteGroups::createScore(int n, DurationType t, std::vector<Chord*>* chords)
{
    MCursor c;
    c.setTimeSig(_sig);
    c.createScore(iocContext(), u"");
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));
    c.addKeySig(Key::C);

    TimeSig* nts = c.addTimeSig(_sig);
    if (!_z.isEmpty()) {
        nts->setNumeratorString(_z);
    }
    if (!_n.isEmpty()) {
        nts->setDenominatorString(_n);
    }
    GroupNode node { 0, 0 };
    Groups ng;
    ng.addNode(node);
    nts->setGroups(ng);

    for (int i = 0; i < n; ++i) {
        Chord* chord = c.addChord(77, t);
        Fraction tick = chord->rtick();
        chord->setBeamMode(_groups.beamMode(tick.ticks(), t));
        chord->setStemDirection(DirectionV::UP);
        chords->push_back(chord);
    }

    c.score()->style().set(Sid::pageOddLeftMargin, 0.0);
    c.score()->style().set(Sid::pageOddTopMargin, 10.0 / INCH);
    c.score()->style().set(Sid::startBarlineSingle, true);

    StaffType* st = c.score()->staff(0)->staffType(Fraction(0, 1));
    st->setLines(1);            // single line only
    st->setGenClef(false);      // no clef
//    st->setGenTimesig(false); // don't display time sig since NoteGroupsExampleView is unable to reflect custom time sig text/symbols

    return c.score();
}

NoteGroups::NoteGroups(QWidget* parent)
    : QGroupBox(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setupUi(this);

    iconPalette->setName(QT_TRANSLATE_NOOP("palette", "Beam selector"));
    iconPalette->setGridSize(27, 40);
    iconPalette->setDrawGrid(true);

    iconPalette->appendActionIcon(ActionIconType::BEAM_JOIN, "beam-join");
    iconPalette->appendActionIcon(ActionIconType::BEAM_BREAK_LEFT, "beam-break-left");
    iconPalette->appendActionIcon(ActionIconType::BEAM_BREAK_INNER_8TH, "beam-break-inner-8th");
    iconPalette->appendActionIcon(ActionIconType::BEAM_BREAK_INNER_16TH, "beam-break-inner-16th");

    iconPalette->setReadOnly(true);
    iconPalette->setApplyingElementsDisabled(true);
    iconPalette->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    iconPalette->setFixedHeight(iconPalette->heightForWidth(iconPalette->width()));
    iconPalette->setMinimumWidth(27 * 4 * paletteConfiguration()->paletteScaling() + 1); // enough room for all icons, with roundoff
    iconPalette->updateGeometry();

    connect(resetGroups, &QPushButton::clicked, this, &NoteGroups::resetClicked);

    connect(view8, &NoteGroupsExampleView::noteClicked, this, &NoteGroups::noteClicked);
    connect(view16, &NoteGroupsExampleView::noteClicked, this, &NoteGroups::noteClicked);
    connect(view32, &NoteGroupsExampleView::noteClicked, this, &NoteGroups::noteClicked);

    connect(view8, &NoteGroupsExampleView::beamPropertyDropped, this, &NoteGroups::beamPropertyDropped);
    connect(view16, &NoteGroupsExampleView::beamPropertyDropped, this, &NoteGroups::beamPropertyDropped);
    connect(view32, &NoteGroupsExampleView::beamPropertyDropped, this, &NoteGroups::beamPropertyDropped);
}

void NoteGroups::setSig(Fraction sig, const Groups& g, const QString& z, const QString& n)
{
    _sig    = sig;
    _z      = z;
    _n      = n;
    _groups = g;
    chords8.clear();
    chords16.clear();
    chords32.clear();
    Fraction f = _sig.reduced();
    int nn   = f.numerator() * (8 / f.denominator());
    view8->setScore(createScore(nn, DurationType::V_EIGHTH, &chords8));
    nn   = f.numerator() * (16 / f.denominator());
    view16->setScore(createScore(nn, DurationType::V_16TH, &chords16));
    nn   = f.numerator() * (32 / f.denominator());
    view32->setScore(createScore(nn, DurationType::V_32ND, &chords32));
}

Groups NoteGroups::groups()
{
    Groups g;
    for (Chord* chord : chords8) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    for (Chord* chord : chords16) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    for (Chord* chord : chords32) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    return g;
}

void NoteGroups::resetClicked()
{
    setSig(_sig, _groups, _z, _n);
}

void NoteGroups::noteClicked(Note* note)
{
    Chord* chord = note->chord();
    if (chord->beamMode() == BeamMode::AUTO) {
        updateBeams(chord, BeamMode::BEGIN);
    } else {
        updateBeams(chord, BeamMode::AUTO);
    }
}

void NoteGroups::beamPropertyDropped(Chord* chord, ActionIcon* icon)
{
    switch (icon->actionType()) {
    case ActionIconType::BEAM_JOIN:
        updateBeams(chord, BeamMode::AUTO);
        break;
    case ActionIconType::BEAM_BREAK_LEFT:
        updateBeams(chord, BeamMode::BEGIN);
        break;
    case ActionIconType::BEAM_BREAK_INNER_8TH:
        updateBeams(chord, BeamMode::BEGIN16);
        break;
    case ActionIconType::BEAM_BREAK_INNER_16TH:
        updateBeams(chord, BeamMode::BEGIN32);
        break;
    default:
        break;
    }
}

/// takes into account current state of changeShorterCheckBox to update smaller valued notes as well
void NoteGroups::updateBeams(Chord* chord, BeamMode m)
{
    chord->setBeamMode(m);
    chord->score()->doLayout();

    if (changeShorterCheckBox->checkState() == Qt::Checked) {
        Fraction tick = chord->tick();
        bool foundChord = false;
        for (Chord* c : chords8) {
            if (c == chord) {
                foundChord = true;
                break;
            }
        }
        for (Chord* c : chords16) {
            if (foundChord) {
                if (c->tick() == tick) {
                    c->setBeamMode(m);
                    c->score()->doLayout();
                    break;
                }
            } else if (c == chord) {
                foundChord = true;
                break;
            }
        }
        for (Chord* c : chords32) {
            if (foundChord) {
                if (c->tick() == tick) {
                    c->setBeamMode(m);
                    c->score()->doLayout();
                    break;
                }
            }
        }
    }

    view8->update();
    view16->update();
    view32->update();
}
