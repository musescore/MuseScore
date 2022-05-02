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

#include "palettecreator.h"

#include "translation.h"

#include "engraving/types/typesconv.h"
#include "engraving/types/symnames.h"

#include "libmscore/factory.h"
#include "libmscore/accidental.h"
#include "libmscore/actionicon.h"
#include "libmscore/ambitus.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/bagpembell.h"
#include "libmscore/barline.h"
#include "libmscore/bend.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/chordline.h"
#include "libmscore/chordrest.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/fermata.h"
#include "libmscore/fingering.h"
#include "libmscore/fret.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/instrchange.h"
#include "libmscore/jump.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/letring.h"
#include "libmscore/tempochangeranged.h"
#include "libmscore/marker.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/palmmute.h"
#include "libmscore/pedal.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/segment.h"
#include "libmscore/select.h"
#include "libmscore/slur.h"
#include "libmscore/spacer.h"
#include "libmscore/staffstate.h"
#include "libmscore/stafftext.h"
#include "libmscore/playtechannotation.h"
#include "libmscore/stafftypechange.h"
#include "libmscore/systemtext.h"
#include "libmscore/tempo.h"
#include "libmscore/tempotext.h"
#include "libmscore/textline.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tremolobar.h"
#include "libmscore/trill.h"
#include "libmscore/undo.h"
#include "libmscore/vibrato.h"
#include "libmscore/volta.h"

using namespace mu::palette;
using namespace mu::engraving;
using namespace Ms;

template<typename T> std::shared_ptr<T> makeElement(Ms::Score* score)
{
    return std::make_shared<T>(score->dummy());
}

#define MAKE_ELEMENT(T, P) \
    template<> \
    std::shared_ptr<T> makeElement<T>(Ms::Score * score) { return std::make_shared<T>(P); } \

MAKE_ELEMENT(Dynamic, score->dummy()->segment())
MAKE_ELEMENT(MeasureRepeat, score->dummy()->segment())
MAKE_ELEMENT(Hairpin, score->dummy()->segment())
MAKE_ELEMENT(SystemText, score->dummy()->segment())
MAKE_ELEMENT(TempoText, score->dummy()->segment())
MAKE_ELEMENT(StaffText, score->dummy()->segment())
MAKE_ELEMENT(PlayTechAnnotation, score->dummy()->segment())
MAKE_ELEMENT(RehearsalMark, score->dummy()->segment())

MAKE_ELEMENT(Jump, score->dummy()->measure())
MAKE_ELEMENT(MeasureNumber, score->dummy()->measure())

MAKE_ELEMENT(Fingering, score->dummy()->note())
MAKE_ELEMENT(NoteHead, score->dummy()->note())

PaletteTreePtr PaletteCreator::newMasterPaletteTree()
{
    PaletteTreePtr tree = std::make_shared<PaletteTree>();

    tree->append(newClefsPalette());
    tree->append(newKeySigPalette());
    tree->append(newTimePalette());
    tree->append(newTempoPalette());
    tree->append(newAccidentalsPalette());
    tree->append(newDynamicsPalette());
    tree->append(newArticulationsPalette());
    tree->append(newTextPalette());
    tree->append(newLinesPalette());
    tree->append(newRepeatsPalette());
    tree->append(newBarLinePalette());
    tree->append(newLayoutPalette());
    tree->append(newBracketsPalette());
    tree->append(newOrnamentsPalette());
    tree->append(newBreathPalette());
    tree->append(newGraceNotePalette());
    tree->append(newNoteHeadsPalette());
    tree->append(newArpeggioPalette());
    tree->append(newTremoloPalette());
    tree->append(newFingeringPalette());
    tree->append(newFretboardDiagramPalette());
    tree->append(newAccordionPalette());
    tree->append(newBagpipeEmbellishmentPalette());
    tree->append(newBeamPalette());

    return tree;
}

PaletteTreePtr PaletteCreator::newDefaultPaletteTree()
{
    PaletteTreePtr defaultPalette = std::make_shared<PaletteTree>();

    defaultPalette->append(newClefsPalette(true));
    defaultPalette->append(newKeySigPalette());
    defaultPalette->append(newTimePalette(true));
    defaultPalette->append(newTempoPalette(true));
    defaultPalette->append(newAccidentalsPalette(true));
    defaultPalette->append(newDynamicsPalette(true));
    defaultPalette->append(newArticulationsPalette(true));
    defaultPalette->append(newTextPalette(true));
    defaultPalette->append(newLinesPalette(true));
    defaultPalette->append(newRepeatsPalette(true));
    defaultPalette->append(newBarLinePalette(true));
    defaultPalette->append(newLayoutPalette());
    defaultPalette->append(newBracketsPalette());
    defaultPalette->append(newOrnamentsPalette(true));
    defaultPalette->append(newBreathPalette(true));
    defaultPalette->append(newGraceNotePalette());
    defaultPalette->append(newNoteHeadsPalette());
    defaultPalette->append(newArpeggioPalette());
    defaultPalette->append(newTremoloPalette());
    defaultPalette->append(newFingeringPalette(true));
    defaultPalette->append(newFretboardDiagramPalette());
    defaultPalette->append(newAccordionPalette());
    defaultPalette->append(newBagpipeEmbellishmentPalette());
    defaultPalette->append(newBeamPalette());

    return defaultPalette;
}

PalettePtr PaletteCreator::newBeamPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Beam);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Beam properties"));
    sp->setGridSize(27, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    sp->appendActionIcon(ActionIconType::BEAM_START, "beam-start");
    sp->appendActionIcon(ActionIconType::BEAM_MID, "beam-mid");
    sp->appendActionIcon(ActionIconType::BEAM_NONE, "no-beam");
    sp->appendActionIcon(ActionIconType::BEAM_BEGIN_32, "beam-32");
    sp->appendActionIcon(ActionIconType::BEAM_BEGIN_64, "beam-64");
    sp->appendActionIcon(ActionIconType::BEAM_AUTO, "auto-beam");
    sp->appendActionIcon(ActionIconType::BEAM_FEATHERED_SLOWER, "fbeam1");
    sp->appendActionIcon(ActionIconType::BEAM_FEATHERED_FASTER, "fbeam2");

    return sp;
}

PalettePtr PaletteCreator::newDynamicsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Dynamic);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Dynamics"));
    sp->setGridSize(defaultPalette ? 50 : 60, 28);
    sp->setDrawGrid(true);

    static const std::vector<const char*> defaultDynamics = {
        "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
        "rfz", "rf", "fz"
    };

    static const std::vector<const char*> masterDynamics = {
        "pppppp", "ppppp", "pppp",
        "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
        "ffff", "fffff", "ffffff",
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
        "rfz", "rf", "fz", "m", "r", "s", "z", "n"
    };

    for (const char* dynamicType : defaultPalette ? defaultDynamics : masterDynamics) {
        auto dynamic = makeElement<Dynamic>(Ms::gpaletteScore);
        dynamic->setDynamicType(dynamicType);
        sp->appendElement(dynamic, TConv::toUserName(dynamic->dynamicType()));
    }
    return sp;
}

