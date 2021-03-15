//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

// For menus in the menu bar, like File, Edit, and View, see mscore/musescore.cpp

#include "palettecreator.h"

#include <QAction>

#include "actions/actiontypes.h"

#include "libmscore/score.h"
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
#include "libmscore/icon.h"
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

using namespace mu::actions;

namespace Ms {
extern bool useFactorySettings;

static Palette* toPalette(PalettePanel* pp)
{
    return new Palette(PalettePanelPtr(pp));
}

//---------------------------------------------------------
//   populateIconPalette
//---------------------------------------------------------

void populateIconPalette(Palette* p, const IconAction* a)
{
    auto adapter = mu::framework::ioc()->resolve<mu::palette::IPaletteAdapter>("palette");
    while (a->subtype != IconType::NONE) {
        auto ik = makeElement<Icon>(Ms::gscore);
        ik->setIconType(a->subtype);
        ActionItem action = adapter->getAction(codeFromQString(a->action));
        ik->setAction(a->action, static_cast<char16_t>(action.iconCode));
        p->append(ik, QString::fromStdString(action.title));
        ++a;
    }
}

//---------------------------------------------------------
//   newBeamPalette
//---------------------------------------------------------

Palette* PaletteCreator::newBeamPalette()
{
    return toPalette(newBeamPalettePanel());
}

//---------------------------------------------------------
//   newDynamicsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newDynamicsPalette(bool defaultPalette)
{
    return toPalette(newDynamicsPalettePanel(defaultPalette));
}

//---------------------------------------------------------
//   newKeySigPalette
//---------------------------------------------------------

Palette* PaletteCreator::newKeySigPalette()
{
    return toPalette(newKeySigPalettePanel());
}

//---------------------------------------------------------
//   newAccidentalsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newAccidentalsPalette(bool defaultPalette)
{
    return toPalette(newAccidentalsPalettePanel(defaultPalette));
}

//---------------------------------------------------------
//   newBarLinePalette
//---------------------------------------------------------

Palette* PaletteCreator::newBarLinePalette()
{
    return toPalette(newBarLinePalettePanel());
}

Palette* PaletteCreator::newLayoutPalette()
{
    return toPalette(newLayoutPalettePanel());
}

//---------------------------------------------------------
//   newRepeatsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newRepeatsPalette()
{
    return toPalette(newRepeatsPalettePanel());
}

//---------------------------------------------------------
//   newFingeringPalette
//---------------------------------------------------------

Palette* PaletteCreator::newFingeringPalette()
{
    return toPalette(newFingeringPalettePanel());
}

//---------------------------------------------------------
//   newTremoloPalette
//---------------------------------------------------------

Palette* PaletteCreator::newTremoloPalette()
{
    return toPalette(newTremoloPalettePanel());
}

//---------------------------------------------------------
//   newNoteHeadsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newNoteHeadsPalette()
{
    return toPalette(newNoteHeadsPalettePanel());
}

//---------------------------------------------------------
//   newArticulationsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newArticulationsPalette()
{
    return toPalette(newArticulationsPalettePanel());
}

//---------------------------------------------------------
//   newOrnamentsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newOrnamentsPalette()
{
    return toPalette(newOrnamentsPalettePanel());
}

//---------------------------------------------------------
//   newAccordionPalette
//---------------------------------------------------------

Palette* PaletteCreator::newAccordionPalette()
{
    return toPalette(newAccordionPalettePanel());
}

//---------------------------------------------------------
//   newBracketsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newBracketsPalette()
{
    return toPalette(newBracketsPalettePanel());
}

//---------------------------------------------------------
//   newBreathPalette
//---------------------------------------------------------

Palette* PaletteCreator::newBreathPalette()
{
    return toPalette(newBreathPalettePanel());
}

//---------------------------------------------------------
//   newArpeggioPalette
//---------------------------------------------------------

Palette* PaletteCreator::newArpeggioPalette()
{
    return toPalette(newArpeggioPalettePanel());
}

//---------------------------------------------------------
//   newClefsPalette
//---------------------------------------------------------

Palette* PaletteCreator::newClefsPalette(bool defaultPalette)
{
    return toPalette(newClefsPalettePanel(defaultPalette));
}

//---------------------------------------------------------
//   newGraceNotePalette
//---------------------------------------------------------

Palette* PaletteCreator::newGraceNotePalette()
{
    return toPalette(newGraceNotePalettePanel());
}

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalette
//---------------------------------------------------------

Palette* PaletteCreator::newBagpipeEmbellishmentPalette()
{
    return toPalette(newBagpipeEmbellishmentPalettePanel());
}

//---------------------------------------------------------
//   newLinesPalette
//---------------------------------------------------------

Palette* PaletteCreator::newLinesPalette()
{
    return toPalette(newLinesPalettePanel());
}

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

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

//---------------------------------------------------------
//   newTempoPalette
//---------------------------------------------------------

Palette* PaletteCreator::newTempoPalette(bool defaultPalette)
{
    return toPalette(newTempoPalettePanel(defaultPalette));
}

//---------------------------------------------------------
//   newTextPalette
//---------------------------------------------------------

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

//---------------------------------------------------------
//    newFretboardDiagramPalette
//---------------------------------------------------------

Palette* PaletteCreator::newFretboardDiagramPalette()
{
    return toPalette(newFretboardDiagramPalettePanel());
}

//---------------------------------------------------------
//   newMasterPaletteTree
//---------------------------------------------------------

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

PaletteTree* PaletteCreator::newDefaultPaletteTree()
{
    PaletteTree* defaultPalette = new PaletteTree();

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

//---------------------------------------------------------
//   populateIconPalettePanel
//---------------------------------------------------------

static void populateIconPalettePanel(PalettePanel* p, const IconAction* a)
{
    auto adapter = mu::framework::ioc()->resolve<mu::palette::IPaletteAdapter>("palette");
    while (a->subtype != IconType::NONE) {
        auto ik = makeElement<Icon>(gscore);
        ik->setIconType(a->subtype);
        ActionItem action = adapter->getAction(codeFromQString(a->action));
        ik->setAction(a->action, static_cast<char16_t>(action.iconCode));
        p->append(ik, QString::fromStdString(action.title));
        ++a;
    }
}

//---------------------------------------------------------
//   newBeamPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newBeamPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Beam);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Beam Properties"));
    sp->setGrid(27, 40);
    sp->setDrawGrid(true);

    const IconAction bpa[] = {
        { IconType::SBEAM,    "beam-start" },
        { IconType::MBEAM,    "beam-mid" },
        { IconType::NBEAM,    "no-beam" },
        { IconType::BEAM32,   "beam32" },
        { IconType::BEAM64,   "beam64" },
        { IconType::AUTOBEAM, "auto-beam" },
        { IconType::FBEAM1,   "fbeam1" },
        { IconType::FBEAM2,   "fbeam2" },
        { IconType::NONE,     "" }
    };

    populateIconPalettePanel(sp, bpa);
    return sp;
}

//---------------------------------------------------------
//   newDynamicsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newDynamicsPalettePanel(bool defaultPalettePanel)
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Dynamic);
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
        sp->setMoreElements(true);
    } else {
        array = &array1;
        sp->setGrid(60, 28);
    }

    for (const char* c :  *array) {
        auto dynamic = makeElement<Dynamic>(Ms::gscore);
        dynamic->setDynamicType(c);
        sp->append(dynamic, dynamic->dynamicTypeName());
    }
    return sp;
}

