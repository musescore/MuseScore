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

#include "realizeharmonydialog.h"

#include "libmscore/harmony.h"
#include "libmscore/staff.h"

#include "translation.h"

using namespace mu::notation;

RealizeHarmonyDialog::RealizeHarmonyDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("RealizeHarmonyDialog");
    setupUi(this);

    chordTable->setVisible(false);
    connect(showButton, SIGNAL(clicked()), SLOT(toggleChordTable()));

    voicingSelect->setLiteral(true);

    //make the chord list uneditable
    chordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    INotationInteractionPtr interaction = this->interaction();
    if (!interaction) {
        return;
    }

    std::vector<Ms::EngravingItem*> selectedElements = interaction->selection()->elements();
    QList<Ms::Harmony*> selectedHarmonyList;

    for (Ms::EngravingItem* element : selectedElements) {
        if (element->isHarmony()) {
            selectedHarmonyList << Ms::toHarmony(element);
        }
    }

    setChordList(selectedHarmonyList);
}

RealizeHarmonyDialog::RealizeHarmonyDialog(const RealizeHarmonyDialog& dialog)
    : RealizeHarmonyDialog(dialog.parentWidget())
{
}

INotationInteractionPtr RealizeHarmonyDialog::interaction() const
{
    const INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

void RealizeHarmonyDialog::accept()
{
    INotationInteractionPtr interaction = this->interaction();
    if (!interaction) {
        return;
    }

    bool optionsOverride = optionsBox->isChecked();
    Voicing voicing = optionsOverride ? Voicing(voicingSelect->getVoicing())
                      : Voicing::INVALID;
    HarmonyDurationType durationType = optionsOverride ? HarmonyDurationType(voicingSelect->getDuration())
                                       : HarmonyDurationType::INVALID;

    interaction->realizeSelectedChordSymbols(voicingSelect->getLiteral(), voicing, durationType);

    QDialog::accept();
}

void RealizeHarmonyDialog::toggleChordTable()
{
    int visible = chordTable->isVisible();
    chordTable->setVisible(!visible);
    showButton->setText(!visible ? mu::qtrc("global", "Show less…") : mu::qtrc("global", "Show more…"));
}

//---------------------------------------------------------
//   setChordList
///   fill the chord list with the list of harmonies
//---------------------------------------------------------
void RealizeHarmonyDialog::setChordList(const QList<Harmony*>& hlist)
{
    static const QStringList header = { "ID", "Name", "Intervals", "Notes" };
    QString s;    //chord label string
    int rows = hlist.size();

    chordTable->setRowCount(rows);
    chordTable->setColumnCount(header.size());
    chordTable->setHorizontalHeaderLabels(header);
    for (int i = 0; i < rows; ++i) {
        Harmony* h = hlist.at(i);
        if (!h->isRealizable()) {
            continue;
        }

        s += h->harmonyName() + " ";
        QString intervals;
        QString noteNames;
        int rootTpc;

        //adjust for nashville function
        if (h->harmonyType() == Ms::HarmonyType::NASHVILLE) {
            rootTpc = function2Tpc(h->hFunction(), h->staff()->key(h->tick()));
        } else {
            rootTpc = h->rootTpc();
        }

        noteNames = tpc2name(rootTpc, Ms::NoteSpellingType::STANDARD, Ms::NoteCaseType::AUTO);
        Ms::RealizedHarmony::PitchMap map = h->getRealizedHarmony().notes();
        for (int pitch : map.keys()) {
            intervals += QString::number((pitch - Ms::tpc2pitch(rootTpc)) % 128 % 12) + " ";
        }
        for (int tpc : map.values()) {
            noteNames += ", " + Ms::tpc2name(tpc, Ms::NoteSpellingType::STANDARD, Ms::NoteCaseType::AUTO);
        }

        chordTable->setItem(i, 0, new QTableWidgetItem(QString::number(h->id())));
        chordTable->setItem(i, 1, new QTableWidgetItem(h->harmonyName()));
        chordTable->setItem(i, 2, new QTableWidgetItem(intervals));
        chordTable->setItem(i, 3, new QTableWidgetItem(noteNames));
    }
    chordLabel->setText(s);

    //set table uneditable again
    chordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