PalettePtr PaletteCreator::newKeySigPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::KeySig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Key signatures"));
    sp->setMag(0.8);
    sp->setGridSize(56, 50);
    sp->setDrawGrid(true);
    sp->setYOffset(1.0);

    for (int i = 0; i < 7; ++i) {
        auto k = Factory::makeKeySig(Ms::gpaletteScore->dummy()->segment());
        k->setKey(Key(i + 1));
        sp->appendElement(k, keyNames[i * 2]);
    }
    for (int i = -7; i < 0; ++i) {
        auto k = Factory::makeKeySig(Ms::gpaletteScore->dummy()->segment());
        k->setKey(Key(i));
        sp->appendElement(k, keyNames[(7 + i) * 2 + 1]);
    }
    auto k = Factory::makeKeySig(Ms::gpaletteScore->dummy()->segment());
    k->setKey(Key::C);
    sp->appendElement(k, keyNames[14]);

    // atonal key signature
    KeySigEvent nke;
    nke.setKey(Key::C);
    nke.setCustom(true);
    nke.setMode(KeyMode::NONE);
    auto nk = Factory::makeKeySig(Ms::gpaletteScore->dummy()->segment());
    nk->setKeySigEvent(nke);
    sp->appendElement(nk, keyNames[15]);

    return sp;
}

PalettePtr PaletteCreator::newAccidentalsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Accidental);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Accidentals"));
    sp->setGridSize(33, 36);
    sp->setDrawGrid(true);

    int end = 0;
    if (defaultPalette) {
        end = int(AccidentalType::SHARP3);
    } else {
        end = int(AccidentalType::END);
    }

    for (int i = int(AccidentalType::FLAT); i < end; ++i) {
        auto ac = Factory::makeAccidental(gpaletteScore->dummy());
        ac->setAccidentalType(AccidentalType(i));
        if (ac->symbol() != SymId::noSym) {
            sp->appendElement(ac, ac->subtypeUserName());
        }
    }

    if (!defaultPalette) {
        sp->appendActionIcon(ActionIconType::PARENTHESES, "add-parentheses");
        sp->appendActionIcon(ActionIconType::BRACKETS, "add-brackets");
    }

    return sp;
}

PalettePtr PaletteCreator::newBarLinePalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::BarLine);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Barlines"));
    sp->setMag(0.8);
    sp->setGridSize(48, 38);
    sp->setDrawGrid(true);

    // bar line styles
    for (unsigned i = 0;; ++i) {
        const BarLineTableItem* bti = BarLine::barLineTableItem(i);
        if (!bti) {
            break;
        }
        auto b = Factory::makeBarLine(gpaletteScore->dummy()->segment());
        b->setBarLineType(bti->type);
        sp->appendElement(b, BarLine::userTypeName(bti->type));
    }

    // bar line spans
    if (!defaultPalette) {
        const struct {
            int from, to;
            const char* userName;
        } spans[] = {
            { BARLINE_SPAN_TICK1_FROM,  BARLINE_SPAN_TICK1_TO,  SymNames::userNameForSymId(SymId::barlineDashed) },
            { BARLINE_SPAN_TICK2_FROM,  BARLINE_SPAN_TICK2_TO,  QT_TRANSLATE_NOOP("symUserNames", "Tick barline 2") },  // Not in SMuFL
            { BARLINE_SPAN_SHORT1_FROM, BARLINE_SPAN_SHORT1_TO, SymNames::userNameForSymId(SymId::barlineShort) },
            { BARLINE_SPAN_SHORT2_FROM, BARLINE_SPAN_SHORT2_TO, QT_TRANSLATE_NOOP("symUserNames", "Short barline 2") }, // Not in SMuFL
        };
        for (auto span : spans) {
            auto b = Factory::makeBarLine(gpaletteScore->dummy()->segment());
            b->setBarLineType(BarLineType::NORMAL);
            b->setSpanFrom(span.from);
            b->setSpanTo(span.to);
            sp->appendElement(b, span.userName);
        }
    }
    return sp;
}

PalettePtr PaletteCreator::newRepeatsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Repeat);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Repeats & jumps"));
    sp->setMag(0.85);
    sp->setGridSize(75, 28);
    sp->setDrawGrid(true);

    struct MeasureRepeatInfo
    {
        SymId id = SymId::noSym;
        int measuresCount = 0;
    };

    std::vector<MeasureRepeatInfo> defaultMeasureRepeats {
        { SymId::repeat1Bar, 1 }
    };

    std::vector<MeasureRepeatInfo> masterMeasureRepeats {
        { SymId::repeat1Bar, 1 },
        { SymId::repeat2Bars, 2 },
        { SymId::repeat4Bars, 4 }
    };

    for (MeasureRepeatInfo repeat: defaultPalette ? defaultMeasureRepeats : masterMeasureRepeats) {
        auto rm = makeElement<MeasureRepeat>(gpaletteScore);
        rm->setSymId(repeat.id);
        rm->setNumMeasures(repeat.measuresCount);
        sp->appendElement(rm, mu::qtrc("symUserNames", SymNames::userNameForSymId(repeat.id)));
    }

    const std::vector<MarkerTypeItem> defaultMarkers = {
        { Marker::Type::SEGNO, QT_TRANSLATE_NOOP("markerType", "Segno") },
        { Marker::Type::CODA, QT_TRANSLATE_NOOP("markerType", "Coda") },
        { Marker::Type::FINE, QT_TRANSLATE_NOOP("markerType", "Fine") },
        { Marker::Type::TOCODA, QT_TRANSLATE_NOOP("markerType", "To coda") }
    };

    for (MarkerTypeItem markerTypeItem : (defaultPalette ? defaultMarkers : markerTypeTable)) {
        if (markerTypeItem.type == Marker::Type::CODETTA || markerTypeItem.type == Marker::Type::USER) {// Codetta not in SMuFL
            continue;
        }

        auto mk = makeElement<Marker>(gpaletteScore);
        mk->setMarkerType(markerTypeItem.type);
        mk->styleChanged();
        sp->appendElement(mk, markerTypeItem.name);
    }

    for (int i = 0; i < jumpTypeTableSize(); i++) {
        auto jp = makeElement<Jump>(gpaletteScore);
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

        auto b = Factory::makeBarLine(gpaletteScore->dummy()->segment());
        b->setBarLineType(bti->type);
        PaletteCellPtr cell = sp->appendElement(b, BarLine::userTypeName(bti->type));
        cell->drawStaff = false;
    }

    return sp;
}