//---------------------------------------------------------
//   newKeySigPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newKeySigPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::KeySig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Key Signatures"));
    sp->setMag(1.0);
    sp->setGrid(56, 55);
    sp->setYOffset(1.0);

    for (int i = 0; i < 7; ++i) {
        auto k = makeElement<KeySig>(Ms::gscore);
        k->setKey(Key(i + 1));
        sp->append(k, keyNames[i * 2]);
    }
    for (int i = -7; i < 0; ++i) {
        auto k = makeElement<KeySig>(gscore);
        k->setKey(Key(i));
        sp->append(k, keyNames[(7 + i) * 2 + 1]);
    }
    auto k = makeElement<KeySig>(gscore);
    k->setKey(Key::C);
    sp->append(k, keyNames[14]);

    // atonal key signature
    KeySigEvent nke;
    nke.setKey(Key::C);
    nke.setCustom(true);
    nke.setMode(KeyMode::NONE);
    auto nk = makeElement<KeySig>(gscore);
    nk->setKeySigEvent(nke);
    sp->append(nk, keyNames[15]);

    return sp;
}

//---------------------------------------------------------
//   newAccidentalsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newAccidentalsPalettePanel(bool defaultPalettePanel)
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Accidental);
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
            sp->append(ac, ac->subtypeUserName());
        }
    }

    if (defaultPalettePanel) {
        sp->setMoreElements(true);
    }

    auto ik = makeElement<Icon>(gscore);
    ik->setIconType(IconType::BRACKETS);
    ActionItem action = adapter()->getAction("add-brackets");
    ik->setAction(action.code, static_cast<char16_t>(action.iconCode));
    sp->append(ik, QString::fromStdString(action.title));

    ik = makeElement<Icon>(gscore);
    ik->setIconType(IconType::PARENTHESES);
    action = adapter()->getAction("add-parentheses");
    ik->setAction(action.code, static_cast<char16_t>(action.iconCode));
    sp->append(ik, QString::fromStdString(action.title));

    ik = makeElement<Icon>(gscore);
    ik->setIconType(IconType::BRACES);
    action = adapter()->getAction("add-braces");
    ik->setAction(action.code, static_cast<char16_t>(action.iconCode));
    sp->append(ik, QString::fromStdString(action.title));

    return sp;
}

