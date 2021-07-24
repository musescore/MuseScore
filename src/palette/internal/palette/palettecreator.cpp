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

// For menus in the menu bar, like File, Edit, and View, see mscore/musescore.cpp

#include "palettecreator.h"

#include <QAction>

#include "actions/actiontypes.h"

#include "libmscore/masterscore.h"
#include "libmscore/note.h"
#include "libmscore/chordrest.h"
#include "libmscore/dynamic.h"
#include "libmscore/slur.h"
#include "libmscore/sym.h"
#include "libmscore/hairpin.h"
#include "libmscore/select.h"
#include "libmscore/tempo.h"
#include "libmscore/segment.h"
#include "libmscore/undo.h"
#include "libmscore/bracket.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/trill.h"
#include "libmscore/pedal.h"
#include "libmscore/clef.h"
#include "libmscore/timesig.h"
#include "libmscore/barline.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/breath.h"
#include "libmscore/arpeggio.h"
#include "libmscore/tremolo.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/tempotext.h"
#include "libmscore/glissando.h"
#include "libmscore/articulation.h"
#include "libmscore/chord.h"
#include "libmscore/drumset.h"
#include "libmscore/spacer.h"
#include "libmscore/measure.h"
#include "libmscore/fret.h"
#include "libmscore/staffstate.h"
#include "libmscore/fingering.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/chordline.h"
#include "libmscore/stafftext.h"
#include "libmscore/systemtext.h"
#include "libmscore/instrchange.h"
#include "libmscore/actionicon.h"
#include "libmscore/accidental.h"
#include "libmscore/harmony.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "libmscore/bagpembell.h"
#include "libmscore/ambitus.h"
#include "libmscore/stafftypechange.h"
#include "libmscore/letring.h"
#include "libmscore/vibrato.h"
#include "libmscore/palmmute.h"
#include "libmscore/fermata.h"
#include "libmscore/measurenumber.h"

#include "palette/palette.h"
#include "translation.h"

using namespace mu::palette;
using namespace mu::actions;
using namespace Ms;

static Palette* toPalette(PalettePanelPtr palettePanel)
{
    return new Palette(palettePanel);
}

void PaletteCreator::populateIconPalettePanel(PalettePanelPtr palettePanel, const PaletteActionIconList& actions)
{
    for (const PaletteActionIcon& paletteAction : actions) {
        const mu::ui::UiAction& action = actionsRegister()->action(paletteAction.actionCode);
        auto icon = makeElement<ActionIcon>(gscore);
        icon->setActionType(paletteAction.actionType);
        icon->setAction(paletteAction.actionCode, static_cast<char16_t>(action.iconCode));
        palettePanel->appendElement(icon, action.title);
    }
}

void PaletteCreator::populateIconPalette(Palette* palette, const PaletteActionIconList& actions)
{
    for (const PaletteActionIcon& paletteAction : actions) {
        const mu::ui::UiAction& action = actionsRegister()->action(paletteAction.actionCode);
        auto icon = makeElement<ActionIcon>(gscore);
        icon->setActionType(paletteAction.actionType);
        icon->setAction(paletteAction.actionCode, static_cast<char16_t>(action.iconCode));
        palette->append(icon, action.title);
    }
}

Palette* PaletteCreator::newBeamPalette()
{
    return toPalette(newBeamPalettePanel());
}

Palette* PaletteCreator::newDynamicsPalette(bool defaultPalette)
{
    return toPalette(newDynamicsPalettePanel(defaultPalette));
}

Palette* PaletteCreator::newKeySigPalette()
{
    return toPalette(newKeySigPalettePanel());
}

Palette* PaletteCreator::newAccidentalsPalette(bool defaultPalette)
{
    return toPalette(newAccidentalsPalettePanel(defaultPalette));
}

Palette* PaletteCreator::newBarLinePalette()
{
    return toPalette(newBarLinePalettePanel());
}

Palette* PaletteCreator::newLayoutPalette()
{
    return toPalette(newLayoutPalettePanel());
}

Palette* PaletteCreator::newRepeatsPalette()
{
    return toPalette(newRepeatsPalettePanel());
}

Palette* PaletteCreator::newFingeringPalette()
{
    return toPalette(newFingeringPalettePanel());
}

Palette* PaletteCreator::newTremoloPalette()
{
    return toPalette(newTremoloPalettePanel());
}

Palette* PaletteCreator::newNoteHeadsPalette()
{
    return toPalette(newNoteHeadsPalettePanel());
}

Palette* PaletteCreator::newArticulationsPalette()
{
    return toPalette(newArticulationsPalettePanel());
}

Palette* PaletteCreator::newOrnamentsPalette()
{
    return toPalette(newOrnamentsPalettePanel());
}

Palette* PaletteCreator::newAccordionPalette()
{
    return toPalette(newAccordionPalettePanel());
}

Palette* PaletteCreator::newBracketsPalette()
{
    return toPalette(newBracketsPalettePanel());
}

Palette* PaletteCreator::newBreathPalette()
{
    return toPalette(newBreathPalettePanel());
}

Palette* PaletteCreator::newArpeggioPalette()
{
    return toPalette(newArpeggioPalettePanel());
}

Palette* PaletteCreator::newClefsPalette(bool defaultPalette)
{
    return toPalette(newClefsPalettePanel(defaultPalette));
}

Palette* PaletteCreator::newGraceNotePalette()
{
    return toPalette(newGraceNotePalettePanel());
}

Palette* PaletteCreator::newBagpipeEmbellishmentPalette()
{
    return toPalette(newBagpipeEmbellishmentPalettePanel());
}

Palette* PaletteCreator::newLinesPalette()
{
    return toPalette(newLinesPalettePanel());
}

struct TempoPattern {
    QString pattern;
    const char* name;
    double f;
    bool relative;
    bool italian;
    bool followText;
    bool basic;
    bool masterOnly;

    TempoPattern(const QString& s, const char* n, double v, bool r, bool i, bool f, bool b, bool m)
        : pattern(s),
        name(n), f(v), relative(r), italian(i), followText(f), basic(b), masterOnly(m) {}
};

Palette* PaletteCreator::newTempoPalette(bool defaultPalette)
{
    return toPalette(newTempoPalettePanel(defaultPalette));
}

Palette* PaletteCreator::newTextPalette(bool defaultPalette)
{
    return toPalette(newTextPalettePanel(defaultPalette));
}

//---------------------------------------------------------
//   newTimePalette
//    create default time signature palette
//---------------------------------------------------------

Palette* PaletteCreator::newTimePalette()
{
    return toPalette(newTimePalettePanel());
}

Palette* PaletteCreator::newFretboardDiagramPalette()
{
    return toPalette(newFretboardDiagramPalettePanel());
}