PalettePtr PaletteCreator::newLayoutPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Layout);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Layout"));
    sp->setGridSize(42, 36);
    sp->setDrawGrid(true);

    auto lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
    lb->setLayoutBreakType(LayoutBreakType::LINE);
    PaletteCellPtr cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "System break"));
    cell->mag = 1.2;

    lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
    lb->setLayoutBreakType(LayoutBreakType::PAGE);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "Page break"));
    cell->mag = 1.2;

    lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
    lb->setLayoutBreakType(LayoutBreakType::SECTION);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "Section break"));
    cell->mag = 1.2;

    lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
    lb->setLayoutBreakType(LayoutBreakType::NOBREAK);
    cell = sp->appendElement(lb, QT_TRANSLATE_NOOP("palette", "Group measures"));
    cell->mag = 1.2;

    qreal _spatium = gpaletteScore->spatium();
    auto spacer = Factory::makeSpacer(gpaletteScore->dummy()->measure());
    spacer->setSpacerType(SpacerType::DOWN);
    spacer->setGap(Millimetre(3 * _spatium));
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer down"));
    cell->mag = .7;

    spacer = Factory::makeSpacer(gpaletteScore->dummy()->measure());
    spacer->setSpacerType(SpacerType::UP);
    spacer->setGap(Millimetre(3 * _spatium));
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer up"));
    cell->mag = .7;

    spacer = Factory::makeSpacer(gpaletteScore->dummy()->measure());
    spacer->setSpacerType(SpacerType::FIXED);
    spacer->setGap(Millimetre(3 * _spatium));
    cell = sp->appendElement(spacer, QT_TRANSLATE_NOOP("palette", "Staff spacer fixed down"));
    cell->mag = .7;

    auto stc = Factory::makeStaffTypeChange(gpaletteScore->dummy()->measure());
    sp->appendElement(stc, QT_TRANSLATE_NOOP("palette", "Staff type change"));

    sp->appendActionIcon(ActionIconType::VFRAME, "insert-vbox");
    sp->appendActionIcon(ActionIconType::HFRAME, "insert-hbox");
    sp->appendActionIcon(ActionIconType::TFRAME, "insert-textframe");
    if (configuration()->enableExperimental()) {
        sp->appendActionIcon(ActionIconType::FFRAME, "insert-fretframe");
    }
    sp->appendActionIcon(ActionIconType::MEASURE, "insert-measure");

    return sp;
}

PalettePtr PaletteCreator::newFingeringPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Fingering);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fingering"));
    sp->setMag(1.5);
    sp->setGridSize(28, 30);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    const char* finger = "012345";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "Fingering %1"));
    }

    finger = "pimac";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->setTextStyleType(TextStyleType::RH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "RH guitar fingering %1"));
    }

    finger = "012345T";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->setTextStyleType(TextStyleType::LH_GUITAR_FINGERING);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "LH guitar fingering %1"));
    }
    finger = "0123456";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->setTextStyleType(TextStyleType::STRING_NUMBER);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "String number %1"));
    }

    static const SymIdList defaultLute {
        SymId::stringsThumbPosition
    };
    static const SymIdList masterLute {
        SymId::stringsThumbPosition,
        SymId::luteFingeringRHThumb, SymId::luteFingeringRHFirst,
        SymId::luteFingeringRHSecond, SymId::luteFingeringRHThird
    };
    // include additional symbol-based fingerings (temporarily?) implemented as articulations
    for (auto i : defaultPalette ? defaultLute : masterLute) {
        auto s = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        s->setSymId(i);
        sp->appendElement(s, s->typeUserName());
    }
    return sp;
}

PalettePtr PaletteCreator::newTremoloPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Tremolo);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Tremolos"));
    sp->setGridSize(27, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
        auto tremolo = Factory::makeTremolo(gpaletteScore->dummy()->chord());
        tremolo->setTremoloType(TremoloType(i));
        sp->appendElement(tremolo, tremolo->subtypeName());
    }
    return sp;
}

PalettePtr PaletteCreator::newNoteHeadsPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::NoteHead);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Noteheads"));
    sp->setMag(1.3);
    sp->setGridSize(33, 36);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = 0; i < int(NoteHeadGroup::HEAD_DO_WALKER); ++i) {
        SymId sym = Note::noteHead(0, NoteHeadGroup(i), NoteHeadType::HEAD_HALF);
        // HEAD_BREVIS_ALT shows up only for brevis value
        if (i == int(NoteHeadGroup::HEAD_BREVIS_ALT)) {
            sym = Note::noteHead(0, NoteHeadGroup(i), NoteHeadType::HEAD_BREVIS);
        }
        auto nh = makeElement<NoteHead>(gpaletteScore);
        nh->setSym(sym);
        sp->appendElement(nh, TConv::toUserName((NoteHeadGroup(i))));
    }

    sp->appendActionIcon(ActionIconType::PARENTHESES, "add-parentheses");

    return sp;
}

PalettePtr PaletteCreator::newArticulationsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Articulation);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Articulations"));
    sp->setGridSize(42, 25);
    sp->setDrawGrid(true);

    static const SymIdList defaultArticulations {
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
        SymId::brassMuteOpen,
        SymId::brassMuteClosed,
        SymId::stringsHarmonic,
        SymId::stringsUpBow,
        SymId::stringsDownBow
    };

    static const SymIdList masterArticulations {
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

    for (SymId articulationType : defaultPalette ? defaultArticulations : masterArticulations) {
        auto artic = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        artic->setSymId(articulationType);
        sp->appendElement(artic, artic->typeUserName());
    }

    if (!defaultPalette) {
        auto bend = Factory::makeBend(gpaletteScore->dummy()->note());
        bend->points().push_back(PitchValue(0,    0, false));
        bend->points().push_back(PitchValue(15, 100, false));
        bend->points().push_back(PitchValue(60, 100, false));
        sp->appendElement(bend, QT_TRANSLATE_NOOP("palette", "Bend"));

        auto tb = Factory::makeTremoloBar(gpaletteScore->dummy());
        tb->points().push_back(PitchValue(0,     0, false));       // "Dip"
        tb->points().push_back(PitchValue(30, -100, false));
        tb->points().push_back(PitchValue(60,    0, false));
        sp->appendElement(tb, QT_TRANSLATE_NOOP("palette", "Tremolo bar"));
    }

    return sp;
}

PalettePtr PaletteCreator::newOrnamentsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Ornament);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Ornaments"));
    sp->setGridSize(42, 25);
    sp->setMag(1.2);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
    static const SymIdList defaultOrnaments {
        SymId::ornamentTurnInverted,
        SymId::ornamentTurn,
        SymId::ornamentTrill,
        SymId::ornamentShortTrill,
        SymId::ornamentMordent
    };

    static const SymIdList masterOrnaments {
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
        SymId::ornamentPrecompSlide
    };

    for (auto ornamentType : defaultPalette ? defaultOrnaments : masterOrnaments) {
        auto ornament = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        ornament->setSymId(ornamentType);
        sp->appendElement(ornament, ornament->typeUserName());
    }
    return sp;
}

PalettePtr PaletteCreator::newAccordionPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Accordion);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Accordion"));
    sp->setGridSize(42, 25);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
    static SymIdList art {
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
        auto s = makeElement<Symbol>(gpaletteScore);
        s->setSym(i);
        sp->appendElement(s, SymNames::translatedUserNameForSymId(i));
    }
    return sp;
}