//---------------------------------------------------------
//   newBarLinePalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newBarLinePalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::BarLine);
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
        sp->append(b, BarLine::userTypeName(bti->type));
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
        sp->append(b, span.userName);
    }
    return sp;
}

//---------------------------------------------------------
//   newRepeatsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newRepeatsPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Repeat);
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
        sp->append(rm, mu::qtrc("symUserNames", Sym::symUserNames[int(repeat.id)]));
    }

    for (int i = 0; i < markerTypeTableSize(); i++) {
        if (markerTypeTable[i].type == Marker::Type::CODETTA) { // not in SMuFL
            continue;
        }

        auto mk = makeElement<Marker>(gscore);
        mk->setMarkerType(markerTypeTable[i].type);
        mk->styleChanged();
        sp->append(mk, markerTypeTable[i].name);
    }

    for (int i = 0; i < jumpTypeTableSize(); i++) {
        auto jp = makeElement<Jump>(gscore);
        jp->setJumpType(jumpTypeTable[i].type);
        sp->append(jp, jumpTypeTable[i].userText);
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
        PaletteCellPtr cell = sp->append(b, BarLine::userTypeName(bti->type));
        cell->drawStaff = false;
    }

    return sp;
}

PalettePanel* PaletteCreator::newLayoutPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Layout);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Layout"));
    sp->setGrid(42, 36);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::LINE);
    PaletteCellPtr cell = sp->append(lb, QT_TRANSLATE_NOOP("palette", "System break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
    cell = sp->append(lb, QT_TRANSLATE_NOOP("palette", "Page break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
    cell = sp->append(lb, QT_TRANSLATE_NOOP("palette", "Section break"));
    cell->mag = 1.2;

    lb = makeElement<LayoutBreak>(gscore);
    lb->setLayoutBreakType(LayoutBreak::Type::NOBREAK);
    cell = sp->append(lb, QT_TRANSLATE_NOOP("Palette", "Group measures"));
    cell->mag = 1.2;

    qreal _spatium = gscore->spatium();
    auto spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::DOWN);
    spacer->setGap(3 * _spatium);
    cell = sp->append(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer down"));
    cell->mag = .7;

    spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::UP);
    spacer->setGap(3 * _spatium);
    cell = sp->append(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer up"));
    cell->mag = .7;

    spacer = makeElement<Spacer>(gscore);
    spacer->setSpacerType(SpacerType::FIXED);
    spacer->setGap(3 * _spatium);
    cell = sp->append(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer fixed down"));
    cell->mag = .7;

    auto stc = makeElement<StaffTypeChange>(gscore);
    sp->append(stc, QT_TRANSLATE_NOOP("palette", "Staff type change"));

    if (configuration()->enableExperimental()) {
        static const IconAction bpa[] = {
            { IconType::VFRAME,   "insert-vbox" },
            { IconType::HFRAME,   "insert-hbox" },
            { IconType::TFRAME,   "insert-textframe" },
            { IconType::FFRAME,   "insert-fretframe" },
            { IconType::MEASURE,  "insert-measure" },
            { IconType::NONE,     "" }
        };
        populateIconPalettePanel(sp, bpa);
    } else {
        static const IconAction bpa[] = {
            { IconType::VFRAME,   "insert-vbox" },
            { IconType::HFRAME,   "insert-hbox" },
            { IconType::TFRAME,   "insert-textframe" },
            { IconType::MEASURE,  "insert-measure" },
            { IconType::NONE,     "" }
        };
        populateIconPalettePanel(sp, bpa);
    }

    return sp;
}

//---------------------------------------------------------
//   newFingeringPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newFingeringPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Fingering);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fingering"));
    sp->setMag(1.5);
    sp->setGrid(28, 30);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    const char* finger = "012345";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setXmlText(QString(finger[i]));
        sp->append(f, QT_TRANSLATE_NOOP("palette", "Fingering %1"));
    }
    finger = "pimac";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::RH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->append(f, QT_TRANSLATE_NOOP("palette", "RH Guitar Fingering %1"));
    }
    finger = "012345T";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::LH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->append(f, QT_TRANSLATE_NOOP("palette", "LH Guitar Fingering %1"));
    }
    finger = "0123456";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gscore);
        f->setTid(Tid::STRING_NUMBER);
        f->setXmlText(QString(finger[i]));
        sp->append(f, QT_TRANSLATE_NOOP("palette", "String number %1"));
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
        sp->append(s, s->userName());
    }
    return sp;
}