PaletteTreePtr PaletteCreator::newMasterPaletteTree()
{
    PaletteTreePtr tree = std::make_shared<PaletteTree>();

    tree->append(PaletteCreator::newClefsPalettePanel());
    tree->append(PaletteCreator::newKeySigPalettePanel());
    tree->append(PaletteCreator::newTimePalettePanel());
    tree->append(PaletteCreator::newBracketsPalettePanel());
    tree->append(PaletteCreator::newAccidentalsPalettePanel());
    tree->append(PaletteCreator::newArticulationsPalettePanel());
    tree->append(PaletteCreator::newOrnamentsPalettePanel());
    tree->append(PaletteCreator::newBreathPalettePanel());
    tree->append(PaletteCreator::newGraceNotePalettePanel());
    tree->append(PaletteCreator::newNoteHeadsPalettePanel());
    tree->append(PaletteCreator::newLinesPalettePanel());
    tree->append(PaletteCreator::newBarLinePalettePanel());
    tree->append(PaletteCreator::newArpeggioPalettePanel());
    tree->append(PaletteCreator::newTremoloPalettePanel());
    tree->append(PaletteCreator::newTextPalettePanel());
    tree->append(PaletteCreator::newTempoPalettePanel());
    tree->append(PaletteCreator::newDynamicsPalettePanel());
    tree->append(PaletteCreator::newFingeringPalettePanel());
    tree->append(PaletteCreator::newRepeatsPalettePanel());
    tree->append(PaletteCreator::newFretboardDiagramPalettePanel());
    tree->append(PaletteCreator::newAccordionPalettePanel());
    tree->append(PaletteCreator::newBagpipeEmbellishmentPalettePanel());
    tree->append(PaletteCreator::newLayoutPalettePanel());
    tree->append(PaletteCreator::newBeamPalettePanel());

    return tree;
}

PaletteTreePtr PaletteCreator::newDefaultPaletteTree()
{
    PaletteTreePtr defaultPalette = std::make_shared<PaletteTree>();

    defaultPalette->append(PaletteCreator::newClefsPalettePanel(true));
    defaultPalette->append(PaletteCreator::newKeySigPalettePanel());
    defaultPalette->append(PaletteCreator::newTimePalettePanel());
    defaultPalette->append(PaletteCreator::newBracketsPalettePanel());
    defaultPalette->append(PaletteCreator::newAccidentalsPalettePanel(true));
    defaultPalette->append(PaletteCreator::newArticulationsPalettePanel());
    defaultPalette->append(PaletteCreator::newOrnamentsPalettePanel());
    defaultPalette->append(PaletteCreator::newBreathPalettePanel());
    defaultPalette->append(PaletteCreator::newGraceNotePalettePanel());
    defaultPalette->append(PaletteCreator::newNoteHeadsPalettePanel());
    defaultPalette->append(PaletteCreator::newLinesPalettePanel());
    defaultPalette->append(PaletteCreator::newBarLinePalettePanel());
    defaultPalette->append(PaletteCreator::newArpeggioPalettePanel());
    defaultPalette->append(PaletteCreator::newTremoloPalettePanel());
    defaultPalette->append(PaletteCreator::newTextPalettePanel(true));
    defaultPalette->append(PaletteCreator::newTempoPalettePanel(true));
    defaultPalette->append(PaletteCreator::newDynamicsPalettePanel(true));
    defaultPalette->append(PaletteCreator::newFingeringPalettePanel());
    defaultPalette->append(PaletteCreator::newRepeatsPalettePanel());
    defaultPalette->append(PaletteCreator::newFretboardDiagramPalettePanel());
    defaultPalette->append(PaletteCreator::newAccordionPalettePanel());
    defaultPalette->append(PaletteCreator::newBagpipeEmbellishmentPalettePanel());
    defaultPalette->append(PaletteCreator::newLayoutPalettePanel());
    defaultPalette->append(PaletteCreator::newBeamPalettePanel());

    return defaultPalette;
}

PalettePanelPtr PaletteCreator::newBeamPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Beam);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Beam Properties"));
    sp->setGrid(27, 40);
    sp->setDrawGrid(true);

    static const PaletteActionIconList actions = {
        { ActionIconType::BEAM_START, "beam-start" },
        { ActionIconType::BEAM_MID, "beam-mid" },
        { ActionIconType::BEAM_NONE, "no-beam" },
        { ActionIconType::BEAM_BEGIN_32, "beam32" },
        { ActionIconType::BEAM_BEGIN_64, "beam64" },
        { ActionIconType::BEAM_AUTO, "auto-beam" },
        { ActionIconType::BEAM_FEATHERED_SLOWER, "fbeam1" },
        { ActionIconType::BEAM_FEATHERED_FASTER, "fbeam2" },
        { ActionIconType::UNDEFINED, "" }
    };

    populateIconPalettePanel(sp, actions);
    return sp;
}

PalettePanelPtr PaletteCreator::newDynamicsPalettePanel(bool defaultPalettePanel)
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Dynamic);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Dynamics"));
    sp->setMag(.8);
    sp->setDrawGrid(true);

    static const std::vector<const char*> array1 = {
        "pppppp", "ppppp", "pppp",
        "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
        "ffff", "fffff", "ffffff",
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
        "rfz", "rf", "fz", "m", "r", "s", "z", "n"
    };
    static const std::vector<const char*> arrayDefault = {
        "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
        "rfz", "rf", "fz", "m", "r", "s", "z", "n"
    };

    const std::vector<const char*>* array = nullptr;
    if (defaultPalettePanel) {
        array = &arrayDefault;
        sp->setGrid(42, 28);
        sp->setHasMoreElements(true);
    } else {
        array = &array1;
        sp->setGrid(60, 28);
    }

    for (const char* c :  *array) {
        auto dynamic = makeElement<Dynamic>(Ms::gscore);
        dynamic->setDynamicType(c);
        sp->appendElement(dynamic, dynamic->dynamicTypeName());
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newKeySigPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::KeySig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Key Signatures"));
    sp->setMag(1.0);
    sp->setGrid(56, 55);
    sp->setYOffset(1.0);

    for (int i = 0; i < 7; ++i) {
        auto k = makeElement<KeySig>(Ms::gscore);
        k->setKey(Key(i + 1));
        sp->appendElement(k, keyNames[i * 2]);
    }
    for (int i = -7; i < 0; ++i) {
        auto k = makeElement<KeySig>(gscore);
        k->setKey(Key(i));
        sp->appendElement(k, keyNames[(7 + i) * 2 + 1]);
    }
    auto k = makeElement<KeySig>(gscore);
    k->setKey(Key::C);
    sp->appendElement(k, keyNames[14]);

    // atonal key signature
    KeySigEvent nke;
    nke.setKey(Key::C);
    nke.setCustom(true);
    nke.setMode(KeyMode::NONE);
    auto nk = makeElement<KeySig>(gscore);
    nk->setKeySigEvent(nke);
    sp->appendElement(nk, keyNames[15]);

    return sp;
}

PalettePanelPtr PaletteCreator::newAccidentalsPalettePanel(bool defaultPalettePanel)
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Accidental);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Accidentals"));
    sp->setGrid(33, 36);
    sp->setDrawGrid(true);

    int end = 0;
    if (defaultPalettePanel) {
        end = int(AccidentalType::SHARP_SHARP);
    } else {
        end = int(AccidentalType::END);
    }

    for (int i = int(AccidentalType::FLAT); i < end; ++i) {
        auto ac = makeElement<Accidental>(gscore);
        ac->setAccidentalType(AccidentalType(i));
        if (ac->symbol() != SymId::noSym) {
            sp->appendElement(ac, ac->subtypeUserName());
        }
    }

    if (defaultPalettePanel) {
        sp->setHasMoreElements(true);
    }

    static const PaletteActionIconList actions {
        { ActionIconType::BRACKETS, "add-brackets" },
        { ActionIconType::PARENTHESES, "add-parentheses" },
        { ActionIconType::BRACES, "add-braces" },
    };
    populateIconPalettePanel(sp, actions);

    return sp;
}