PalettePtr PaletteCreator::newBracketsPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Bracket);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Brackets"));
    sp->setMag(0.7);
    sp->setGridSize(40, 60);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    std::array<std::pair<BracketType, const char*>, 4> types { {
        { BracketType::NORMAL, QT_TRANSLATE_NOOP("palette", "Bracket") },
        { BracketType::BRACE,  QT_TRANSLATE_NOOP("palette", "Brace") },
        { BracketType::SQUARE, QT_TRANSLATE_NOOP("palette", "Square") },
        { BracketType::LINE,   QT_TRANSLATE_NOOP("palette", "Line") }
    } };

    static Part* bracketItemOwnerPart = new Part(gpaletteScore);
    static Staff* bracketItemOwner = Factory::createStaff(bracketItemOwnerPart);
    bracketItemOwner->setBracketType(static_cast<int>(types.size()) - 1, BracketType::NORMAL);

    for (size_t i = 0; i < types.size(); ++i) {
        auto b1 = Factory::makeBracket(gpaletteScore->dummy());
        auto bi1 = bracketItemOwner->brackets()[static_cast<int>(i)];
        const auto& type = types[i];
        bi1->setBracketType(type.first);
        b1->setBracketItem(bi1);
        sp->appendElement(b1, type.second); // Bracket, Brace, Square, Line
    }
    return sp;
}

PalettePtr PaletteCreator::newBreathPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Breath);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Breaths & pauses"));
    sp->setGridSize(40, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    // Fermatas

    static const SymIdList defaultFermatas {
        SymId::fermataAbove,
        SymId::fermataShortAbove,
        SymId::fermataLongAbove
    };

    static const SymIdList masterFermatas {
        SymId::fermataAbove,
        SymId::fermataShortAbove,
        SymId::fermataLongAbove,
        SymId::fermataLongHenzeAbove,
        SymId::fermataShortHenzeAbove,
        SymId::fermataVeryLongAbove,
        SymId::fermataVeryShortAbove,
    };

    // Breaths

    for (auto i : defaultPalette ? defaultFermatas : masterFermatas) {
        auto f = Factory::makeFermata(gpaletteScore->dummy());
        f->setSymId(i);
        sp->appendElement(f, f->typeUserName());
    }

    for (BreathType breath : Breath::breathList) { //Last breath is not a default breath. Hence, - 1
        if (breath.id == SymId::chantCaesura && defaultPalette) {
            continue;
        }
        auto a = Factory::makeBreath(gpaletteScore->dummy()->segment());
        a->setSymId(breath.id);
        a->setPause(breath.pause);
        sp->appendElement(a, SymNames::translatedUserNameForSymId(breath.id));
    }

    return sp;
}

PalettePtr PaletteCreator::newArpeggioPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Arpeggio);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Arpeggios & glissandi"));
    sp->setGridSize(42, 44);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (int i = 0; i < 6; ++i) {
        auto a = Factory::makeArpeggio(gpaletteScore->dummy()->chord());
        a->setArpeggioType(ArpeggioType(i));
        sp->appendElement(a, a->arpeggioTypeName());
    }

    for (int i = 0; i < 2; ++i) {
        auto a = makeElement<Glissando>(gpaletteScore);
        a->setGlissandoType(GlissandoType(i));
        sp->appendElement(a, a->glissandoTypeName());
    }

    //fall and doits

    auto cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::FALL);
    sp->appendElement(cl, scorelineNames[0]);

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::DOIT);
    sp->appendElement(cl, scorelineNames[1]);

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::PLOP);
    sp->appendElement(cl, scorelineNames[2]);

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::SCOOP);
    sp->appendElement(cl, scorelineNames[3]);

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::FALL);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide out down"));

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::DOIT);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide out up"));

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::PLOP);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide in above"));

    cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
    cl->setChordLineType(ChordLineType::SCOOP);
    cl->setStraight(true);
    sp->appendElement(cl, QT_TRANSLATE_NOOP("Ms", "Slide in below"));

    return sp;
}

PalettePtr PaletteCreator::newClefsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Clef);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Clefs"));
    sp->setMag(0.8);
    sp->setGridSize(36, 50);
    sp->setDrawGrid(true);
    sp->setYOffset(1.0);

    static std::vector<ClefType> clefsDefault  {
        ClefType::G,     ClefType::G8_VA,  ClefType::G15_MA,  ClefType::G8_VB,    ClefType::C3,
        ClefType::C4, ClefType::F,   ClefType::F_8VA,
        ClefType::F8_VB, ClefType::PERC, ClefType::TAB, ClefType::TAB4
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

    for (ClefType clefType : defaultPalette ? clefsDefault : clefsMaster) {
        auto clef = Factory::makeClef(gpaletteScore->dummy()->segment());
        clef->setClefType(ClefTypeList(clefType, clefType));
        sp->appendElement(clef, TConv::toUserName(clefType));
    }
    return sp;
}

PalettePtr PaletteCreator::newGraceNotePalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::GraceNote);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Grace notes"));
    sp->setGridSize(45, 40);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    sp->appendActionIcon(ActionIconType::ACCIACCATURA,  "acciaccatura");
    sp->appendActionIcon(ActionIconType::APPOGGIATURA,  "appoggiatura");
    sp->appendActionIcon(ActionIconType::GRACE4,        "grace4");
    sp->appendActionIcon(ActionIconType::GRACE16,       "grace16");
    sp->appendActionIcon(ActionIconType::GRACE32,       "grace32");
    sp->appendActionIcon(ActionIconType::GRACE8_AFTER,  "grace8after");
    sp->appendActionIcon(ActionIconType::GRACE16_AFTER, "grace16after");
    sp->appendActionIcon(ActionIconType::GRACE32_AFTER, "grace32after");

    return sp;
}

PalettePtr PaletteCreator::newBagpipeEmbellishmentPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::BagpipeEmbellishment);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Bagpipe embellishments"));
    sp->setMag(0.8);
    sp->setYOffset(2.0);
    sp->setGridSize(55, 55);
    sp->setVisible(false);

    for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
        auto b = makeElement<BagpipeEmbellishment>(gpaletteScore);
        b->setEmbelType(i);
        sp->appendElement(b, BagpipeEmbellishment::BagpipeEmbellishmentList[i].name);
    }

    return sp;
}