//---------------------------------------------------------
//   newTremoloPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newTremoloPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Tremolo);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Tremolos"));
    sp->setGrid(27, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
        auto tremolo = makeElement<Tremolo>(gscore);
        tremolo->setTremoloType(TremoloType(i));
        sp->append(tremolo, tremolo->subtypeName());
    }
    return sp;
}

//---------------------------------------------------------
//   newNoteHeadsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newNoteHeadsPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::NoteHead);
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
        sp->append(nh, NoteHead::group2userName(NoteHead::Group(i)));
    }

    auto ik = makeElement<Icon>(gscore);
    ik->setIconType(IconType::PARENTHESES);
    ActionItem action = adapter()->getAction("add-parentheses");
    ik->setAction("add-parentheses", static_cast<char16_t>(action.iconCode));
    sp->append(ik, QString::fromStdString(action.title));

    return sp;
}

//---------------------------------------------------------
//   newArticulationsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newArticulationsPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Articulation);
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
        sp->append(s, s->userName());
    }
    auto bend = makeElement<Bend>(gscore);
    bend->points().append(PitchValue(0,    0, false));
    bend->points().append(PitchValue(15, 100, false));
    bend->points().append(PitchValue(60, 100, false));
    sp->append(bend, QT_TRANSLATE_NOOP("palette", "Bend"));

    auto tb = makeElement<TremoloBar>(gscore);
    tb->points().append(PitchValue(0,     0, false));       // "Dip"
    tb->points().append(PitchValue(30, -100, false));
    tb->points().append(PitchValue(60,    0, false));
    sp->append(tb, QT_TRANSLATE_NOOP("palette", "Tremolo bar"));

    return sp;
}

//---------------------------------------------------------
//   newOrnamentsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newOrnamentsPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Ornament);
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
        sp->append(s, s->userName());
    }
    return sp;
}

//---------------------------------------------------------
//   newAccordionPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newAccordionPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Accordion);
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
        sp->append(s, Sym::id2userName(i));
    }
    return sp;
}

//---------------------------------------------------------
//   newBracketsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newBracketsPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Bracket);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Brackets"));
    sp->setMag(0.7);
    sp->setGrid(40, 60);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    std::array<std::pair<BracketType,const char*>, 4> types {
        { { BracketType::NORMAL, QT_TRANSLATE_NOOP("palette", "Bracket") },
            { BracketType::BRACE,  QT_TRANSLATE_NOOP("palette", "Brace") },
            { BracketType::SQUARE, QT_TRANSLATE_NOOP("palette", "Square") },
            { BracketType::LINE,   QT_TRANSLATE_NOOP("palette", "Line") } }
    };

    for (auto type : types) {
        auto b1 = makeElement<Bracket>(gscore);
        auto bi1 = makeElement<BracketItem>(gscore);
        bi1->setBracketType(type.first);
        b1->setBracketItem(bi1.get());
        sp->append(b1, type.second); // Bracket, Brace, Square, Line
    }
    return sp;
}

//---------------------------------------------------------
//   newBreathPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newBreathPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Breath);
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
        sp->append(f, f->userName());
    }

    for (BreathType bt : Breath::breathList) {
        auto a = makeElement<Breath>(gscore);
        a->setSymId(bt.id);
        a->setPause(bt.pause);
        sp->append(a, Sym::id2userName(bt.id));
    }

    return sp;
}

//---------------------------------------------------------
//   newArpeggioPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newArpeggioPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Arpeggio);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Arpeggios & Glissandi"));
    sp->setGrid(27, 50);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = 0; i < 6; ++i) {
        auto a = makeElement<Arpeggio>(gscore);
        a->setArpeggioType(ArpeggioType(i));
        sp->append(a, a->arpeggioTypeName());
    }
    for (int i = 0; i < 2; ++i) {
        auto a = makeElement<Glissando>(gscore);
        a->setGlissandoType(GlissandoType(i));
        sp->append(a, a->glissandoTypeName());
    }

    //fall and doits

    auto cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::FALL);
    sp->append(cl, scorelineNames[0]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::DOIT);
    sp->append(cl, scorelineNames[1]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::PLOP);
    sp->append(cl, scorelineNames[2]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::SCOOP);
    sp->append(cl, scorelineNames[3]);

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::FALL);
    cl->setStraight(true);
    sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide out down"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::DOIT);
    cl->setStraight(true);
    sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide out up"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::PLOP);
    cl->setStraight(true);
    sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide in above"));

    cl = makeElement<ChordLine>(gscore);
    cl->setChordLineType(ChordLineType::SCOOP);
    cl->setStraight(true);
    sp->append(cl, QT_TRANSLATE_NOOP("Ms", "Slide in below"));

    return sp;
}