PalettePanelPtr PaletteCreator::newBarLinePalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::BarLine);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Barlines"));
    sp->setMag(0.8);
    sp->setGrid(42, 38);

    // bar line styles
    for (unsigned i = 0;; ++i) {
        const BarLineTableItem* bti = BarLine::barLineTableItem(i);
        if (!bti) {
            break;
        }
        auto b = makeElement<BarLine>(gscore);
        b->setBarLineType(bti->type);
        sp->appendElement(b, BarLine::userTypeName(bti->type));
    }

    // bar line spans
    struct {
        int from, to;
        const char* userName;
    } spans[] = {
        { BARLINE_SPAN_TICK1_FROM,  BARLINE_SPAN_TICK1_TO,  Sym::symUserNames[int(SymId::barlineDashed)] },
        { BARLINE_SPAN_TICK2_FROM,  BARLINE_SPAN_TICK2_TO,  QT_TRANSLATE_NOOP("symUserNames", "Tick barline 2") },  // Not in SMuFL
        { BARLINE_SPAN_SHORT1_FROM, BARLINE_SPAN_SHORT1_TO, Sym::symUserNames[int(SymId::barlineShort)] },
        { BARLINE_SPAN_SHORT2_FROM, BARLINE_SPAN_SHORT2_TO, QT_TRANSLATE_NOOP("symUserNames", "Short barline 2") }, // Not in SMuFL
    };
    for (auto span : spans) {
        auto b = makeElement<BarLine>(gscore);
        b->setBarLineType(BarLineType::NORMAL);
        b->setSpanFrom(span.from);
        b->setSpanTo(span.to);
        sp->appendElement(b, span.userName);
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newRepeatsPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Repeat);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Repeats & Jumps"));
    sp->setMag(0.65);
    sp->setGrid(75, 28);
    sp->setDrawGrid(true);

    struct MeasureRepeatInfo
    {
        SymId id = SymId::noSym;
        int measuresCount = 0;
    };

    std::vector<MeasureRepeatInfo> measureRepeats {
        { SymId::repeat1Bar, 1 },
        { SymId::repeat2Bars, 2 },
        { SymId::repeat4Bars, 4 }
    };

    for (MeasureRepeatInfo repeat: measureRepeats) {
        auto rm = makeElement<MeasureRepeat>(gscore);
        rm->setSymId(repeat.id);
        rm->setNumMeasures(repeat.measuresCount);
        sp->appendElement(rm, mu::qtrc("symUserNames", Sym::symUserNames[int(repeat.id)]));
    }

    for (int i = 0; i < markerTypeTableSize(); i++) {
        if (markerTypeTable[i].type == Marker::Type::CODETTA) { // not in SMuFL
            continue;
        }

        auto mk = makeElement<Marker>(gscore);
        mk->setMarkerType(markerTypeTable[i].type);
        mk->styleChanged();
        sp->appendElement(mk, markerTypeTable[i].name);
    }

    for (int i = 0; i < jumpTypeTableSize(); i++) {
        auto jp = makeElement<Jump>(gscore);
        jp->setJumpType(jumpTypeTable[i].type);
        sp->appendElement(jp, jumpTypeTable[i].userText);
    }

    for (unsigned i = 0;; ++i) {
        const BarLineTableItem* bti = BarLine::barLineTableItem(i);
        if (!bti) {
            break;
        }
        switch (bti->type) {
        case BarLineType::START_REPEAT:
        case BarLineType::END_REPEAT:
        case BarLineType::END_START_REPEAT:
            break;
        default:
            continue;
        }

        auto b = makeElement<BarLine>(gscore);
        b->setBarLineType(bti->type);
        PaletteCellPtr cell = sp->appendElement(b, BarLine::userTypeName(bti->type));
        cell->drawStaff = false;
    }

    return sp;
}

PalettePanelPtr PaletteCreator::newLayoutPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Layout);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Layout"));
    sp->setGrid(42, 36);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::LINE);
    PaletteCellPtr cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "System break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "Page break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "Section break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::NOBREAK);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("Palette", "Group measures"));
    cell->mag = 1.2;

    qreal _spatium = gscore->spatium();
    auto spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::DOWN);
    spacer->setGap(3 * _spatium);
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer down"));
    cell->mag = .7;

    spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::UP);
    spacer->setGap(3 * _spatium);
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer up"));
    cell->mag = .7;

    spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::FIXED);
    spacer->setGap(3 * _spatium);
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer fixed down"));
    cell->mag = .7;

    auto stc = makeElement<StaffTypeChange>(gscore);
    sp->appendElement(stc, QT_TRANSLATE_NOOP("palette", "Staff type change"));

    if (configuration()->enableExperimental()) {
        static const PaletteActionIconList actions {
            { ActionIconType::VFRAME, "insert-vbox" },
            { ActionIconType::HFRAME, "insert-hbox" },
            { ActionIconType::TFRAME, "insert-textframe" },
            { ActionIconType::FFRAME, "insert-fretframe" },
            { ActionIconType::MEASURE, "insert-measure" },
        };
        populateIconPalettePanel(sp, actions);
    } else {
        static const PaletteActionIconList actions {
            { ActionIconType::VFRAME, "insert-vbox" },
            { ActionIconType::HFRAME, "insert-hbox" },
            { ActionIconType::TFRAME, "insert-textframe" },
            { ActionIconType::MEASURE, "insert-measure" },
        };
        populateIconPalettePanel(sp, actions);
    }

    return sp;
}

PalettePanelPtr PaletteCreator::newFingeringPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Fingering);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fingering"));
    sp->setMag(1.5);
    sp->setGrid(28, 30);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    const char* finger = "012345";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "Fingering %1"));
    }
    finger = "pimac";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::RH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "RH Guitar Fingering %1"));
    }
    finger = "012345T";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::LH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "LH Guitar Fingering %1"));
    }
    finger = "0123456";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::STRING_NUMBER);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "String number %1"));
    }

    static const std::vector<SymId> lute {
        SymId::stringsThumbPosition,
        SymId::luteFingeringRHThumb, SymId::luteFingeringRHFirst,
        SymId::luteFingeringRHSecond, SymId::luteFingeringRHThird
    };
    // include additional symbol-based fingerings (temporarily?) implemented as articulations
    for (auto i : lute) {
        auto s = makeElement<Articulation>(gscore);
        s->setSymId(i);
        sp->appendElement(s, s->userName());
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newTremoloPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Tremolo);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Tremolos"));
    sp->setGrid(27, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
        auto tremolo = makeElement<Tremolo>(gscore);
        tremolo->setTremoloType(TremoloType(i));
        sp->appendElement(tremolo, tremolo->subtypeName());
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newNoteHeadsPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::NoteHead);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Noteheads"));
    sp->setMag(1.3);
    sp->setGrid(33, 36);
    sp->setDrawGrid(true);

    for (int i = 0; i < int(NoteHead::Group::HEAD_DO_WALKER); ++i) {
        SymId sym = Note::noteHead(0, NoteHead::Group(i), NoteHead::Type::HEAD_HALF);
        // HEAD_BREVIS_ALT shows up only for brevis value
        if (i == int(NoteHead::Group::HEAD_BREVIS_ALT)) {
            sym = Note::noteHead(0, NoteHead::Group(i), NoteHead::Type::HEAD_BREVIS);
        }
        auto nh = makeElement<NoteHead>(gscore);
        nh->setSym(sym);
        sp->appendElement(nh, NoteHead::group2userName(NoteHead::Group(i)));
    }

    static const PaletteActionIconList actions {
        { ActionIconType::PARENTHESES, "add-parentheses" },
    };
    populateIconPalettePanel(sp, actions);

    return sp;
}

