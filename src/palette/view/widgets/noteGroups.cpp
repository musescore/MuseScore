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

#include "noteGroups.h"

#include "libmscore/chord.h"
#include "libmscore/mcursor.h"
#include "libmscore/timesig.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/key.h"
#include "libmscore/actionicon.h"
#include "libmscore/staff.h"

#include "translation.h"
#include "ui/view/iconcodes.h"

#include <QHBoxLayout>

using namespace mu::ui;
using namespace mu::actions;

namespace Ms {
struct BeamAction {
    ActionCode code;
    BeamMode mode = BeamMode::BEGIN;
};

static const QList<BeamAction> BEAM_ACTIONS {
    { "beam-start", BeamMode::BEGIN },
    { "beam-mid", BeamMode::AUTO },
    { "beam-32", BeamMode::BEGIN32 },
    { "beam-64", BeamMode::BEGIN64 }
};

Score* NoteGroups::createScore(int n, DurationType t, std::vector<Chord*>* chords)
{
    MCursor c;
    c.setTimeSig(_sig);
    c.createScore("");
    c.addPart("voice");
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
//      st->setGenTimesig(false); // don't display time sig since ExampleView is unable to reflect custom time sig text/symbols

    return c.score();
}

NoteGroups::NoteGroups(QWidget* parent)
    : QGroupBox(parent)
{
    setupUi(this);

    QHBoxLayout* actionsLayout = new QHBoxLayout(this);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);

    groupBox->setLayout(actionsLayout);
    groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    groupBox->setProperty("border-width", 0);

    QString iconFontFamily = QString::fromStdString(uiConfiguration()->iconsFontFamily());
    int iconFontSize = uiConfiguration()->iconsFontSize();

    QString actionButtonStyle = QString("QPushButton { font-family: %1; font-size: %2pt }")
                               .arg(iconFontFamily)
                               .arg(iconFontSize);

    QMap<QPushButton*, BeamMode> buttonToBeamMode;

    for (const BeamAction& beamAction : BEAM_ACTIONS) {
        const UiAction& action = actionsRegister()->action(beamAction.code);
        QChar icon = iconCodeToChar(action.iconCode);

        QPushButton* actionBtn = new QPushButton(icon, this);
        actionBtn->setStyleSheet(actionButtonStyle);
        actionBtn->setFixedSize(30, 30);

        buttonToBeamMode[actionBtn] = beamAction.mode;
        actionsLayout->addWidget(actionBtn);
    }

    for (QPushButton* btn : buttonToBeamMode.keys()) {
        BeamMode mode = buttonToBeamMode[btn];

        connect(btn, &QPushButton::clicked, this, [this, mode, buttonToBeamMode]() {
            m_currentBeamMode = mode;

            for (QPushButton* btn : buttonToBeamMode.keys()) {
                btn->setProperty("accentButton", m_currentBeamMode == buttonToBeamMode[btn]);
                btn->update();
            }
        });
    }

    if (QPushButton* defBtn = buttonToBeamMode.key(BeamMode::BEGIN)) {
        defBtn->setProperty("accentButton", true);
    }

    bool changeShorterNotes = paletteConfiguration()->applyBeamModeToShorterNoteValues();
    changeShorterCheckBox->setChecked(changeShorterNotes);

    connect(changeShorterCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        paletteConfiguration()->setApplyBeamModeToShorterNoteValues(checked);
    });

    connect(resetGroups, &QPushButton::clicked, this, &NoteGroups::resetClicked);

    connect(view8, &ExampleView::noteClicked, this, &NoteGroups::noteClicked);
    connect(view16, &ExampleView::noteClicked, this, &NoteGroups::noteClicked);
    connect(view32, &ExampleView::noteClicked, this, &NoteGroups::noteClicked);
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

    emit beamsUpdated();
}

Groups NoteGroups::groups()
{
    Groups g;
    for (const Chord* chord : chords8) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    for (const Chord* chord : chords16) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    for (const Chord* chord : chords32) {
        g.addStop(chord->rtick().ticks(), chord->durationType().type(), chord->beamMode());
    }
    return g;
}

void NoteGroups::resetClicked()
{
    setSig(_sig, Groups::endings(_sig), _z, _n);
}

void NoteGroups::noteClicked(Note* note)
{
    Chord* chord = note->chord();
    updateBeams(chord, m_currentBeamMode);
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

    emit beamsUpdated();
}
}