//---------------------------------------------------------
//   newClefsPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newClefsPalettePanel(bool defaultPalettePanel)
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Clef);
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
        sp->setMoreElements(true);
    } else {
        items = &clefsMaster;
        sp->setMoreElements(false);
    }

    for (ClefType j : *items) {
        auto k = makeElement<Ms::Clef>(gscore);
        k->setClefType(ClefTypeList(j, j));
        sp->append(k, ClefInfo::name(j));
    }
    return sp;
}

//---------------------------------------------------------
//   newGraceNotePalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newGraceNotePalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::GraceNote);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Grace Notes"));
    sp->setGrid(32, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    static const IconAction gna[] = {
        { IconType::ACCIACCATURA,  "acciaccatura" },
        { IconType::APPOGGIATURA,  "appoggiatura" },
        { IconType::GRACE4,        "grace4" },
        { IconType::GRACE16,       "grace16" },
        { IconType::GRACE32,       "grace32" },
        { IconType::GRACE8_AFTER,  "grace8after" },
        { IconType::GRACE16_AFTER, "grace16after" },
        { IconType::GRACE32_AFTER, "grace32after" },
        { IconType::NONE,          "" }
    };
    populateIconPalettePanel(sp, gna);
    return sp;
}

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newBagpipeEmbellishmentPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::BagpipeEmbellishment);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Bagpipe Embellishments"));
    sp->setMag(0.8);
    sp->setYOffset(2.0);
    sp->setGrid(55, 55);
    sp->setVisible(false);

    for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
        auto b = makeElement<BagpipeEmbellishment>(gscore);
        b->setEmbelType(i);
        sp->append(b, BagpipeEmbellishment::BagpipeEmbellishmentList[i].name);
    }

    return sp;
}

//---------------------------------------------------------
//   newLinesPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newLinesPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Line);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Lines"));
    sp->setMag(.8);
    sp->setGrid(75, 28);
    sp->setDrawGrid(true);

    qreal w = gscore->spatium() * 8;

    auto slur = makeElement<Slur>(gscore);
    sp->append(slur, QT_TRANSLATE_NOOP("palette", "Slur"));

    auto gabel0 = makeElement<Hairpin>(gscore);
    gabel0->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel0->setLen(w);
    sp->append(gabel0, QT_TRANSLATE_NOOP("palette", "Crescendo hairpin"));

    auto gabel1 = makeElement<Hairpin>(gscore);
    gabel1->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    gabel1->setLen(w);
    sp->append(gabel1, QT_TRANSLATE_NOOP("palette", "Diminuendo hairpin"));

    auto gabel2 = makeElement<Hairpin>(gscore);
    gabel2->setHairpinType(HairpinType::CRESC_LINE);
    gabel2->setLen(w);
    sp->append(gabel2, QT_TRANSLATE_NOOP("palette", "Crescendo line"));

    auto gabel3 = makeElement<Hairpin>(gscore);
    gabel3->setHairpinType(HairpinType::DECRESC_LINE);
    gabel3->setLen(w);
    sp->append(gabel3, QT_TRANSLATE_NOOP("palette", "Diminuendo line"));

    auto gabel4 = makeElement<Hairpin>(gscore);
    gabel4->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel4->setBeginText("<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    gabel4->setBeginTextAlign(Align::VCENTER);
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, PropertyFlags::UNSTYLED);
    gabel4->setLen(w);
    sp->append(gabel4, QT_TRANSLATE_NOOP("palette", "Dynamic + hairpin"));

    auto volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("1.");
    QList<int> il;
    il.append(1);
    volta->setEndings(il);
    sp->append(volta, QT_TRANSLATE_NOOP("palette", "Prima volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("2.");
    il.clear();
    il.append(2);
    volta->setEndings(il);
    sp->append(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("3.");
    il.clear();
    il.append(3);
    volta->setEndings(il);
    sp->append(volta, QT_TRANSLATE_NOOP("palette", "Terza volta"));

    volta = makeElement<Volta>(gscore);
    volta->setVoltaType(Volta::Type::OPEN);
    volta->setLen(w);
    volta->setText("2.");
    il.clear();
    il.append(2);
    volta->setEndings(il);
    sp->append(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta, open"));

    auto ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "8va alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VB);
    ottava->setLen(w);
    ottava->setPlacement(Placement::BELOW);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "8va bassa"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_15MA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "15ma alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_15MB);
    ottava->setLen(w);
    ottava->setPlacement(Placement::BELOW);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "15ma bassa"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_22MA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "22ma alta"));

    ottava = makeElement<Ottava>(gscore);
    ottava->setOttavaType(OttavaType::OTTAVA_22MB);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->append(ottava, QT_TRANSLATE_NOOP("palette", "22ma bassa"));

    auto pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginText("<sym>keyboardPedalPed</sym>");
    pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
    pedal->setEndHookType(HookType::HOOK_90);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and line)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginText("<sym>keyboardPedalPed</sym>");
    pedal->setContinueText("(<sym>keyboardPedalPed</sym>)");
    pedal->setEndText("<sym>keyboardPedalUp</sym>");
    pedal->setLineVisible(false);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and asterisk)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_90);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (straight hooks)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_45);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled end hook)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_45);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (both hooks angled)"));

    pedal = makeElement<Pedal>(gscore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_90);
    sp->append(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled start hook)"));

    for (int i = 0; i < trillTableSize(); i++) {
        auto trill = makeElement<Trill>(gscore);
        trill->setTrillType(trillTable[i].type);
        trill->setLen(w);
        sp->append(trill, trillTable[i].userName);
    }

    auto textLine = makeElement<TextLine>(gscore);
    textLine->setLen(w);
    textLine->setBeginText("VII");
    textLine->setEndHookType(HookType::HOOK_90);
    sp->append(textLine, QT_TRANSLATE_NOOP("palette", "Text line"));

    auto line = makeElement<TextLine>(gscore);
    line->setLen(w);
    line->setDiagonal(true);
    sp->append(line, QT_TRANSLATE_NOOP("palette", "Line"));

    auto a = makeElement<Ambitus>(gscore);
    sp->append(a, QT_TRANSLATE_NOOP("palette", "Ambitus"));

    auto letRing = makeElement<LetRing>(gscore);
    letRing->setLen(w);
    sp->append(letRing, QT_TRANSLATE_NOOP("palette", "Let Ring"));

    for (int i = 0; i < vibratoTableSize(); i++) {
        auto vibrato = makeElement<Vibrato>(gscore);
        vibrato->setVibratoType(vibratoTable[i].type);
        vibrato->setLen(w);
        sp->append(vibrato, vibratoTable[i].userName);
    }

    auto pm = makeElement<PalmMute>(gscore);
    pm->setLen(w);
    sp->append(pm, QT_TRANSLATE_NOOP("palette", "Palm Mute"));

    return sp;
}