PalettePanelPtr PaletteCreator::newArticulationsPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Articulation);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Articulations"));
    sp->setGrid(42, 25);
    sp->setDrawGrid(true);

    static const std::vector<SymId> art {
        SymId::articAccentAbove,
        SymId::articStaccatoAbove,
        SymId::articStaccatissimoAbove,
        SymId::articTenutoAbove,
        SymId::articTenutoStaccatoAbove,
        SymId::articMarcatoAbove,
        SymId::articAccentStaccatoAbove,
        SymId::articLaissezVibrerAbove,
        SymId::articMarcatoStaccatoAbove,
        SymId::articMarcatoTenutoAbove,
        SymId::articStaccatissimoStrokeAbove,
        SymId::articStaccatissimoWedgeAbove,
        SymId::articStressAbove,
        SymId::articTenutoAccentAbove,
        SymId::articUnstressAbove,

        SymId::articSoftAccentAbove,                        // supplemental articulations
        SymId::articSoftAccentStaccatoAbove,
        SymId::articSoftAccentTenutoAbove,
        SymId::articSoftAccentTenutoStaccatoAbove,

        SymId::guitarFadeIn,
        SymId::guitarFadeOut,
        SymId::guitarVolumeSwell,
        SymId::wiggleSawtooth,
        SymId::wiggleSawtoothWide,
        SymId::wiggleVibratoLargeFaster,
        SymId::wiggleVibratoLargeSlowest,
        SymId::brassMuteOpen,
        SymId::brassMuteClosed,
        SymId::stringsHarmonic,
        SymId::stringsUpBow,
        SymId::stringsDownBow,
        SymId::pluckedSnapPizzicatoAbove,
        // SymId::stringsThumbPosition,
        // SymId::luteFingeringRHThumb,
        // SymId::luteFingeringRHFirst,
        // SymId::luteFingeringRHSecond,
        // SymId::luteFingeringRHThird,
    };
    for (auto i : art) {
        auto s = makeElement<Articulation>(gscore);
        s->setSymId(i);
        sp->appendElement(s, s->userName());
    }
    auto bend = makeElement<Bend>(gscore);
    bend->points().append(PitchValue(0,    0, false));
    bend->points().append(PitchValue(15, 100, false));
    bend->points().append(PitchValue(60, 100, false));
    sp->appendElement(bend, QT_TRANSLATE_NOOP("palette", "Bend"));

    auto tb = makeElement<TremoloBar>(gscore);
    tb->points().append(PitchValue(0,     0, false));       // "Dip"
    tb->points().append(PitchValue(30, -100, false));
    tb->points().append(PitchValue(60,    0, false));
    sp->appendElement(tb, QT_TRANSLATE_NOOP("palette", "Tremolo bar"));

    return sp;
}

PalettePanelPtr PaletteCreator::newOrnamentsPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Ornament);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Ornaments"));
    sp->setGrid(42, 25);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
    static const std::vector<SymId> art {
        SymId::ornamentTurnInverted,
        SymId::ornamentTurnSlash,
        SymId::ornamentTurn,
        SymId::ornamentTrill,
        SymId::ornamentShortTrill,
        SymId::ornamentMordent,
        SymId::ornamentTremblement,
        SymId::ornamentPrallMordent,
        SymId::ornamentUpPrall,
        SymId::ornamentPrecompMordentUpperPrefix,           // SymId::ornamentDownPrall,
        SymId::ornamentUpMordent,
        SymId::ornamentDownMordent,
        SymId::ornamentPrallDown,
        SymId::ornamentPrallUp,
        SymId::ornamentLinePrall,
        SymId::ornamentPrecompSlide,
    };
    for (auto i : art) {
        auto s = makeElement<Articulation>(gscore);
        s->setSymId(i);
        sp->appendElement(s, s->userName());
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newAccordionPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Accordion);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Accordion"));
    sp->setGrid(42, 25);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
    static std::vector<SymId> art {
        SymId::accdnCombDot,
        SymId::accdnCombLH2RanksEmpty,
        SymId::accdnCombLH3RanksEmptySquare,
        SymId::accdnCombRH3RanksEmpty,
        SymId::accdnCombRH4RanksEmpty,
        SymId::accdnDiatonicClef,
        SymId::accdnLH2Ranks16Round,
        SymId::accdnLH2Ranks8Plus16Round,
        SymId::accdnLH2Ranks8Round,
        SymId::accdnLH2RanksFullMasterRound,

        SymId::accdnLH2RanksMasterPlus16Round,
        SymId::accdnLH2RanksMasterRound,
        SymId::accdnLH3Ranks2Plus8Square,
        SymId::accdnLH3Ranks2Square,
        SymId::accdnLH3Ranks8Square,
        SymId::accdnLH3RanksDouble8Square,
        SymId::accdnLH3RanksTuttiSquare,
        SymId::accdnPull,
        SymId::accdnPush,
        SymId::accdnRH3RanksAccordion,

        SymId::accdnRH3RanksAuthenticMusette,
        SymId::accdnRH3RanksBandoneon,
        SymId::accdnRH3RanksBassoon,
        SymId::accdnRH3RanksClarinet,
        SymId::accdnRH3RanksDoubleTremoloLower8ve,
        SymId::accdnRH3RanksDoubleTremoloUpper8ve,
        SymId::accdnRH3RanksFullFactory,
        SymId::accdnRH3RanksHarmonium,
        SymId::accdnRH3RanksImitationMusette,
        SymId::accdnRH3RanksLowerTremolo8,

        SymId::accdnRH3RanksMaster,
        SymId::accdnRH3RanksOboe,
        SymId::accdnRH3RanksOrgan,
        SymId::accdnRH3RanksPiccolo,
        SymId::accdnRH3RanksTremoloLower8ve,
        SymId::accdnRH3RanksTremoloUpper8ve,
        SymId::accdnRH3RanksTwoChoirs,
        SymId::accdnRH3RanksUpperTremolo8,
        SymId::accdnRH3RanksViolin,
        SymId::accdnRH4RanksAlto,

        SymId::accdnRH4RanksBassAlto,
        SymId::accdnRH4RanksMaster,
        SymId::accdnRH4RanksSoftBass,
        SymId::accdnRH4RanksSoftTenor,
        SymId::accdnRH4RanksSoprano,
        SymId::accdnRH4RanksTenor,
        SymId::accdnRicochet2,
        SymId::accdnRicochet3,
        SymId::accdnRicochet4,
        SymId::accdnRicochet5,

        SymId::accdnRicochet6,
        SymId::accdnRicochetStem2,
        SymId::accdnRicochetStem3,
        SymId::accdnRicochetStem4,
        SymId::accdnRicochetStem5,
        SymId::accdnRicochetStem6
    };
    for (auto i : art) {
        auto s = makeElement<Symbol>(gscore);
        s->setSym(i);
        sp->appendElement(s, Sym::id2userName(i));
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newBracketsPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Bracket);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Brackets"));
    sp->setMag(0.7);
    sp->setGrid(40, 60);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    std::array<std::pair<BracketType, const char*>, 4> types {
        { { BracketType::NORMAL, QT_TRANSLATE_NOOP("palette", "Bracket") },
            { BracketType::BRACE,  QT_TRANSLATE_NOOP("palette", "Brace") },
            { BracketType::SQUARE, QT_TRANSLATE_NOOP("palette", "Square") },
            { BracketType::LINE,   QT_TRANSLATE_NOOP("palette", "Line") } }
    };

    static Staff bracketItemOwner(gscore);
    bracketItemOwner.setBracketType(static_cast<int>(types.size()) - 1, BracketType::NORMAL);

    for (size_t i = 0; i < types.size(); ++i) {
        auto b1 = makeElement<Bracket>(gscore);
        auto bi1 = bracketItemOwner.brackets()[static_cast<int>(i)];
        const auto& type = types[i];
        bi1->setBracketType(type.first);
        b1->setBracketItem(bi1);
        sp->appendElement(b1, type.second); // Bracket, Brace, Square, Line
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newBreathPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Breath);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Breaths & Pauses"));
    sp->setGrid(42, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    static const std::vector<SymId> fermatas {
        SymId::fermataAbove,
        SymId::fermataShortAbove,
        SymId::fermataLongAbove,
        SymId::fermataLongHenzeAbove,
        SymId::fermataShortHenzeAbove,
        SymId::fermataVeryLongAbove,
        SymId::fermataVeryShortAbove,
    };

    for (auto i : fermatas) {
        auto f = makeElement<Fermata>(gscore);
        f->setSymId(i);
        sp->appendElement(f, f->userName());
    }

    for (BreathType bt : Breath::breathList) {
        auto a = makeElement<Breath>(gscore);
        a->setSymId(bt.id);
        a->setPause(bt.pause);
        sp->appendElement(a, Sym::id2userName(bt.id));
    }

    return sp;
}