PalettePtr PaletteCreator::newLinesPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Line);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Lines"));
    sp->setMag(.8);
    sp->setGridSize(75, 28);
    sp->setDrawGrid(true);

    qreal w = gpaletteScore->spatium() * 8;

    auto slur = Factory::makeSlur(gpaletteScore->dummy());
    sp->appendElement(slur, QT_TRANSLATE_NOOP("palette", "Slur"));

    auto gabel0 = Factory::makeHairpin(gpaletteScore->dummy()->segment());
    gabel0->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel0->setLen(w);
    sp->appendElement(gabel0, QT_TRANSLATE_NOOP("palette", "Crescendo hairpin"));

    auto gabel1 = Factory::makeHairpin(gpaletteScore->dummy()->segment());
    gabel1->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    gabel1->setLen(w);
    sp->appendElement(gabel1, QT_TRANSLATE_NOOP("palette", "Diminuendo hairpin"));

    auto gabel2 = Factory::makeHairpin(gpaletteScore->dummy()->segment());
    gabel2->setHairpinType(HairpinType::CRESC_LINE);
    gabel2->setLen(w);
    sp->appendElement(gabel2, QT_TRANSLATE_NOOP("palette", "Crescendo line"));

    auto gabel3 = Factory::makeHairpin(gpaletteScore->dummy()->segment());
    gabel3->setHairpinType(HairpinType::DECRESC_LINE);
    gabel3->setLen(w);
    sp->appendElement(gabel3, QT_TRANSLATE_NOOP("palette", "Diminuendo line"));

    auto gabel4 = Factory::makeHairpin(gpaletteScore->dummy()->segment());
    gabel4->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel4->setBeginText("<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    gabel4->setBeginTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    gabel4->setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, PropertyFlags::UNSTYLED);
    gabel4->setLen(w);
    sp->appendElement(gabel4, QT_TRANSLATE_NOOP("palette", "Dynamic + hairpin"));

    auto volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText("1.");
    std::vector<int> il;
    il.push_back(1);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Prima volta"));

    if (!defaultPalette) {
        volta = makeElement<Volta>(gpaletteScore);
        volta->setVoltaType(Volta::Type::CLOSED);
        volta->setLen(w);
        volta->setText("2.");
        il.clear();
        il.push_back(2);
        volta->setEndings(il);
        sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta"));

        volta = makeElement<Volta>(gpaletteScore);
        volta->setVoltaType(Volta::Type::CLOSED);
        volta->setLen(w);
        volta->setText("3.");
        il.clear();
        il.push_back(3);
        volta->setEndings(il);
        sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Terza volta"));
    }

    volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::OPEN);
    volta->setLen(w);
    volta->setText("2.");
    il.clear();
    il.push_back(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta, open"));

    auto ottava = makeElement<Ottava>(gpaletteScore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "8va alta"));

    ottava = makeElement<Ottava>(gpaletteScore);
    ottava->setOttavaType(OttavaType::OTTAVA_8VB);
    ottava->setLen(w);
    ottava->setPlacement(PlacementV::BELOW);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "8va bassa"));

    ottava = makeElement<Ottava>(gpaletteScore);
    ottava->setOttavaType(OttavaType::OTTAVA_15MA);
    ottava->setLen(w);
    ottava->styleChanged();
    sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "15ma alta"));

    if (!defaultPalette) {
        ottava = makeElement<Ottava>(gpaletteScore);
        ottava->setOttavaType(OttavaType::OTTAVA_15MB);
        ottava->setLen(w);
        ottava->setPlacement(PlacementV::BELOW);
        ottava->styleChanged();
        sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "15ma bassa"));

        ottava = makeElement<Ottava>(gpaletteScore);
        ottava->setOttavaType(OttavaType::OTTAVA_22MA);
        ottava->setLen(w);
        ottava->styleChanged();
        sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "22ma alta"));

        ottava = makeElement<Ottava>(gpaletteScore);
        ottava->setOttavaType(OttavaType::OTTAVA_22MB);
        ottava->setPlacement(PlacementV::BELOW);
        ottava->setLen(w);
        ottava->styleChanged();
        sp->appendElement(ottava, QT_TRANSLATE_NOOP("palette", "22ma bassa"));
    }

    auto pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginText(Pedal::PEDAL_SYMBOL);
    pedal->setContinueText(QString("(%1)").arg(Pedal::PEDAL_SYMBOL));
    pedal->setEndHookType(HookType::HOOK_90);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and line)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginText(Pedal::PEDAL_SYMBOL);
    pedal->setContinueText(QString("(%1)").arg(Pedal::PEDAL_SYMBOL));
    pedal->setEndText(Pedal::STAR_SYMBOL);
    pedal->setLineVisible(false);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and asterisk)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_90);
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (straight hooks)"));

    if (!defaultPalette) {
        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_90);
        pedal->setEndHookType(HookType::HOOK_45);
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled end hook)"));

        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_45);
        pedal->setEndHookType(HookType::HOOK_45);
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (both hooks angled)"));

        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_45);
        pedal->setEndHookType(HookType::HOOK_90);
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled start hook)"));
    }

    const std::vector<TrillTableItem> defaultTrills = {
        { Trill::Type::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("trillType", "Trill line") },
        { Trill::Type::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("trillType", "Downprall line") },
        { Trill::Type::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("trillType", "Prallprall line") }
    };

    for (TrillTableItem trillTableItem : defaultPalette ? defaultTrills : trillTable) {
        auto trill = makeElement<Trill>(gpaletteScore);
        trill->setTrillType(trillTableItem.type);
        trill->setLen(w);
        sp->appendElement(trill, trillTableItem.userName);
    }

    auto staffTextLine = makeElement<TextLine>(gpaletteScore);
    staffTextLine->setLen(w);
    staffTextLine->setBeginText("Staff");
    staffTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(staffTextLine, QT_TRANSLATE_NOOP("palette", "Staff Text line"));

    auto systemTextLine = makeElement<TextLine>(gpaletteScore);
    systemTextLine->setSystemFlag(true);
    systemTextLine->setLen(w);
    systemTextLine->setBeginText("System");
    systemTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(systemTextLine, QT_TRANSLATE_NOOP("palette", "System Text line"));

    if (!defaultPalette) {
        auto textLine = makeElement<TextLine>(gpaletteScore);
        textLine->setLen(w);
        textLine->setBeginText("VII");
        textLine->setEndHookType(HookType::HOOK_90);
        sp->appendElement(textLine, QT_TRANSLATE_NOOP("palette", "Text line"));
    }

    auto line = makeElement<TextLine>(gpaletteScore);
    line->setLen(w);
    line->setDiagonal(true);
    sp->appendElement(line, QT_TRANSLATE_NOOP("palette", "Line"));

    auto a = Factory::makeAmbitus(gpaletteScore->dummy()->segment());
    sp->appendElement(a, QT_TRANSLATE_NOOP("palette", "Ambitus"));

    auto letRing = makeElement<LetRing>(gpaletteScore);
    letRing->setLen(w);
    sp->appendElement(letRing, QT_TRANSLATE_NOOP("palette", "Let ring"));

    const std::vector<VibratoTableItem> defaultVibratoTable = {
        { Vibrato::Type::GUITAR_VIBRATO,        "guitarVibrato",       QT_TRANSLATE_NOOP("vibratoType", "Guitar vibrato") },
        { Vibrato::Type::VIBRATO_SAWTOOTH,      "vibratoSawtooth",     QT_TRANSLATE_NOOP("vibratoType", "Vibrato sawtooth") },
    };

    for (VibratoTableItem vibratoTableItem : defaultPalette ? defaultVibratoTable : vibratoTable) {
        auto vibrato = makeElement<Vibrato>(gpaletteScore);
        vibrato->setVibratoType(vibratoTableItem.type);
        vibrato->setLen(w);
        sp->appendElement(vibrato, vibratoTableItem.userName);
    }

    auto pm = makeElement<PalmMute>(gpaletteScore);
    pm->setLen(w);
    sp->appendElement(pm, QT_TRANSLATE_NOOP("palette", "Palm mute"));

    return sp;
}