//---------------------------------------------------------
//   newTempoPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newTempoPalettePanel(bool defaultPalettePanel)
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Tempo);
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
    sp->append(stxt, QT_TRANSLATE_NOOP("palette", "Swing"))->setElementTranslated(true);

    for (TempoPattern tp : tps) {
        auto tt = makeElement<TempoText>(gscore);
        tt->setFollowText(tp.followText);
        tt->setXmlText(tp.pattern);
        if (tp.relative) {
            tt->setRelative(tp.f);
            sp->append(tt, mu::qtrc("palette", tp.name), 1.5);
        } else if (tp.italian) {
            tt->setTempo(tp.f);
            sp->append(tt, tp.name, 1.3);
        } else {
            tt->setTempo(tp.f);
            sp->append(tt, mu::qtrc("palette", tp.name), 1.5);
        }
    }
    sp->setMoreElements(false);

    return sp;
}

//---------------------------------------------------------
//   newTextPalettePanel
//---------------------------------------------------------

PalettePanel* PaletteCreator::newTextPalettePanel(bool defaultPalettePanel)
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::Text);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Text"));
    sp->setMag(0.85);
    sp->setGrid(84, 28);
    sp->setDrawGrid(true);

    auto st = makeElement<StaffText>(gscore);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Staff Text"));
    sp->append(st, QT_TRANSLATE_NOOP("palette", "Staff text"))->setElementTranslated(true);

    st = makeElement<StaffText>(gscore);
    st->setTid(Tid::EXPRESSION);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Expression"));
    st->setPlacement(Placement::BELOW);
    st->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    sp->append(st, QT_TRANSLATE_NOOP("palette", "Expression text"))->setElementTranslated(true);

    auto is = makeElement<InstrumentChange>(gscore);
    is->setXmlText(QT_TRANSLATE_NOOP("palette", "Change Instr."));
    sp->append(is, QT_TRANSLATE_NOOP("palette", "Instrument change"))->setElementTranslated(true);

    auto rhm = makeElement<RehearsalMark>(gscore);
    rhm->setXmlText("B1");
    sp->append(rhm, QT_TRANSLATE_NOOP("palette", "Rehearsal mark"));

    auto stxt = makeElement<SystemText>(gscore);
    stxt->setTid(Tid::TEMPO);
    /*: System text to switch from swing rhythm back to straight rhythm */
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Straight"));
    // need to be true to enable the "Off" option
    stxt->setSwing(true);
    // 0 (swingUnit) turns of swing; swingRatio is set to default
    stxt->setSwingParameters(0, stxt->score()->styleI(Sid::swingRatio));
    /*: System text to switch from swing rhythm back to straight rhythm */
    sp->append(stxt, QT_TRANSLATE_NOOP("palette", "Straight"))->setElementTranslated(true);

    stxt = makeElement<SystemText>(gscore);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "System Text"));
    sp->append(stxt, QT_TRANSLATE_NOOP("palette", "System text"))->setElementTranslated(true);

    // Measure numbers, unlike other elements (but like most text elements),
    // are not copied directly into the score when drop.
    // Instead, they simply set the corresponding measure's MeasureNumberMode to SHOW
    // Because of that, the element shown in the palettes does not have to have any particular formatting.
    auto meaNum = makeElement<MeasureNumber>(gscore);
    meaNum->setProperty(Pid::SUB_STYLE, int(Tid::STAFF));   // Make the element bigger in the palettes (using the default measure number style makes it too small)
    meaNum->setXmlText(QT_TRANSLATE_NOOP("palette", "Measure Number"));
    sp->append(meaNum, QT_TRANSLATE_NOOP("palette", "Measure Number"))->setElementTranslated(true);

    if (!defaultPalettePanel) {
        auto pz = makeElement<StaffText>(gscore);
        pz->setXmlText(QT_TRANSLATE_NOOP("palette", "pizz."));
        pz->setChannelName(0, "pizzicato");
        pz->setChannelName(1, "pizzicato");
        pz->setChannelName(2, "pizzicato");
        pz->setChannelName(3, "pizzicato");
        sp->append(pz, QT_TRANSLATE_NOOP("palette", "Pizzicato"))->setElementTranslated(true);

        auto ar = makeElement<StaffText>(gscore);
        ar->setXmlText(QT_TRANSLATE_NOOP("palette", "arco"));
        ar->setChannelName(0, "arco");
        ar->setChannelName(1, "arco");
        ar->setChannelName(2, "arco");
        ar->setChannelName(3, "arco");
        sp->append(ar, QT_TRANSLATE_NOOP("palette", "Arco"))->setElementTranslated(true);

        auto tm = makeElement<StaffText>(gscore);
        tm->setTid(Tid::EXPRESSION);
        tm->setXmlText(QT_TRANSLATE_NOOP("palette", "tremolo"));
        tm->setChannelName(0, "tremolo");
        tm->setChannelName(1, "tremolo");
        tm->setChannelName(2, "tremolo");
        tm->setChannelName(3, "tremolo");
        sp->append(tm, QT_TRANSLATE_NOOP("palette", "Tremolo"))->setElementTranslated(true);

        auto mu = makeElement<StaffText>(gscore);
        /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
        mu->setXmlText(QT_TRANSLATE_NOOP("palette", "mute"));
        mu->setChannelName(0, "mute");
        mu->setChannelName(1, "mute");
        mu->setChannelName(2, "mute");
        mu->setChannelName(3, "mute");
        /*: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music) */
        sp->append(mu, QT_TRANSLATE_NOOP("palette", "Mute"))->setElementTranslated(true);

        auto no = makeElement<StaffText>(gscore);
        /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
        no->setXmlText(QT_TRANSLATE_NOOP("palette", "open"));
        no->setChannelName(0, "open");
        no->setChannelName(1, "open");
        no->setChannelName(2, "open");
        no->setChannelName(3, "open");
        /*: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music) */
        sp->append(no, QT_TRANSLATE_NOOP("palette", "Open"))->setElementTranslated(true);

        auto sa = makeElement<StaffText>(gscore);
        sa->setXmlText(QT_TRANSLATE_NOOP("palette", "S/A"));
        sa->setChannelName(0, "Soprano");
        sa->setChannelName(1, "Alto");
        sa->setChannelName(2, "Soprano");
        sa->setChannelName(3, "Alto");
        sa->setVisible(false);
        sp->append(sa, QT_TRANSLATE_NOOP("palette", "Soprano/Alto"))->setElementTranslated(true);

        auto tb = makeElement<StaffText>(gscore);
        tb->setXmlText(QT_TRANSLATE_NOOP("palette", "T/B"));
        tb->setChannelName(0, "Tenor");
        tb->setChannelName(1, "Bass");
        tb->setChannelName(2, "Tenor");
        tb->setChannelName(3, "Bass");
        tb->setVisible(false);
        sp->append(tb, QT_TRANSLATE_NOOP("palette", "Tenor/Bass"))->setElementTranslated(true);

        auto tl = makeElement<StaffText>(gscore);
        tl->setXmlText(QT_TRANSLATE_NOOP("palette", "T/L"));
        tl->setChannelName(0, "TENOR");
        tl->setChannelName(1, "LEAD");
        tl->setChannelName(2, "TENOR");
        tl->setChannelName(3, "LEAD");
        tl->setVisible(false);
        sp->append(tl, QT_TRANSLATE_NOOP("palette", "Tenor/Lead"))->setElementTranslated(true);

        auto bb = makeElement<StaffText>(gscore);
        bb->setXmlText(QT_TRANSLATE_NOOP("palette", "B/B"));
        bb->setChannelName(0, "BARI");
        bb->setChannelName(1, "BASS");
        bb->setChannelName(2, "BARI");
        bb->setChannelName(3, "BASS");
        bb->setVisible(false);
        sp->append(bb, QT_TRANSLATE_NOOP("palette", "Bari/Bass"))->setElementTranslated(true);
    }

    return sp;
}