PalettePanelPtr PaletteCreator::newArpeggioPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Arpeggio);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Arpeggios & Glissandi"));
    sp->setGrid(27, 50);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = 0; i < 6; ++i) {
        auto a = makeElement<Arpeggio>(gscore);
        a->setArpeggioType(ArpeggioType(i));
        sp->appendElement(a, a->arpeggioTypeName());
    }
    for (int i = 0; i < 2; ++i) {
        auto a = makeElement<Glissando>(gscore);
        a->setGlissandoType(GlissandoType(i));
        sp->appendElement(a, a->glissandoTypeName());
    }

    //fall and doits

    auto cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::FALL);
    sp->appendElement(cl, scorelineNames[0]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::DOIT);
    sp->appendElement(cl, scorelineNames[1]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::PLOP);
    sp->appendElement(cl, scorelineNames[2]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::SCOOP);
    sp->appendElement(cl, scorelineNames[3]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::FALL);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide out down"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::DOIT);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide out up"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::PLOP);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide in above"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::SCOOP);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide in below"));

    return sp;
}

PalettePanelPtr PaletteCreator::newClefsPalettePanel(bool defaultPalettePanel)
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Clef);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Clefs"));
    sp->setMag(0.8);
    sp->setGrid(35, 50);
    sp->setYOffset(1.0);

    static std::vector<ClefType> clefsDefault  {
        ClefType::G,     ClefType::G8_VA,  ClefType::G15_MA,  ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
        ClefType::G8_VB_P,    ClefType::G_1,  ClefType::C1,  ClefType::C2,    ClefType::C3,
        ClefType::C4,    ClefType::C5, ClefType::F,   ClefType::F_8VA, ClefType::F_15MA,
        ClefType::F8_VB,    ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::PERC,
        ClefType::PERC2, ClefType::TAB, ClefType::TAB4
    };
    static std::vector<ClefType> clefsMaster  {
        ClefType::G,     ClefType::G8_VA,  ClefType::G15_MA,  ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
        ClefType::G8_VB_P,    ClefType::G_1,  ClefType::C1,  ClefType::C2,    ClefType::C3,
        ClefType::C4,    ClefType::C5,  ClefType::C_19C, ClefType::C1_F18C, ClefType::C3_F18C, ClefType::C4_F18C,
        ClefType::C1_F20C, ClefType::C3_F20C, ClefType::C4_F20C,
        ClefType::F,   ClefType::F_8VA, ClefType::F_15MA,
        ClefType::F8_VB,    ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::F_F18C, ClefType::F_19C,
        ClefType::PERC,
        ClefType::PERC2, ClefType::TAB, ClefType::TAB4, ClefType::TAB_SERIF, ClefType::TAB4_SERIF
    };

    std::vector<ClefType>* items = nullptr;
    if (defaultPalettePanel) {
        items = &clefsDefault;
        sp->setHasMoreElements(true);
    } else {
        items = &clefsMaster;
        sp->setHasMoreElements(false);
    }

    for (ClefType j : *items) {
        auto k = makeElement<Ms::Clef>(gscore);
        k->setClefType(ClefTypeList(j, j));
        sp->appendElement(k, ClefInfo::name(j));
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newGraceNotePalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::GraceNote);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Grace Notes"));
    sp->setGrid(32, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    static const PaletteActionIconList actions {
        { ActionIconType::ACCIACCATURA,  "acciaccatura" },
        { ActionIconType::APPOGGIATURA,  "appoggiatura" },
        { ActionIconType::GRACE4,        "grace4" },
        { ActionIconType::GRACE16,       "grace16" },
        { ActionIconType::GRACE32,       "grace32" },
        { ActionIconType::GRACE8_AFTER,  "grace8after" },
        { ActionIconType::GRACE16_AFTER, "grace16after" },
        { ActionIconType::GRACE32_AFTER, "grace32after" },
    };
    populateIconPalettePanel(sp, actions);

    return sp;
}

PalettePanelPtr PaletteCreator::newBagpipeEmbellishmentPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::BagpipeEmbellishment);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Bagpipe Embellishments"));
    sp->setMag(0.8);
    sp->setYOffset(2.0);
    sp->setGrid(55, 55);
    sp->setVisible(false);

    for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
        auto b = makeElement<BagpipeEmbellishment>(gscore);
        b->setEmbelType(i);
        sp->appendElement(b, BagpipeEmbellishment::BagpipeEmbellishmentList[i].name);
    }

    return sp;
}

