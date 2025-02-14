/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionutilities.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/stem.h"

#include "engraving/dom/masterscore.h"

#include "engraving/dom/factory.h"

using namespace mu::notation;
using namespace mu::engraving;

void PercussionUtilities::readDrumset(const muse::ByteArray& drumMapping, Drumset& drumset)
{
    XmlReader reader(drumMapping);

    while (reader.readNextStartElement()) {
        if (reader.name() == "museScore") {
            while (reader.readNextStartElement()) {
                if (reader.name() == "Drum") {
                    drumset.load(reader);
                } else {
                    reader.unknown();
                }
            }
        }
    }
}

/// Returns a drum note prepared for preview.
std::shared_ptr<Chord> PercussionUtilities::getDrumNoteForPreview(const Drumset* drumset, int pitch)
{
    double _spatium = gpaletteScore->style().spatium(); // TODO: Don't use palette here?

    bool up = false;
    int line = drumset->line(pitch);
    NoteHeadGroup noteHead = drumset->noteHead(pitch);
    int voice = drumset->voice(pitch);
    DirectionV dir = drumset->stemDirection(pitch);

    if (dir == DirectionV::UP) {
        up = true;
    } else if (dir == DirectionV::DOWN) {
        up = false;
    } else {
        up = line > 4;
    }

    auto chord = Factory::makeChord(gpaletteScore->dummy()->segment());
    chord->setDurationType(DurationType::V_QUARTER);
    chord->setStemDirection(dir);
    chord->setIsUiItem(true);
    chord->setTrack(voice);

    Note* note = Factory::createNote(chord.get());
    note->setMark(true);
    note->setParent(chord.get());
    note->setTrack(voice);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    note->setLine(line);
    note->setPos(0.0, _spatium * .5 * line);
    note->setHeadGroup(noteHead);

    SymId noteheadSym = SymId::noteheadBlack;
    if (noteHead == NoteHeadGroup::HEAD_CUSTOM) {
        noteheadSym = drumset->noteHeads(pitch, NoteHeadType::HEAD_QUARTER);
    } else {
        noteheadSym = note->noteHead(true, noteHead, NoteHeadType::HEAD_QUARTER);
    }

    note->mutldata()->cachedNoteheadSym.set_value(noteheadSym); // we use the cached notehead so we don't recompute it at each layout
    chord->add(note);

    Stem* stem = Factory::createStem(chord.get());
    stem->setParent(chord.get());
    stem->setBaseLength(Millimetre((up ? -3.0 : 3.0) * _spatium));
    engravingRender()->layoutItem(stem);
    chord->add(stem);

    return chord;
}

/// Opens the percussion shortcut dialog, modifies drumset with user input
void PercussionUtilities::editPercussionShortcut(Drumset& drumset, int originPitch)
{
    IF_ASSERT_FAILED(drumset.isValid(originPitch)) {
        return;
    }

    const muse::RetVal<muse::Val> rv = openPercussionShortcutDialog(drumset, originPitch);
    if (!rv.ret) {
        return;
    }

    const QVariantMap vals = rv.val.toQVariant().toMap();

    drumset.drum(originPitch).shortcut = vals.value("newShortcut").toString();

    const int conflictShortcutPitch = vals.value("conflictDrumPitch").toInt();
    if (conflictShortcutPitch > -1 && drumset.isValid(conflictShortcutPitch)) {
        drumset.drum(conflictShortcutPitch).shortcut.clear();
    }
}

muse::RetVal<muse::Val> PercussionUtilities::openPercussionShortcutDialog(const Drumset& drumset, int originPitch)
{
    muse::UriQuery query("musescore://notation/editpercussionshortcut?sync=true&modal=true");

    const mu::engraving::DrumInstrument& originDrum = drumset.drum(originPitch);
    query.addParam("originDrum", muse::Val::fromQVariant(drumToQVariantMap(originPitch, originDrum)));

    QVariantList drumsWithShortcut;
    for (int otherPitch = 0; otherPitch < mu::engraving::DRUM_INSTRUMENTS; ++otherPitch) {
        if (otherPitch == originPitch || !drumset.isValid(otherPitch) || drumset.shortcut(otherPitch).isEmpty()) {
            continue;
        }
        QVariantMap drumVariant = drumToQVariantMap(otherPitch, drumset.drum(otherPitch));
        drumsWithShortcut << drumVariant;
    }
    query.addParam("drumsWithShortcut", muse::Val::fromQVariant(drumsWithShortcut));

    QVariantList applicationShortcuts;
    for (const muse::ui::UiAction& action : uiactionsRegister()->actionList()) {
        muse::shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
        if (!shortcut.isValid()) {
            continue;
        }
        for (const std::string& str : shortcut.sequences) {
            QVariantMap sc;
            sc ["title"] = action.title.qTranslatedWithoutMnemonic();
            sc ["shortcut"] = QString::fromStdString(str);
            applicationShortcuts << sc;
        }
    }
    query.addParam("applicationShortcuts", muse::Val::fromQVariant(applicationShortcuts));

    return interactive()->open(query);
}

QVariantMap PercussionUtilities::drumToQVariantMap(int pitch, const engraving::DrumInstrument& drum)
{
    QVariantMap qv;
    qv["pitch"] = pitch;
    qv["title"] = drum.name.toQString();
    qv["shortcut"] = drum.shortcut.toQString();
    return qv;
}