//---------------------------------------------------------
//   newTimePalettePanel
//    create default time signature palette
//---------------------------------------------------------

PalettePanel* PaletteCreator::newTimePalettePanel()
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

    PalettePanel* sp = new PalettePanel(PalettePanel::Type::TimeSig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Time Signatures"));
    sp->setMag(.8);
    sp->setGrid(42, 38);

    for (unsigned i = 0; i < sizeof(tsList) / sizeof(*tsList); ++i) {
        auto ts = makeElement<TimeSig>(gscore);
        ts->setSig(Fraction(tsList[i].numerator, tsList[i].denominator), tsList[i].type);
        sp->append(ts, tsList[i].name);
    }
    return sp;
}

//-----------------------------------
//    newFretboardDiagramPalettePanel
//-----------------------------------

PalettePanel* PaletteCreator::newFretboardDiagramPalettePanel()
{
    PalettePanel* sp = new PalettePanel(PalettePanel::Type::FretboardDiagram);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fretboard Diagrams"));
    sp->setGrid(42, 45);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto fret = FretDiagram::createFromString(gscore, "X32O1O");
    fret->setHarmony("C");
    sp->append(fret, "C");
    fret = FretDiagram::createFromString(gscore, "X-554-");
    fret->setHarmony("Cm");
    sp->append(fret, "Cm");
    fret = FretDiagram::createFromString(gscore, "X3231O");
    fret->setHarmony("C7");
    sp->append(fret, "C7");

    fret = FretDiagram::createFromString(gscore, "XXO232");
    fret->setHarmony("D");
    sp->append(fret, "D");
    fret = FretDiagram::createFromString(gscore, "XXO231");
    fret->setHarmony("Dm");
    sp->append(fret, "Dm");
    fret = FretDiagram::createFromString(gscore, "XXO212");
    fret->setHarmony("D7");
    sp->append(fret, "D7");

    fret = FretDiagram::createFromString(gscore, "O221OO");
    fret->setHarmony("E");
    sp->append(fret, "E");
    fret = FretDiagram::createFromString(gscore, "O22OOO");
    fret->setHarmony("Em");
    sp->append(fret, "Em");
    fret = FretDiagram::createFromString(gscore, "O2O1OO");
    fret->setHarmony("E7");
    sp->append(fret, "E7");

    fret = FretDiagram::createFromString(gscore, "-332--");
    fret->setHarmony("F");
    sp->append(fret, "F");
    fret = FretDiagram::createFromString(gscore, "-33---");
    fret->setHarmony("Fm");
    sp->append(fret, "Fm");
    fret = FretDiagram::createFromString(gscore, "-3-2--");
    fret->setHarmony("F7");
    sp->append(fret, "F7");

    fret = FretDiagram::createFromString(gscore, "32OOO3");
    fret->setHarmony("G");
    sp->append(fret, "G");
    fret = FretDiagram::createFromString(gscore, "-55---");
    fret->setHarmony("Gm");
    sp->append(fret, "Gm");
    fret = FretDiagram::createFromString(gscore, "32OOO1");
    fret->setHarmony("G7");
    sp->append(fret, "G7");

    fret = FretDiagram::createFromString(gscore, "XO222O");
    fret->setHarmony("A");
    sp->append(fret, "A");
    fret = FretDiagram::createFromString(gscore, "XO221O");
    fret->setHarmony("Am");
    sp->append(fret, "Am");
    fret = FretDiagram::createFromString(gscore, "XO2O2O");
    fret->setHarmony("A7");
    sp->append(fret, "A7");

    fret = FretDiagram::createFromString(gscore, "X-444-");
    fret->setHarmony("B");
    sp->append(fret, "B");
    fret = FretDiagram::createFromString(gscore, "X-443-");
    fret->setHarmony("Bm");
    sp->append(fret, "Bm");
    fret = FretDiagram::createFromString(gscore, "X212O2");
    fret->setHarmony("B7");
    sp->append(fret, "B7");

    return sp;
}
}