PalettePanelPtr PaletteCreator::newLinesPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Line);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Lines"));
    sp->setMag(.8);
    sp->setGrid(75, 28);
    sp->setDrawGrid(true);

    qreal w = gscore->spatium() * 8;

    auto slur = makeElement<Slur>(gscore);
    sp->appendElement(slur, QT_TRANSLATE_NOOP("palette", "Slur"));

    auto gabel0 = makeElement<Hairpin>(gscore);
    gabel0->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel0->setLen(w);
    sp->appendElement(gabel0, QT_TRANSLATE_NOOP("palette", "Crescendo hairpin"));

    auto gabel1 = makeElement<Hairpin>(gscore);
    gabel1->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    gabel1->setLen(w);
    sp->appendElement(gabel1, QT_TRANSLATE_NOOP("palette", "Diminuendo hairpin"));

    auto gabel2 = makeElement<Hairpin>(gscore);
    gabel2->setHairpinType(HairpinType::CRESC_LINE);
    gabel2->setLen(w);
    sp->appendElement(gabel2, QT_TRANSLATE_NOOP("palette", "Crescendo line"));

    auto gabel3 = makeElement<Hairpin>(gscore);
    gabel3->setHairpinType(HairpinType::DECRESC_LINE);
    gabel3->setLen(w);
    sp->appendElement(gabel3, QT_TRANSLATE_NOOP("palette", "Diminuendo line"));

    auto gabel4 = makeElement<Hairpin>(gscore);
    gabel4->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel4->setBeginText("<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    gabel4->setBeginTextAlign(Align::VCENTER);
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, PropertyFlags::UNSTYLED);
    gabel4->setLen(w);
    sp->appendElement(gabel4, QT_TRANSLATE_NOOP("palette", "Dynamic + hairpin"));

    auto volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("1.");
    QList<int> il;
    il.append(1);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Prima volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("2.");
    il.clear();
    il.append(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("3.");
    il.clear();
    il.append(3);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Terza volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::OPEN);
    volta->setLen(w);
    volta->setText("2.");
    il.clear();
    il.append(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta, open"));

    auto ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "8va alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VB);
    ottava->setLen(w);
    ottava->setPlacement(Placement::BELOW);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "8va bassa"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_15MA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "15ma alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_15MB);
    ottava->setLen(w);
    ottava->setPlacement(Placement::BELOW);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "15ma bassa"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_22MA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "22ma alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_22MB);
    ottava->setPlacement(Placement::BELOW);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "22ma bassa"));

    auto pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginText("<sym>keyboardPedalPed</sym>");
    pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
    pedal->setEndHookType(HookType::HOOK_90);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and line)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginText("<sym>keyboardPedalPed</sym>");
    pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
    pedal->setEndText("<sym>keyboardPedalUp</sym>");
    pedal->setLineVisible(false);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and asterisk)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_90);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (straight hooks)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_45);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled end hook)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_45);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (both hooks angled)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_90);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled start hook)"));

    for (int i = 0; i < trillTableSize(); i++) {
        auto trill = makeElement<Trill>(gscore);
        trill->setTrillType(trillTable[i].type);
        trill->setLen(w);
        sp->appendElement(trill, trillTable[i].userName);
    }

    auto textLine = makeElement<TextLine>(gscore);
    textLine->setLen(w);
    textLine->setBeginText("VII");
    textLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(textLine, QT_TRANSLATE_NOOP("palette", "Text line"));

    auto line = makeElement<TextLine>(gscore);
    line->setLen(w);
    line->setDiagonal(true);
    sp->appendElement(line, QT_TRANSLATE_NOOP("palette", "Line"));

    auto a = makeElement<Ambitus>(gscore);
    sp->appendElement(a, QT_TRANSLATE_NOOP("palette", "Ambitus"));

    auto letRing = makeElement<LetRing>(gscore);
    letRing->setLen(w);
    sp->appendElement(letRing, QT_TRANSLATE_NOOP("palette", "Let Ring"));

    for (int i = 0; i < vibratoTableSize(); i++) {
        auto vibrato = makeElement<Vibrato>(gscore);
        vibrato->setVibratoType(vibratoTable[i].type);
        vibrato->setLen(w);
        sp->appendElement(vibrato, vibratoTable[i].userName);
    }

    auto pm = makeElement<PalmMute>(gscore);
    pm->setLen(w);
    sp->appendElement(pm, QT_TRANSLATE_NOOP("palette", "Palm Mute"));

    return sp;
}

PalettePanelPtr PaletteCreator::newTempoPalettePanel(bool defaultPalettePanel)
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Tempo);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Tempo"));
    sp->setMag(0.65);
    if (defaultPalettePanel) {
        sp->setGrid(66, 28);
    } else {
        sp->setGrid(116, 28);
    }
    sp->setDrawGrid(true);

    static const TempoPattern tps[] = {
        TempoPattern("<sym>metNoteHalfUp</sym> = 80",    QT_TRANSLATE_NOOP("palette",
                                                                           "Half note = 80 BPM"),    80.0 / 30.0, false, false, true, true,
                     false),                                                                                                                                                     // 1/2
        TempoPattern("<sym>metNoteQuarterUp</sym> = 80", QT_TRANSLATE_NOOP("palette",
                                                                           "Quarter note = 80 BPM"), 80.0 / 60.0, false, false, true, true,
                     false),                                                                                                                                                     // 1/4
        TempoPattern("<sym>metNote8thUp</sym> = 80",     QT_TRANSLATE_NOOP("palette",
                                                                           "Eighth note = 80 BPM"),  80.0 / 120.0, false, false, true, true,
                     false),                                                                                                                                                     // 1/8
        TempoPattern("<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette",
                                       "Dotted half note = 80 BPM"),    120 / 30.0, false, false, true, false, false),                                                                                                  // dotted 1/2
        TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette",
                                       "Dotted quarter note = 80 BPM"), 120 / 60.0, false, false, true, true,  false),                                                                                                  // dotted 1/4
        TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette",
                                       "Dotted eighth note = 80 BPM"),  120 / 120.0, false, false, true, false,
                     false),                                                                                                                                                                                            // dotted 1/8

        TempoPattern("Grave",            "Grave",             35.0 / 60.0, false, true, false, false, false),
        TempoPattern("Largo",            "Largo",             50.0 / 60.0, false, true, false, false, false),
        TempoPattern("Lento",            "Lento",             52.5 / 60.0, false, true, false, false, false),
        TempoPattern("Larghetto",        "Larghetto",         63.0 / 60.0, false, true, false, false, true),
        TempoPattern("Adagio",           "Adagio",            71.0 / 60.0, false, true, false, false, false),
        TempoPattern("Andante",          "Andante",           92.0 / 60.0, false, true, false, false, false),
        TempoPattern("Andantino",        "Andantino",         94.0 / 60.0, false, true, false, false, true),
        TempoPattern("Moderato",         "Moderato",         114.0 / 60.0, false, true, false, false, false),
        TempoPattern("Allegretto",       "Allegretto",       116.0 / 60.0, false, true, false, false, false),
        TempoPattern("Allegro moderato", "Allegro moderato", 118.0 / 60.0, false, true, false, false, true),
        TempoPattern("Allegro",          "Allegro",          144.0 / 60.0, false, true, false, false, false),
        TempoPattern("Vivace",           "Vivace",           172.0 / 60.0, false, true, false, false, false),
        TempoPattern("Presto",           "Presto",           187.0 / 60.0, false, true, false, false, false),
        TempoPattern("Prestissimo",      "Prestissimo",      200.0 / 60.0, false, true, false, false, true),

        TempoPattern(
            "<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym>", QT_TRANSLATE_NOOP(
                "palette",
                "Quarter note = dotted quarter note metric modulation"), 3.0 / 2.0, true, false, true, false, false),
        TempoPattern(
            "<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>", QT_TRANSLATE_NOOP(
                "palette",
                "Dotted quarter note = quarter note metric modulation"), 2.0 / 3.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteHalfUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette",
                                       "Half note = quarter note metric modulation"),    1.0 / 2.0, true, false, true, false,
                     false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteHalfUp</sym>",
                     QT_TRANSLATE_NOOP("palette",
                                       "Quarter note = half note metric modulation"),    2.0 / 1.0, true, false, true, false,
                     false),
        TempoPattern("<sym>metNote8thUp</sym> = <sym>metNote8thUp</sym>",
                     QT_TRANSLATE_NOOP("palette",
                                       "Eighth note = eighth note metric modulation"),   1.0 / 1.0, true, false, true, false,
                     false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette",
                                       "Quarter note = quarter note metric modulation"), 1.0 / 1.0, true, false, true, false,
                     false),
        TempoPattern(
            "<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",     QT_TRANSLATE_NOOP(
                "palette",
                "Dotted eighth note = quarter note metric modulation"),  2.0 / 3.0, true, false, true, false, false),
    };

    auto stxt = makeElement<SystemText>(gscore);
    stxt->setTid(Tid::TEMPO);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Swing"));
    stxt->setSwing(true);
    sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "Swing"))->setElementTranslated(true);

    for (TempoPattern tp : tps) {
        auto tt = makeElement<TempoText>(gscore);
        tt->setFollowText(tp.followText);
        tt->setXmlText(tp.pattern);
        if (tp.relative) {
            tt->setRelative(tp.f);
            sp->appendElement(tt, mu::qtrc("palette", tp.name), 1.5);
        } else if (tp.italian) {
            tt->setTempo(tp.f);
            sp->appendElement(tt, tp.name, 1.3);
        } else {
            tt->setTempo(tp.f);
            sp->appendElement(tt, mu::qtrc("palette", tp.name), 1.5);
        }
    }
    sp->setHasMoreElements(false);

    return sp;
}