PalettePtr PaletteCreator::newTempoPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Tempo);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Tempo"));
    sp->setMag(0.65);
    sp->setGridSize(90, 30);
    sp->setDrawGrid(true);

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
            : pattern(s), name(n), f(v), relative(r), italian(i), followText(f), basic(b), masterOnly(m) {}
    };

    static const TempoPattern tps[] = {
        TempoPattern("<sym>metNoteHalfUp</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Half note = 80 BPM"),
                     80.0 / 30.0, false, false, true, true, false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Quarter note = 80 BPM"),
                     80.0 / 60.0, false, false, true, true, false),
        TempoPattern("<sym>metNote8thUp</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Eighth note = 80 BPM"),
                     80.0 / 120.0, false, false, true, true, false),
        TempoPattern("<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Dotted half note = 80 BPM"),
                     120 / 30.0, false, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Dotted quarter note = 80 BPM"),
                     120 / 60.0, false, false, true, true,  false),
        TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80",
                     QT_TRANSLATE_NOOP("palette", "Dotted eighth note = 80 BPM"),
                     120 / 120.0, false, false, true, false, false),

        TempoPattern("Grave",            "Grave",             35.0 / 60.0, false, true, false, false, false),
        TempoPattern("Largo",            "Largo",             50.0 / 60.0, false, true, false, false, false),
        TempoPattern("Lento",            "Lento",             52.5 / 60.0, false, true, false, false, false),
        TempoPattern("Larghetto",        "Larghetto",         63.0 / 60.0, false, true, false, false, true),
        TempoPattern("Adagio",           "Adagio",            71.0 / 60.0, false, true, false, false, false),
        TempoPattern("Andante",          "Andante",           92.0 / 60.0, false, true, false, false, false),
        TempoPattern("Andantino",        "Andantino",         94.0 / 60.0, false, true, false, false, true),
        TempoPattern("Moderato",         "Moderato",         114.0 / 60.0, false, true, false, false, false),
        TempoPattern("Allegretto",       "Allegretto",       116.0 / 60.0, false, true, false, false, false),
        TempoPattern("Allegro",          "Allegro",          144.0 / 60.0, false, true, false, false, false),
        TempoPattern("Vivace",           "Vivace",           172.0 / 60.0, false, true, false, false, false),
        TempoPattern("Presto",           "Presto",           187.0 / 60.0, false, true, false, false, false),
        TempoPattern("Prestissimo",      "Prestissimo",      200.0 / 60.0, false, true, false, false, true),

        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym>",
                     QT_TRANSLATE_NOOP("palette", "Quarter note = dotted quarter note metric modulation"),
                     3.0 / 2.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Dotted quarter note = quarter note metric modulation"),
                     2.0 / 3.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteHalfUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Half note = quarter note metric modulation"),
                     1.0 / 2.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteHalfUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Quarter note = half note metric modulation"),
                     2.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNote8thUp</sym> = <sym>metNote8thUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Eighth note = eighth note metric modulation"),
                     1.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Quarter note = quarter note metric modulation"),
                     1.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Dotted eighth note = quarter note metric modulation"),
                     4.0 / 3.0, true, false, true, false, false),
    };

    for (TempoPattern tp : tps) {
        auto tt = makeElement<TempoText>(gpaletteScore);
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

    static const std::map<TempoTechniqueType, const char*> DEFAULT_TEMPO_CHANGE = {
        { TempoTechniqueType::Accelerando, QT_TRANSLATE_NOOP("palette", "accel.") },
        { TempoTechniqueType::Allargando, QT_TRANSLATE_NOOP("palette", "allarg.") },
        { TempoTechniqueType::Rallentando, QT_TRANSLATE_NOOP("palette", "rall.") },
        { TempoTechniqueType::Ritardando, QT_TRANSLATE_NOOP("palette", "rit.") },
    };

    static const std::map<TempoTechniqueType, const char*> MASTER_TEMPO_CHANGE = {
        { TempoTechniqueType::Accelerando, QT_TRANSLATE_NOOP("palette", "accel.") },
        { TempoTechniqueType::Allargando, QT_TRANSLATE_NOOP("palette", "allarg.") },
        { TempoTechniqueType::Calando, QT_TRANSLATE_NOOP("palette", "calando") },
        { TempoTechniqueType::Lentando, QT_TRANSLATE_NOOP("palette", "lentando") },
        { TempoTechniqueType::Morendo, QT_TRANSLATE_NOOP("palette", "morendo") },
        { TempoTechniqueType::Precipitando, QT_TRANSLATE_NOOP("palette", "precipitando") },
        { TempoTechniqueType::Rallentando, QT_TRANSLATE_NOOP("palette", "rall.") },
        { TempoTechniqueType::Ritardando, QT_TRANSLATE_NOOP("palette", "rit.") },
        { TempoTechniqueType::Smorzando, QT_TRANSLATE_NOOP("palette", "smorz.") },
        { TempoTechniqueType::Sostenuto, QT_TRANSLATE_NOOP("palette", "sost.") },
        { TempoTechniqueType::Stringendo, QT_TRANSLATE_NOOP("palette", "string.") }
    };

    for (const auto& pair : defaultPalette ? DEFAULT_TEMPO_CHANGE : MASTER_TEMPO_CHANGE) {
        auto item = makeElement<TempoChangeRanged>(gpaletteScore);
        item->setTempoChangeType(pair.first);
        item->setBeginText(pair.second);
        sp->appendElement(item, pair.second, 1.3)->yoffset = 0.4;
    }

    auto stxt = makeElement<SystemText>(gpaletteScore);
    stxt->setTextStyleType(TextStyleType::TEMPO);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Swing"));
    stxt->setSwing(true);
    PaletteCellPtr cell = sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "Swing"), 1.3);
    cell->yoffset = 0.4;
    cell->setElementTranslated(true);

    stxt = makeElement<SystemText>(gpaletteScore);
    stxt->setTextStyleType(TextStyleType::TEMPO);
    /*: System text to switch from swing rhythm back to straight rhythm */
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Straight"));
    // need to be true to enable the "Off" option
    stxt->setSwing(true);
    // 0 (swingUnit) turns of swing; swingRatio is set to default
    stxt->setSwingParameters(0, stxt->score()->styleI(Sid::swingRatio));
    /*: System text to switch from swing rhythm back to straight rhythm */
    cell = sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "Straight"), 1.3);
    cell->yoffset = 0.4;
    cell->setElementTranslated(true);
    return sp;
}