PalettePanelPtr PaletteCreator::newTextPalettePanel(bool defaultPalettePanel)
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::Text);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Text"));
    sp->setMag(0.85);
    sp->setGrid(84, 28);
    sp->setDrawGrid(true);

    auto st = makeElement<StaffText>(gscore);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Staff Text"));
    sp->appendElement(st, QT_TRANSLATE_NOOP("palette", "Staff text"))->setElementTranslated(true);

    st = makeElement<StaffText>(gscore);
    st->setTid(Tid::EXPRESSION);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Expression"));
    st->setPlacement(Placement::BELOW);
    st->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    sp->appendElement(st, QT_TRANSLATE_NOOP("palette", "Expression text"))->setElementTranslated(true);

    auto is = makeElement<InstrumentChange>(gscore);
    is->setXmlText(QT_TRANSLATE_NOOP("palette", "Change Instr."));
    sp->appendElement(is, QT_TRANSLATE_NOOP("palette", "Instrument change"))->setElementTranslated(true);

    auto rhm = makeElement<RehearsalMark>(gscore);
    rhm->setXmlText("B1");
    sp->appendElement(rhm, QT_TRANSLATE_NOOP("palette", "Rehearsal mark"));

    auto stxt = makeElement<SystemText>(gscore);
    stxt->setTid(Tid::TEMPO);
    /*: System text to switch from swing rhythm back to straight rhythm */
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Straight"));
    // need to be true to enable the "Off" option
    stxt->setSwing(true);
    // 0 (swingUnit) turns of swing; swingRatio is set to default
    stxt->setSwingParameters(0, stxt->score()->styleI(Sid::swingRatio));
    /*: System text to switch from swing rhythm back to straight rhythm */
    sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "Straight"))->setElementTranslated(true);

    stxt = makeElement<SystemText>(gscore);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "System Text"));
    sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "System text"))->setElementTranslated(true);

    // Measure numbers, unlike other elements (but like most text elements),
    // are not copied directly into the score when drop.
    // Instead, they simply set the corresponding measure's MeasureNumberMode to SHOW
    // Because of that, the element shown in the palettes does not have to have any particular formatting.
    auto meaNum = makeElement<MeasureNumber>(gscore);
    meaNum->setProperty(Pid::SUB_STYLE, int(Tid::STAFF));   // Make the element bigger in the palettes (using the default measure number style makes it too small)
    meaNum->setXmlText(QT_TRANSLATE_NOOP("palette", "Measure Number"));
    sp->appendElement(meaNum, QT_TRANSLATE_NOOP("palette", "Measure Number"))->setElementTranslated(true);

    if (!defaultPalettePanel) {
        auto pz = makeElement<StaffText>(gscore);
        pz->setXmlText(QT_TRANSLATE_NOOP("palette", "pizz."));
        pz->setChannelName(0, "pizzicato");
        pz->setChannelName(1, "pizzicato");
        pz->setChannelName(2, "pizzicato");
        pz->setChannelName(3, "pizzicato");
        sp->appendElement(pz, QT_TRANSLATE_NOOP("palette", "Pizzicato"))->setElementTranslated(true);

        auto ar = makeElement<StaffText>(gscore);
        ar->setXmlText(QT_TRANSLATE_NOOP("palette", "arco"));
        ar->setChannelName(0, "arco");
        ar->setChannelName(1, "arco");
        ar->setChannelName(2, "arco");
        ar->setChannelName(3, "arco");
        sp->appendElement(ar, QT_TRANSLATE_NOOP("palette", "Arco"))->setElementTranslated(true);

        auto tm = makeElement<StaffText>(gscore);
        tm->setTid(Tid::EXPRESSION);
        tm->setXmlText(QT_TRANSLATE_NOOP("palette", "tremolo"));
        tm->setChannelName(0, "tremolo");
        tm->setChannelName(1, "tremolo");
        tm->setChannelName(2, "tremolo");
        tm->setChannelName(3, "tremolo");
        sp->appendElement(tm, QT_TRANSLATE_NOOP("palette", "Tremolo"))->setElementTranslated(true);

        auto mu = makeElement<StaffText>(gscore);
        /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
        mu->setXmlText(QT_TRANSLATE_NOOP("palette", "mute"));
        mu->setChannelName(0, "mute");
        mu->setChannelName(1, "mute");
        mu->setChannelName(2, "mute");
        mu->setChannelName(3, "mute");
        /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
        sp->appendElement(mu, QT_TRANSLATE_NOOP("palette", "Mute"))->setElementTranslated(true);

        auto no = makeElement<StaffText>(gscore);
        /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
        no->setXmlText(QT_TRANSLATE_NOOP("palette", "open"));
        no->setChannelName(0, "open");
        no->setChannelName(1, "open");
        no->setChannelName(2, "open");
        no->setChannelName(3, "open");
        /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
        sp->appendElement(no, QT_TRANSLATE_NOOP("palette", "Open"))->setElementTranslated(true);

        auto sa = makeElement<StaffText>(gscore);
        sa->setXmlText(QT_TRANSLATE_NOOP("palette", "S/A"));
        sa->setChannelName(0, "Soprano");
        sa->setChannelName(1, "Alto");
        sa->setChannelName(2, "Soprano");
        sa->setChannelName(3, "Alto");
        sa->setVisible(false);
        sp->appendElement(sa, QT_TRANSLATE_NOOP("palette", "Soprano/Alto"))->setElementTranslated(true);

        auto tb = makeElement<StaffText>(gscore);
        tb->setXmlText(QT_TRANSLATE_NOOP("palette", "T/B"));
        tb->setChannelName(0, "Tenor");
        tb->setChannelName(1, "Bass");
        tb->setChannelName(2, "Tenor");
        tb->setChannelName(3, "Bass");
        tb->setVisible(false);
        sp->appendElement(tb, QT_TRANSLATE_NOOP("palette", "Tenor/Bass"))->setElementTranslated(true);

        auto tl = makeElement<StaffText>(gscore);
        tl->setXmlText(QT_TRANSLATE_NOOP("palette", "T/L"));
        tl->setChannelName(0, "TENOR");
        tl->setChannelName(1, "LEAD");
        tl->setChannelName(2, "TENOR");
        tl->setChannelName(3, "LEAD");
        tl->setVisible(false);
        sp->appendElement(tl, QT_TRANSLATE_NOOP("palette", "Tenor/Lead"))->setElementTranslated(true);

        auto bb = makeElement<StaffText>(gscore);
        bb->setXmlText(QT_TRANSLATE_NOOP("palette", "B/B"));
        bb->setChannelName(0, "BARI");
        bb->setChannelName(1, "BASS");
        bb->setChannelName(2, "BARI");
        bb->setChannelName(3, "BASS");
        bb->setVisible(false);
        sp->appendElement(bb, QT_TRANSLATE_NOOP("palette", "Bari/Bass"))->setElementTranslated(true);
    }

    return sp;
}

PalettePanelPtr PaletteCreator::newTimePalettePanel()
{
    struct TS {
        int numerator;
        int denominator;
        TimeSigType type;
        QString name;
    };

    TS tsList[] = {
        { 2,  4, TimeSigType::NORMAL, "2/4" },
        { 3,  4, TimeSigType::NORMAL, "3/4" },
        { 4,  4, TimeSigType::NORMAL, "4/4" },
        { 5,  4, TimeSigType::NORMAL, "5/4" },
        { 6,  4, TimeSigType::NORMAL, "6/4" },
        { 3,  8, TimeSigType::NORMAL, "3/8" },
        { 6,  8, TimeSigType::NORMAL, "6/8" },
        { 7,  8, TimeSigType::NORMAL, "7/8" },
        { 9,  8, TimeSigType::NORMAL, "9/8" },
        { 12, 8, TimeSigType::NORMAL, "12/8" },
        { 4,  4, TimeSigType::FOUR_FOUR,  QT_TRANSLATE_NOOP("symUserNames", "Common time") },
        { 2,  2, TimeSigType::ALLA_BREVE, QT_TRANSLATE_NOOP("symUserNames", "Cut time") },
        { 2,  2, TimeSigType::NORMAL, "2/2" },
        { 3,  2, TimeSigType::NORMAL, "3/2" },
        { 4,  2, TimeSigType::NORMAL, "4/2" },
        { 2,  2, TimeSigType::CUT_BACH, QT_TRANSLATE_NOOP("symUserNames", "Cut time (Bach)") },
        { 9,  8, TimeSigType::CUT_TRIPLE, QT_TRANSLATE_NOOP("symUserNames", "Cut triple time (9/8)") },
    };

    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::TimeSig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Time Signatures"));
    sp->setMag(.8);
    sp->setGrid(42, 38);

    for (unsigned i = 0; i < sizeof(tsList) / sizeof(*tsList); ++i) {
        auto ts = makeElement<TimeSig>(gscore);
        ts->setSig(Fraction(tsList[i].numerator, tsList[i].denominator), tsList[i].type);
        sp->appendElement(ts, tsList[i].name);
    }
    return sp;
}

PalettePanelPtr PaletteCreator::newFretboardDiagramPalettePanel()
{
    PalettePanelPtr sp = std::make_shared<PalettePanel>(PalettePanel::Type::FretboardDiagram);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fretboard Diagrams"));
    sp->setGrid(42, 45);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto fret = FretDiagram::createFromString(gscore, "X32O1O");
    fret->setHarmony("C");
    sp->appendElement(fret, "C");
    fret = FretDiagram::createFromString(gscore, "X-554-");
    fret->setHarmony("Cm");
    sp->appendElement(fret, "Cm");
    fret = FretDiagram::createFromString(gscore, "X3231O");
    fret->setHarmony("C7");
    sp->appendElement(fret, "C7");

    fret = FretDiagram::createFromString(gscore, "XXO232");
    fret->setHarmony("D");
    sp->appendElement(fret, "D");
    fret = FretDiagram::createFromString(gscore, "XXO231");
    fret->setHarmony("Dm");
    sp->appendElement(fret, "Dm");
    fret = FretDiagram::createFromString(gscore, "XXO212");
    fret->setHarmony("D7");
    sp->appendElement(fret, "D7");

    fret = FretDiagram::createFromString(gscore, "O221OO");
    fret->setHarmony("E");
    sp->appendElement(fret, "E");
    fret = FretDiagram::createFromString(gscore, "O22OOO");
    fret->setHarmony("Em");
    sp->appendElement(fret, "Em");
    fret = FretDiagram::createFromString(gscore, "O2O1OO");
    fret->setHarmony("E7");
    sp->appendElement(fret, "E7");

    fret = FretDiagram::createFromString(gscore, "-332--");
    fret->setHarmony("F");
    sp->appendElement(fret, "F");
    fret = FretDiagram::createFromString(gscore, "-33---");
    fret->setHarmony("Fm");
    sp->appendElement(fret, "Fm");
    fret = FretDiagram::createFromString(gscore, "-3-2--");
    fret->setHarmony("F7");
    sp->appendElement(fret, "F7");

    fret = FretDiagram::createFromString(gscore, "32OOO3");
    fret->setHarmony("G");
    sp->appendElement(fret, "G");
    fret = FretDiagram::createFromString(gscore, "-55---");
    fret->setHarmony("Gm");
    sp->appendElement(fret, "Gm");
    fret = FretDiagram::createFromString(gscore, "32OOO1");
    fret->setHarmony("G7");
    sp->appendElement(fret, "G7");

    fret = FretDiagram::createFromString(gscore, "XO222O");
    fret->setHarmony("A");
    sp->appendElement(fret, "A");
    fret = FretDiagram::createFromString(gscore, "XO221O");
    fret->setHarmony("Am");
    sp->appendElement(fret, "Am");
    fret = FretDiagram::createFromString(gscore, "XO2O2O");
    fret->setHarmony("A7");
    sp->appendElement(fret, "A7");

    fret = FretDiagram::createFromString(gscore, "X-444-");
    fret->setHarmony("B");
    sp->appendElement(fret, "B");
    fret = FretDiagram::createFromString(gscore, "X-443-");
    fret->setHarmony("Bm");
    sp->appendElement(fret, "Bm");
    fret = FretDiagram::createFromString(gscore, "X212O2");
    fret->setHarmony("B7");
    sp->appendElement(fret, "B7");

    return sp;
}