PalettePtr PaletteCreator::newTextPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Text);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Text"));
    sp->setMag(0.85);
    sp->setGridSize(84, 28);
    sp->setDrawGrid(true);

    auto st = makeElement<StaffText>(gpaletteScore);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Staff text"));
    sp->appendElement(st, QT_TRANSLATE_NOOP("palette", "Staff text"))->setElementTranslated(true);

    st = makeElement<StaffText>(gpaletteScore);
    st->setTextStyleType(TextStyleType::EXPRESSION);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Expression"));
    st->setPlacement(PlacementV::BELOW);
    st->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    sp->appendElement(st, QT_TRANSLATE_NOOP("palette", "Expression text"))->setElementTranslated(true);

    auto is = makeElement<InstrumentChange>(gpaletteScore);
    is->setXmlText(QT_TRANSLATE_NOOP("palette", "Change instr."));
    sp->appendElement(is, QT_TRANSLATE_NOOP("palette", "Instrument change"))->setElementTranslated(true);

    auto rhm = makeElement<RehearsalMark>(gpaletteScore);
    rhm->setXmlText("B1");
    sp->appendElement(rhm, QT_TRANSLATE_NOOP("palette", "Rehearsal mark"));

    auto stxt = makeElement<SystemText>(gpaletteScore);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "System text"));
    sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "System text"))->setElementTranslated(true);

    auto legato = makeElement<PlayTechAnnotation>(gpaletteScore);
    legato->setXmlText(QT_TRANSLATE_NOOP("palette", "legato"));
    legato->setTechniqueType(PlayingTechniqueType::Legato);
    sp->appendElement(legato, QT_TRANSLATE_NOOP("palette", "Legato"))->setElementTranslated(true);

    auto pz = makeElement<PlayTechAnnotation>(gpaletteScore);
    pz->setXmlText(QT_TRANSLATE_NOOP("palette", "pizz."));
    pz->setTechniqueType(PlayingTechniqueType::Pizzicato);
    sp->appendElement(pz, QT_TRANSLATE_NOOP("palette", "Pizzicato"))->setElementTranslated(true);

    auto ar = makeElement<PlayTechAnnotation>(gpaletteScore);
    ar->setXmlText(QT_TRANSLATE_NOOP("palette", "arco"));
    ar->setTechniqueType(PlayingTechniqueType::Natural);
    sp->appendElement(ar, QT_TRANSLATE_NOOP("palette", "Arco"))->setElementTranslated(true);

    auto tm = makeElement<PlayTechAnnotation>(gpaletteScore);
    tm->setTextStyleType(TextStyleType::EXPRESSION);
    tm->setXmlText(QT_TRANSLATE_NOOP("palette", "tremolo"));
    tm->setTechniqueType(PlayingTechniqueType::Tremolo);
    sp->appendElement(tm, QT_TRANSLATE_NOOP("palette", "Tremolo"))->setElementTranslated(true);

    auto mu = makeElement<PlayTechAnnotation>(gpaletteScore);
    //: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music)
    mu->setXmlText(QT_TRANSLATE_NOOP("palette", "mute"));
    mu->setTechniqueType(PlayingTechniqueType::Mute);
    //: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music)
    sp->appendElement(mu, QT_TRANSLATE_NOOP("palette", "Mute"))->setElementTranslated(true);

    auto no = makeElement<PlayTechAnnotation>(gpaletteScore);
    //: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music)
    no->setXmlText(QT_TRANSLATE_NOOP("palette", "open"));
    no->setTechniqueType(PlayingTechniqueType::Open);
    //: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music)
    sp->appendElement(no, QT_TRANSLATE_NOOP("palette", "Open"))->setElementTranslated(true);

    auto distort = makeElement<PlayTechAnnotation>(gpaletteScore);
    distort->setXmlText(QT_TRANSLATE_NOOP("palette", "distort"));
    distort->setTechniqueType(PlayingTechniqueType::Distortion);
    sp->appendElement(distort, QT_TRANSLATE_NOOP("palette", "Distortion"))->setElementTranslated(true);

    auto overdrive = makeElement<PlayTechAnnotation>(gpaletteScore);
    overdrive->setXmlText(QT_TRANSLATE_NOOP("palette", "overdrive"));
    overdrive->setTechniqueType(PlayingTechniqueType::Overdrive);
    sp->appendElement(overdrive, QT_TRANSLATE_NOOP("palette", "Overdrive"))->setElementTranslated(true);

    auto normal = makeElement<PlayTechAnnotation>(gpaletteScore);
    normal->setXmlText(QT_TRANSLATE_NOOP("palette", "normal"));
    normal->setTechniqueType(PlayingTechniqueType::Natural);
    sp->appendElement(normal, QT_TRANSLATE_NOOP("palette", "Normal"))->setElementTranslated(true);

    if (!defaultPalette) {
        // Measure numbers, unlike other elements (but like most text elements),
        // are not copied directly into the score when drop.
        // Instead, they simply set the corresponding measure's MeasureNumberMode to SHOW
        // Because of that, the element shown in the palettes does not have to have any particular formatting.
        auto meaNum = makeElement<MeasureNumber>(gpaletteScore);
        meaNum->setProperty(Pid::TEXT_STYLE, int(TextStyleType::STAFF));       // Make the element bigger in the palettes (using the default measure number style makes it too small)
        meaNum->setXmlText(QT_TRANSLATE_NOOP("palette", "Measure number"));
        sp->appendElement(meaNum, QT_TRANSLATE_NOOP("palette", "Measure number"))->setElementTranslated(true);

        auto detache = makeElement<PlayTechAnnotation>(gpaletteScore);
        detache->setXmlText(QT_TRANSLATE_NOOP("palette", "detache"));
        detache->setTechniqueType(PlayingTechniqueType::Detache);
        sp->appendElement(detache, QT_TRANSLATE_NOOP("palette", "Detache"))->setElementTranslated(true);

        auto martele = makeElement<PlayTechAnnotation>(gpaletteScore);
        martele->setXmlText(QT_TRANSLATE_NOOP("palette", "martele"));
        martele->setTechniqueType(PlayingTechniqueType::Martele);
        sp->appendElement(martele, QT_TRANSLATE_NOOP("palette", "Martele"))->setElementTranslated(true);

        auto colLegno = makeElement<PlayTechAnnotation>(gpaletteScore);
        colLegno->setXmlText(QT_TRANSLATE_NOOP("palette", "col legno"));
        colLegno->setTechniqueType(PlayingTechniqueType::ColLegno);
        sp->appendElement(colLegno, QT_TRANSLATE_NOOP("palette", "Martele"))->setElementTranslated(true);

        auto sulPont = makeElement<PlayTechAnnotation>(gpaletteScore);
        sulPont->setXmlText(QT_TRANSLATE_NOOP("palette", "sul pont."));
        sulPont->setTechniqueType(PlayingTechniqueType::SulPonticello);
        sp->appendElement(sulPont, QT_TRANSLATE_NOOP("palette", "Sul Ponticello"))->setElementTranslated(true);

        auto sulTasto = makeElement<PlayTechAnnotation>(gpaletteScore);
        sulTasto->setXmlText(QT_TRANSLATE_NOOP("palette", "sul tasto"));
        sulTasto->setTechniqueType(PlayingTechniqueType::SulTasto);
        sp->appendElement(sulTasto, QT_TRANSLATE_NOOP("palette", "Sul Tasto"))->setElementTranslated(true);

        auto vibrato = makeElement<PlayTechAnnotation>(gpaletteScore);
        vibrato->setXmlText(QT_TRANSLATE_NOOP("palette", "vibrato"));
        vibrato->setTechniqueType(PlayingTechniqueType::Vibrato);
        sp->appendElement(vibrato, QT_TRANSLATE_NOOP("palette", "Vibrato"))->setElementTranslated(true);

        auto sa = makeElement<StaffText>(gpaletteScore);
        sa->setXmlText(QT_TRANSLATE_NOOP("palette", "S/A"));
        sa->setChannelName(0, "Soprano");
        sa->setChannelName(1, "Alto");
        sa->setChannelName(2, "Soprano");
        sa->setChannelName(3, "Alto");
        sa->setVisible(false);
        sp->appendElement(sa, QT_TRANSLATE_NOOP("palette", "Soprano/Alto"))->setElementTranslated(true);

        auto tb = makeElement<StaffText>(gpaletteScore);
        tb->setXmlText(QT_TRANSLATE_NOOP("palette", "T/B"));
        tb->setChannelName(0, "Tenor");
        tb->setChannelName(1, "Bass");
        tb->setChannelName(2, "Tenor");
        tb->setChannelName(3, "Bass");
        tb->setVisible(false);
        sp->appendElement(tb, QT_TRANSLATE_NOOP("palette", "Tenor/Bass"))->setElementTranslated(true);

        auto tl = makeElement<StaffText>(gpaletteScore);
        tl->setXmlText(QT_TRANSLATE_NOOP("palette", "T/L"));
        tl->setChannelName(0, "TENOR");
        tl->setChannelName(1, "LEAD");
        tl->setChannelName(2, "TENOR");
        tl->setChannelName(3, "LEAD");
        tl->setVisible(false);
        sp->appendElement(tl, QT_TRANSLATE_NOOP("palette", "Tenor/Lead"))->setElementTranslated(true);

        auto bb = makeElement<StaffText>(gpaletteScore);
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

PalettePtr PaletteCreator::newTimePalette(bool defaultPalette)
{
    struct TS {
        int numerator;
        int denominator;
        TimeSigType type;
        QString name;
    };

    PalettePtr sp = std::make_shared<Palette>(Palette::Type::TimeSig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Time signatures"));
    sp->setMag(.8);
    sp->setGridSize(42, 38);

    static std::vector<TS> defaultTimeSignatureList = {
        { 2,  4, TimeSigType::NORMAL, "2/4" },
        { 3,  4, TimeSigType::NORMAL, "3/4" },
        { 4,  4, TimeSigType::NORMAL, "4/4" },
        { 5,  4, TimeSigType::NORMAL, "5/4" },
        { 6,  4, TimeSigType::NORMAL, "6/4" },
        { 3,  8, TimeSigType::NORMAL, "3/8" },
        { 4,  8, TimeSigType::NORMAL, "4/8" },
        { 5,  8, TimeSigType::NORMAL, "5/8" },
        { 6,  8, TimeSigType::NORMAL, "6/8" },
        { 7,  8, TimeSigType::NORMAL, "7/8" },
        { 9,  8, TimeSigType::NORMAL, "9/8" },
        { 12, 8, TimeSigType::NORMAL, "12/8" },
        { 4,  4, TimeSigType::FOUR_FOUR,  QT_TRANSLATE_NOOP("symUserNames", "Common time") },
        { 2,  2, TimeSigType::ALLA_BREVE, QT_TRANSLATE_NOOP("symUserNames", "Cut time") },
        { 2,  2, TimeSigType::NORMAL, "2/2" },
        { 3,  2, TimeSigType::NORMAL, "3/2" }
    };

    static std::vector<TS> masterTimeSignatureList = {
        { 2,  4, TimeSigType::NORMAL, "2/4" },
        { 3,  4, TimeSigType::NORMAL, "3/4" },
        { 4,  4, TimeSigType::NORMAL, "4/4" },
        { 5,  4, TimeSigType::NORMAL, "5/4" },
        { 6,  4, TimeSigType::NORMAL, "6/4" },
        { 3,  8, TimeSigType::NORMAL, "3/8" },
        { 4,  8, TimeSigType::NORMAL, "4/8" },
        { 5,  8, TimeSigType::NORMAL, "5/8" },
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
        { 9,  8, TimeSigType::CUT_TRIPLE, QT_TRANSLATE_NOOP("symUserNames", "Cut triple time (9/8)") }
    };

    for (TS timeSignatureType : defaultPalette ? defaultTimeSignatureList : masterTimeSignatureList) {
        auto timeSignature = Factory::makeTimeSig(gpaletteScore->dummy()->segment());
        timeSignature->setSig(Fraction(timeSignatureType.numerator, timeSignatureType.denominator), timeSignatureType.type);
        sp->appendElement(timeSignature, timeSignatureType.name);
    }

    return sp;
}

PalettePtr PaletteCreator::newFretboardDiagramPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::FretboardDiagram);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fretboard diagrams"));
    sp->setGridSize(42, 45);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto fret = FretDiagram::createFromString(gpaletteScore, "X32O1O");
    fret->setHarmony("C");
    sp->appendElement(fret, "C");
    fret = FretDiagram::createFromString(gpaletteScore, "X-554-");
    fret->setHarmony("Cm");
    sp->appendElement(fret, "Cm");
    fret = FretDiagram::createFromString(gpaletteScore, "X3231O");
    fret->setHarmony("C7");
    sp->appendElement(fret, "C7");

    fret = FretDiagram::createFromString(gpaletteScore, "XXO232");
    fret->setHarmony("D");
    sp->appendElement(fret, "D");
    fret = FretDiagram::createFromString(gpaletteScore, "XXO231");
    fret->setHarmony("Dm");
    sp->appendElement(fret, "Dm");
    fret = FretDiagram::createFromString(gpaletteScore, "XXO212");
    fret->setHarmony("D7");
    sp->appendElement(fret, "D7");

    fret = FretDiagram::createFromString(gpaletteScore, "O221OO");
    fret->setHarmony("E");
    sp->appendElement(fret, "E");
    fret = FretDiagram::createFromString(gpaletteScore, "O22OOO");
    fret->setHarmony("Em");
    sp->appendElement(fret, "Em");
    fret = FretDiagram::createFromString(gpaletteScore, "O2O1OO");
    fret->setHarmony("E7");
    sp->appendElement(fret, "E7");

    fret = FretDiagram::createFromString(gpaletteScore, "-332--");
    fret->setHarmony("F");
    sp->appendElement(fret, "F");
    fret = FretDiagram::createFromString(gpaletteScore, "-33---");
    fret->setHarmony("Fm");
    sp->appendElement(fret, "Fm");
    fret = FretDiagram::createFromString(gpaletteScore, "-3-2--");
    fret->setHarmony("F7");
    sp->appendElement(fret, "F7");

    fret = FretDiagram::createFromString(gpaletteScore, "32OOO3");
    fret->setHarmony("G");
    sp->appendElement(fret, "G");
    fret = FretDiagram::createFromString(gpaletteScore, "-55---");
    fret->setHarmony("Gm");
    sp->appendElement(fret, "Gm");
    fret = FretDiagram::createFromString(gpaletteScore, "32OOO1");
    fret->setHarmony("G7");
    sp->appendElement(fret, "G7");

    fret = FretDiagram::createFromString(gpaletteScore, "XO222O");
    fret->setHarmony("A");
    sp->appendElement(fret, "A");
    fret = FretDiagram::createFromString(gpaletteScore, "XO221O");
    fret->setHarmony("Am");
    sp->appendElement(fret, "Am");
    fret = FretDiagram::createFromString(gpaletteScore, "XO2O2O");
    fret->setHarmony("A7");
    sp->appendElement(fret, "A7");

    fret = FretDiagram::createFromString(gpaletteScore, "X-444-");
    fret->setHarmony("B");
    sp->appendElement(fret, "B");
    fret = FretDiagram::createFromString(gpaletteScore, "X-443-");
    fret->setHarmony("Bm");
    sp->appendElement(fret, "Bm");
    fret = FretDiagram::createFromString(gpaletteScore, "X212O2");
    fret->setHarmony("B7");
    sp->appendElement(fret, "B7");

    return sp;
}
