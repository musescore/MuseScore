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

#include "palettecreator.h"

#include "translation.h"

#include "engraving/types/typesconv.h"
#include "engraving/types/symnames.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/actionicon.h"
#include "engraving/dom/ambitus.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/bagpembell.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/capo.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/hammeronpulloff.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/letring.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurenumber.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/palmmute.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spacer.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stringtunings.h"
#include "engraving/dom/systemtext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tremolobar.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/vibrato.h"
#include "engraving/dom/volta.h"

using namespace mu;
using namespace mu::palette;
using namespace mu::engraving;

// Default wrapper...
template<class C>
struct makeElementImplWrapper {
    template<typename ... Args>
    static std::shared_ptr<C> makeElementImpl(mu::engraving::Score* score, Args&&... args)
    {
        return std::make_shared<C>(score->dummy(), std::forward<Args>(args)...);
    }
};

// Specialised wrappers...
#define IMPL_WRAPPER(T, P) \
    template<> \
    struct makeElementImplWrapper<T> { \
        template<typename ... Args> \
        static std::shared_ptr<T> makeElementImpl(mu::engraving::Score * score, Args && ... args) \
        { \
            return std::make_shared<T>(P, std::forward<Args>(args)...); \
        } \
    }; \

IMPL_WRAPPER(Dynamic, score->dummy()->segment())
IMPL_WRAPPER(MeasureRepeat, score->dummy()->segment())
IMPL_WRAPPER(Hairpin, score->dummy()->segment())
IMPL_WRAPPER(SystemText, score->dummy()->segment())
IMPL_WRAPPER(TempoText, score->dummy()->segment())
IMPL_WRAPPER(StaffText, score->dummy()->segment())
IMPL_WRAPPER(Expression, score->dummy()->segment())
IMPL_WRAPPER(PlayTechAnnotation, score->dummy()->segment())
IMPL_WRAPPER(Capo, score->dummy()->segment())
IMPL_WRAPPER(StringTunings, score->dummy()->segment())
IMPL_WRAPPER(RehearsalMark, score->dummy()->segment())

IMPL_WRAPPER(Jump, score->dummy()->measure())
IMPL_WRAPPER(MeasureNumber, score->dummy()->measure())

IMPL_WRAPPER(Fingering, score->dummy()->note())
IMPL_WRAPPER(NoteHead, score->dummy()->note())

// Dispatcher method ...
template<class C, typename ... Args>
std::shared_ptr<C> makeElement(mu::engraving::Score* score, Args&&... args)
{
    return makeElementImplWrapper<C>::makeElementImpl(score, std::forward<Args>(args)...);
}

PaletteTreePtr PaletteCreator::newMasterPaletteTree()
{
    PaletteTreePtr tree = std::make_shared<PaletteTree>();

    tree->append(newClefsPalette());
    tree->append(newKeySigPalette());
    tree->append(newTimePalette());
    tree->append(newTempoPalette());
    tree->append(newPitchPalette());
    tree->append(newAccidentalsPalette());
    tree->append(newDynamicsPalette());
    tree->append(newArticulationsPalette());
    tree->append(newTextPalette());
    tree->append(newKeyboardPalette());
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
    tree->append(newHarpPalette());
    tree->append(newGuitarPalette());
    tree->append(newFingeringPalette());
    tree->append(newFretboardDiagramPalette());
    tree->append(newAccordionPalette());
    tree->append(newBagpipeEmbellishmentPalette());
    tree->append(newBeamPalette());
    tree->append(newLinesPalette());

    return tree;
}

PaletteTreePtr PaletteCreator::newDefaultPaletteTree()
{
    PaletteTreePtr defaultPalette = std::make_shared<PaletteTree>();

    defaultPalette->append(newClefsPalette(true));
    defaultPalette->append(newKeySigPalette());
    defaultPalette->append(newTimePalette(true));
    defaultPalette->append(newTempoPalette(true));
    defaultPalette->append(newPitchPalette(true));
    defaultPalette->append(newAccidentalsPalette(true));
    defaultPalette->append(newDynamicsPalette(true));
    defaultPalette->append(newArticulationsPalette(true));
    defaultPalette->append(newTextPalette(true));
    defaultPalette->append(newKeyboardPalette());
    defaultPalette->append(newRepeatsPalette(true));
    defaultPalette->append(newBarLinePalette(true));
    defaultPalette->append(newLayoutPalette(true));
    defaultPalette->append(newBracketsPalette());
    defaultPalette->append(newOrnamentsPalette(true));
    defaultPalette->append(newBreathPalette(true));
    defaultPalette->append(newGraceNotePalette());
    defaultPalette->append(newNoteHeadsPalette());
    defaultPalette->append(newArpeggioPalette());
    defaultPalette->append(newTremoloPalette());
    defaultPalette->append(newHarpPalette());
    defaultPalette->append(newGuitarPalette(true));
    defaultPalette->append(newFingeringPalette(true));
    defaultPalette->append(newFretboardDiagramPalette(true));
    defaultPalette->append(newAccordionPalette());
    defaultPalette->append(newBagpipeEmbellishmentPalette());
    defaultPalette->append(newBeamPalette());
    defaultPalette->append(newLinesPalette(true));

    return defaultPalette;
}

PalettePtr PaletteCreator::newBeamPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Beam);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Beam properties"));
    sp->setGridSize(35, 33);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    sp->appendActionIcon(ActionIconType::BEAM_AUTO, "beam-auto")->mag = 1.4;
    sp->appendActionIcon(ActionIconType::BEAM_NONE, "beam-none")->mag = 1.4;
    sp->appendActionIcon(ActionIconType::BEAM_BREAK_LEFT, "beam-break-left")->mag = 1.4;
    sp->appendActionIcon(ActionIconType::BEAM_BREAK_INNER_8TH, "beam-break-inner-8th")->mag = 1.4;
    sp->appendActionIcon(ActionIconType::BEAM_BREAK_INNER_16TH, "beam-break-inner-16th")->mag = 1.4;
    sp->appendActionIcon(ActionIconType::BEAM_JOIN, "beam-join")->mag = 1.4;

    sp->appendActionIcon(ActionIconType::BEAM_FEATHERED_DECELERATE, "beam-feathered-decelerate");
    sp->appendActionIcon(ActionIconType::BEAM_FEATHERED_ACCELERATE, "beam-feathered-accelerate");

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
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfp",
        "rfz", "rf", "fz"
    };

    static const std::vector<const char*> masterDynamics = {
        "pppppp", "ppppp", "pppp",
        "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
        "ffff", "fffff", "ffffff",
        "fp", "pf", "sf", "sfz", "sff", "sffz", "sfff", "sfffz",
        "sfp", "sfpp", "rfz", "rf", "fz", "m", "r", "s", "z", "n"
    };

    for (const char* dynamicType : defaultPalette ? defaultDynamics : masterDynamics) {
        auto dynamic = makeElement<Dynamic>(mu::engraving::gpaletteScore);
        dynamic->setDynamicType(String::fromAscii(dynamicType));
        sp->appendElement(dynamic, TConv::userName(dynamic->dynamicType()));
    }

    static const std::vector<HairpinType> hairpins {
        HairpinType::CRESC_LINE,
        HairpinType::DECRESC_LINE,
        HairpinType::CRESC_HAIRPIN,
        HairpinType::DECRESC_HAIRPIN,
    };

    const qreal w = gpaletteScore->style().spatium() * 8;

    for (HairpinType hairpinType : hairpins) {
        auto hairpin = Factory::makeHairpin(gpaletteScore->dummy());
        hairpin->setHairpinType(hairpinType);
        hairpin->setLen(w);
        qreal mag = (hairpinType == HairpinType::CRESC_LINE || hairpinType == HairpinType::DECRESC_LINE) ? 1 : 0.9;
        const QPointF offset = (hairpinType == HairpinType::CRESC_LINE || hairpinType == HairpinType::DECRESC_LINE)
                               ? QPointF(1, 0.25) : QPointF(0, 0);
        sp->appendElement(hairpin, hairpin->subtypeUserName(), mag, offset);
    }

    return sp;
}

PalettePtr PaletteCreator::newKeySigPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::KeySig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Key signatures"));
    sp->setMag(0.8);
    sp->setGridSize(56, 47);
    sp->setDrawGrid(true);
    sp->setYOffset(1.0);

    for (int i = 0; i < 7; ++i) {
        auto k = Factory::makeKeySig(mu::engraving::gpaletteScore->dummy()->segment());
        k->setKey(Key(i + 1));
        sp->appendElement(k, TConv::userName(Key(i + 1)));
    }
    for (int i = -7; i < 0; ++i) {
        auto k = Factory::makeKeySig(mu::engraving::gpaletteScore->dummy()->segment());
        k->setKey(Key(i));
        sp->appendElement(k, TConv::userName(Key(i)));
    }
    auto k = Factory::makeKeySig(mu::engraving::gpaletteScore->dummy()->segment());
    k->setKey(Key::C);
    sp->appendElement(k, TConv::userName(Key::C));

    // atonal key signature
    KeySigEvent nke;
    nke.setConcertKey(Key::C);
    nke.setCustom(true);
    nke.setMode(KeyMode::NONE);
    auto nk = Factory::makeKeySig(mu::engraving::gpaletteScore->dummy()->segment());
    nk->setKeySigEvent(nke);
    sp->appendElement(nk, TConv::userName(Key::C, true));

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
        if (ac->symId() != SymId::noSym) {
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
    for (const BarLineTableItem& bti : BarLine::barLineTable) {
        auto b = Factory::makeBarLine(gpaletteScore->dummy()->segment());
        b->setBarLineType(bti.type);
        sp->appendElement(b, bti.userName);
    }

    // bar line spans
    if (!defaultPalette) {
        const struct {
            int from, to;
            muse::TranslatableString userName;
        } spans[] = {
            { BARLINE_SPAN_TICK1_FROM,  BARLINE_SPAN_TICK1_TO,  SymNames::userNameForSymId(SymId::barlineTick) },
            { BARLINE_SPAN_TICK2_FROM,  BARLINE_SPAN_TICK2_TO,  muse::TranslatableString("engraving/sym", "Tick barline 2") },  // Not in SMuFL
            { BARLINE_SPAN_SHORT1_FROM, BARLINE_SPAN_SHORT1_TO, SymNames::userNameForSymId(SymId::barlineShort) },
            { BARLINE_SPAN_SHORT2_FROM, BARLINE_SPAN_SHORT2_TO, muse::TranslatableString("engraving/sym", "Short barline 2") }, // Not in SMuFL
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
    sp->setMag(0.75);
    sp->setGridSize(100, 28);
    sp->setDrawGrid(true);

    struct MeasureRepeatInfo
    {
        SymId id = SymId::noSym;
        int measuresCount = 0;
    };

    const std::vector<MeasureRepeatInfo> defaultMeasureRepeats {
        { SymId::repeat1Bar, 1 }
    };

    const std::vector<MeasureRepeatInfo> masterMeasureRepeats {
        { SymId::repeat1Bar, 1 },
        { SymId::repeat2Bars, 2 },
        { SymId::repeat4Bars, 4 }
    };

    for (MeasureRepeatInfo repeat: defaultPalette ? defaultMeasureRepeats : masterMeasureRepeats) {
        std::shared_ptr<MeasureRepeat> rm = makeElement<MeasureRepeat>(gpaletteScore);
        rm->mutldata()->symId = repeat.id;
        rm->setNumMeasures(repeat.measuresCount);
        sp->appendElement(rm, SymNames::userNameForSymId(repeat.id));
    }

    const std::vector<MarkerType> defaultMarkers = {
        MarkerType::SEGNO, MarkerType::CODA, MarkerType::FINE, MarkerType::TOCODA
    };

    const std::vector<MarkerType> allMarkers = {
        MarkerType::SEGNO,
        MarkerType::VARSEGNO,
        MarkerType::CODA,
        MarkerType::VARCODA,
        MarkerType::CODETTA,
        MarkerType::FINE,
        MarkerType::TOCODA,
        MarkerType::TOCODASYM,
        MarkerType::DA_CODA,
        MarkerType::DA_DBLCODA,
        MarkerType::USER
    };

    for (MarkerType markerType : (defaultPalette ? defaultMarkers : allMarkers)) {
        if (markerType == MarkerType::CODETTA || markerType == MarkerType::USER) {// Codetta not in SMuFL
            continue;
        }

        auto mk = makeElement<Marker>(gpaletteScore);
        mk->setMarkerType(markerType);
        mk->styleChanged();
        sp->appendElement(mk, TConv::userName(markerType));
    }

    const std::vector<JumpTypeTableItem> defaultJumpTypeTable {
        { JumpType::DC,         "D.C.",         "start", "end",  "" },
        { JumpType::DC_AL_FINE, "D.C. al Fine", "start", "fine", "" },
        { JumpType::DC_AL_CODA, "D.C. al Coda", "start", "coda", "codab" },
        { JumpType::DS_AL_CODA, "D.S. al Coda", "segno", "coda", "codab" },
        { JumpType::DS_AL_FINE, "D.S. al Fine", "segno", "fine", "" },
        { JumpType::DS,         "D.S.",         "segno", "end",  "" },
    };

    for (const JumpTypeTableItem& item : (defaultPalette ? defaultJumpTypeTable : jumpTypeTable)) {
        auto jp = makeElement<Jump>(gpaletteScore);
        jp->setJumpType(item.type);
        sp->appendElement(jp, TConv::userName(item.type));
    }

    for (const BarLineTableItem& bti : BarLine::barLineTable) {
        switch (bti.type) {
        case BarLineType::START_REPEAT:
        case BarLineType::END_REPEAT:
        case BarLineType::END_START_REPEAT:
            break;
        default:
            continue;
        }

        auto b = Factory::makeBarLine(gpaletteScore->dummy()->segment());
        b->setBarLineType(bti.type);
        PaletteCellPtr cell = sp->appendElement(b, bti.userName);
        cell->drawStaff = false;
    }

    qreal w = gpaletteScore->style().spatium() * 8;
    auto volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText(u"1.");
    std::vector<int> il;
    il.push_back(1);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Prima volta"));

    volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::OPEN);
    volta->setLen(w);
    volta->setText(u"2.");
    il.clear();
    il.push_back(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta, open"));

    volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText(u"2.");
    il.clear();
    il.push_back(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta"));

    volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText(u"3.");
    il.clear();
    il.push_back(3);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Terza volta"));

    if (!defaultPalette) {
        auto vibrato = makeElement<Vibrato>(gpaletteScore);
        vibrato->setVibratoType(VibratoType::VIBRATO_SAWTOOTH);
        vibrato->setLen(w);
        sp->appendElement(vibrato, TConv::userName(VibratoType::VIBRATO_SAWTOOTH));

        vibrato = makeElement<Vibrato>(gpaletteScore);
        vibrato->setVibratoType(VibratoType::VIBRATO_SAWTOOTH_WIDE);
        vibrato->setLen(w);
        sp->appendElement(vibrato, TConv::userName(VibratoType::VIBRATO_SAWTOOTH_WIDE));
    }

    return sp;
}

PalettePtr PaletteCreator::newLayoutPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Layout);
    //: The name of a palette
    sp->setName(QT_TRANSLATE_NOOP("palette", "Layout"));
    sp->setGridSize(42, 36);
    sp->setDrawGrid(true);

    static const std::vector<LayoutBreakType> layoutBreaks  {
        LayoutBreakType::LINE,
        LayoutBreakType::PAGE,
        LayoutBreakType::SECTION,
    };
    for (LayoutBreakType layoutBreakType : layoutBreaks) {
        auto lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
        lb->setLayoutBreakType(layoutBreakType);
        sp->appendElement(lb, TConv::userName(layoutBreakType));
    }

    if (!defaultPalette) {
        auto lb = Factory::makeLayoutBreak(gpaletteScore->dummy()->measure());
        lb->setLayoutBreakType(LayoutBreakType::NOBREAK);
        sp->appendElement(lb, TConv::userName(LayoutBreakType::NOBREAK));
    }

    sp->appendActionIcon(ActionIconType::SYSTEM_LOCK, "toggle-system-lock");

    static const std::vector<SpacerType> spacers  {
        SpacerType::DOWN,
        SpacerType::UP,
        SpacerType::FIXED
    };
    for (SpacerType spacerType : spacers) {
        auto spacer = Factory::makeSpacer(gpaletteScore->dummy()->measure());
        spacer->setSpacerType(spacerType);
        spacer->setGap(Spatium(3));
        PaletteCellPtr cell = sp->appendElement(spacer, spacer->subtypeUserName());
        cell->mag = .7;
    }

    sp->appendActionIcon(ActionIconType::VFRAME, "insert-vbox");
    sp->appendActionIcon(ActionIconType::HFRAME, "insert-hbox");
    sp->appendActionIcon(ActionIconType::TFRAME, "insert-textframe");
    sp->appendActionIcon(ActionIconType::FFRAME, "insert-fretframe");
    sp->appendActionIcon(ActionIconType::STAFF_TYPE_CHANGE, "insert-staff-type-change");
    sp->appendActionIcon(ActionIconType::MEASURE, "insert-measure");

    return sp;
}

PalettePtr PaletteCreator::newFingeringPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Fingering);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fingerings"));
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
    finger = defaultPalette ? "123456" : "0123456789";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->initTextStyleType(TextStyleType::STRING_NUMBER);
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
        sp->appendElement(s, s->subtypeUserName());
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

    for (int i = int(TremoloType::R8); i <= int(TremoloType::BUZZ_ROLL); ++i) {
        auto tremolo = Factory::makeTremoloSingleChord(gpaletteScore->dummy()->chord());
        tremolo->setTremoloType(TremoloType(i));
        sp->appendElement(tremolo, tremolo->subtypeUserName());
    }

    for (int i = int(TremoloType::C8); i <= int(TremoloType::C64); ++i) {
        auto tremolo = Factory::makeTremoloTwoChord(gpaletteScore->dummy()->chord());
        tremolo->setTremoloType(TremoloType(i));
        sp->appendElement(tremolo, tremolo->subtypeUserName());
    }

    static const SymIdList dots {
        SymId::tremoloDivisiDots2,
        SymId::tremoloDivisiDots3,
        SymId::tremoloDivisiDots4,
        SymId::tremoloDivisiDots6
    };
    // include additional symbol-based tremolo articulations, implemented as articulations
    for (auto i : dots) {
        auto s = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        s->setSymId(i);
        sp->appendElement(s, s->subtypeUserName());
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
        sp->appendElement(nh, TConv::userName((NoteHeadGroup(i))));
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

    auto slur = Factory::makeSlur(gpaletteScore->dummy());
    sp->appendElement(slur, QT_TRANSLATE_NOOP("palette", "Slur"));

    // do not include additional symbol-based fingerings (temporarily?) implemented as articulations
    static const SymIdList defaultArticulations {
        SymId::articAccentAbove,
        SymId::articStaccatoAbove,
        SymId::articStaccatissimoAbove,
        SymId::articTenutoAbove,
        SymId::articTenutoStaccatoAbove,
        SymId::articMarcatoAbove,
        SymId::articAccentStaccatoAbove,
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
        SymId::pictHalfOpen2,
        // SymId::stringsThumbPosition,
        // SymId::luteFingeringRHThumb,
        // SymId::luteFingeringRHFirst,
        // SymId::luteFingeringRHSecond,
        // SymId::luteFingeringRHThird,
    };

    for (SymId articulationType : defaultPalette ? defaultArticulations : masterArticulations) {
        auto artic = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        artic->setSymId(articulationType);
        sp->appendElement(artic, artic->subtypeUserName());
    }

    if (!defaultPalette) {
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
    sp->setGridSize(73, 27);
    sp->setMag(1);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    static const SymIdList defaultOrnaments {
        SymId::ornamentTurn,
        SymId::ornamentTurnInverted,
        SymId::ornamentTrill,
        SymId::ornamentShortTrill,
        SymId::ornamentMordent
    };

    static const SymIdList masterOrnaments {
        SymId::ornamentTurn,
        SymId::ornamentTurnInverted,
        SymId::ornamentTurnSlash,
        SymId::ornamentTurnUp,
        SymId::ornamentTurnUpS,
        SymId::ornamentTrill,
        SymId::ornamentShortTrill,
        SymId::ornamentMordent,
        SymId::ornamentHaydn,
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
        SymId::ornamentShake3,
        SymId::ornamentShakeMuffat1,
        SymId::ornamentTremblementCouperin,
        SymId::ornamentPinceCouperin
    };

    for (auto ornamentType : defaultPalette ? defaultOrnaments : masterOrnaments) {
        auto ornament = Factory::makeOrnament(gpaletteScore->dummy()->chord());
        ornament->setSymId(ornamentType);
        qreal mag = ornament->symId() == SymId::ornamentTrill ? 1.0 : 1.2;
        sp->appendElement(ornament, ornament->subtypeUserName(), mag);
    }

    static const std::vector<TrillType> trillTypes = {
        TrillType::TRILL_LINE, TrillType::UPPRALL_LINE, TrillType::DOWNPRALL_LINE, TrillType::PRALLPRALL_LINE,
    };

    for (TrillType trillType : trillTypes) {
        auto trill = makeElement<Trill>(gpaletteScore);
        trill->setTrillType(trillType);
        trill->setLen(gpaletteScore->style().spatium() * 8);

        qreal mag = (trillType == TrillType::TRILL_LINE || trillType == TrillType::PRALLPRALL_LINE) ? 1.0 : 0.8;
        sp->appendElement(trill, TConv::userName(trillType), mag);
    }

    return sp;
}

PalettePtr PaletteCreator::newAccordionPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Accordion);
    //: The name of a palette
    sp->setName(QT_TRANSLATE_NOOP("palette", "Accordion"));
    sp->setGridSize(42, 25);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    static const SymIdList art {
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
        sp->appendElement(s, SymNames::userNameForSymId(i));
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

    const std::array<std::pair<BracketType, const char*>, 4> types { {
        { BracketType::NORMAL, QT_TRANSLATE_NOOP("palette", "Bracket") },
        { BracketType::BRACE,  QT_TRANSLATE_NOOP("palette", "Brace") },
        { BracketType::SQUARE, QT_TRANSLATE_NOOP("palette", "Square") },
        { BracketType::LINE,   QT_TRANSLATE_NOOP("palette", "Line") }
    } };

    static Part* bracketItemOwnerPart = new Part(gpaletteScore);
    static Staff* bracketItemOwner = Factory::createStaff(bracketItemOwnerPart);
    bracketItemOwner->setBracketType(types.size() - 1, BracketType::NORMAL);

    for (size_t i = 0; i < types.size(); ++i) {
        auto b1 = Factory::makeBracket(gpaletteScore->dummy());
        auto bi1 = bracketItemOwner->brackets()[i];
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
        auto f = Factory::makeFermata(gpaletteScore->dummy()->segment());
        f->setSymIdAndTimeStretch(i);
        sp->appendElement(f, f->subtypeUserName());
    }

    for (BreathType breath : Breath::BREATH_LIST) {
        if ((breath.id == SymId::chantCaesura || breath.id == SymId::caesuraSingleStroke) && defaultPalette) {
            continue;
        }
        auto a = Factory::makeBreath(gpaletteScore->dummy()->segment());
        a->setSymId(breath.id);
        a->setPause(breath.pause);
        sp->appendElement(a, SymNames::userNameForSymId(breath.id));
    }

    return sp;
}

PalettePtr PaletteCreator::newArpeggioPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Arpeggio);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Arpeggios & glissandos"));
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
        if (a->glissandoType() != a->style().styleV(Sid::glissandoType).value<GlissandoType>()) {
            a->setPropertyFlags(Pid::GLISS_TYPE, PropertyFlags::UNSTYLED);
        }
        sp->appendElement(a, a->glissandoTypeName());
    }

    //fall and doits

    static const std::vector<ChordLineType> chordLineTypes {
        ChordLineType::FALL,
        ChordLineType::DOIT,
        ChordLineType::PLOP,
        ChordLineType::SCOOP
    };

    for (ChordLineType chordLineType : chordLineTypes) {
        auto cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
        cl->setChordLineType(chordLineType);
        sp->appendElement(cl, cl->chordLineTypeName());
    }

    for (ChordLineType chordLineType : chordLineTypes) {
        auto cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
        cl->setChordLineType(chordLineType);
        cl->setStraight(true);
        sp->appendElement(cl, cl->chordLineTypeName());
    }

    for (ChordLineType chordLineType : chordLineTypes) {
        auto cl = Factory::makeChordLine(gpaletteScore->dummy()->chord());
        cl->setChordLineType(chordLineType);
        cl->setStraight(true);
        cl->setWavy(true);
        sp->appendElement(cl, cl->chordLineTypeName());
    }
    return sp;
}

PalettePtr PaletteCreator::newClefsPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Clef);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Clefs"));
    sp->setMag(0.8);
    sp->setGridSize(36, 55);
    sp->setDrawGrid(true);
    sp->setYOffset(1.0);

    static const std::vector<ClefType> clefsDefault  {
        ClefType::G, ClefType::G8_VA, ClefType::G15_MA, ClefType::G8_VB, ClefType::C3,
        ClefType::C4, ClefType::F, ClefType::F_8VA,
        ClefType::F8_VB, ClefType::PERC, ClefType::TAB, ClefType::TAB4
    };
    static const std::vector<ClefType> clefsMaster  {
        ClefType::G, ClefType::G8_VA, ClefType::G15_MA, ClefType::G8_VB, ClefType::G15_MB, ClefType::G8_VB_O,
        ClefType::G8_VB_C, ClefType::G8_VB_P, ClefType::G_1, ClefType::C1, ClefType::C2, ClefType::C3,
        ClefType::C4, ClefType::C4_8VB, ClefType::C5, ClefType::C_19C, ClefType::C1_F18C, ClefType::C3_F18C, ClefType::C4_F18C,
        ClefType::C1_F20C, ClefType::C3_F20C, ClefType::C4_F20C,
        ClefType::F, ClefType::F_8VA, ClefType::F_15MA,
        ClefType::F8_VB, ClefType::F15_MB, ClefType::F_B, ClefType::F_C, ClefType::F_F18C, ClefType::F_19C,
        ClefType::PERC,
        ClefType::PERC2, ClefType::TAB, ClefType::TAB4, ClefType::TAB_SERIF, ClefType::TAB4_SERIF
    };

    for (ClefType clefType : defaultPalette ? clefsDefault : clefsMaster) {
        auto clef = Factory::makeClef(gpaletteScore->dummy()->segment());
        clef->setClefType(ClefTypeList(clefType, clefType));
        sp->appendElement(clef, TConv::userName(clefType));
    }
    return sp;
}

PalettePtr PaletteCreator::newGraceNotePalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::GraceNote);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Grace notes"));
    sp->setMag(1.35);
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
    sp->setGridSize(55, 72);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    for (size_t i = 0; i < TConv::embellishmentsCount(); ++i) {
        auto b = makeElement<BagpipeEmbellishment>(gpaletteScore);
        b->setEmbelType(EmbellishmentType(i));
        // TODO: pass TranslatableString
        sp->appendElement(b, TConv::userName(static_cast<EmbellishmentType>(i)).str);
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
    sp->setVisible(false);

    qreal w = gpaletteScore->style().spatium() * 8;

    auto slur = Factory::makeSlur(gpaletteScore->dummy());
    sp->appendElement(slur, QT_TRANSLATE_NOOP("palette", "Slur"));

    static const std::vector<HairpinType> hairpins {
        HairpinType::CRESC_HAIRPIN,
        HairpinType::DECRESC_HAIRPIN,
        HairpinType::CRESC_LINE,
        HairpinType::DECRESC_LINE
    };

    for (HairpinType hairpinType : hairpins) {
        auto hairpin = Factory::makeHairpin(gpaletteScore->dummy());
        hairpin->setHairpinType(hairpinType);
        hairpin->setLen(w);
        sp->appendElement(hairpin, hairpin->subtypeUserName());
    }

    auto gabel = Factory::makeHairpin(gpaletteScore->dummy());
    gabel->setHairpinType(HairpinType::CRESC_HAIRPIN);
    gabel->setBeginText(u"<sym>dynamicMezzo</sym><sym>dynamicForte</sym>");
    gabel->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    gabel->setBeginTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    gabel->setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, PropertyFlags::UNSTYLED);
    gabel->setLen(w);
    // Keep name in sync with PaletteCell::translationContext()
    sp->appendElement(gabel, QT_TRANSLATE_NOOP("palette", "Dynamic + hairpin"));

    auto volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::CLOSED);
    volta->setLen(w);
    volta->setText(u"1.");
    std::vector<int> il;
    il.push_back(1);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Prima volta"));

    if (!defaultPalette) {
        volta = makeElement<Volta>(gpaletteScore);
        volta->setVoltaType(Volta::Type::CLOSED);
        volta->setLen(w);
        volta->setText(u"2.");
        il.clear();
        il.push_back(2);
        volta->setEndings(il);
        sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta"));

        volta = makeElement<Volta>(gpaletteScore);
        volta->setVoltaType(Volta::Type::CLOSED);
        volta->setLen(w);
        volta->setText(u"3.");
        il.clear();
        il.push_back(3);
        volta->setEndings(il);
        sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Terza volta"));
    }

    volta = makeElement<Volta>(gpaletteScore);
    volta->setVoltaType(Volta::Type::OPEN);
    volta->setLen(w);
    volta->setText(u"2.");
    il.clear();
    il.push_back(2);
    volta->setEndings(il);
    sp->appendElement(volta, QT_TRANSLATE_NOOP("palette", "Seconda volta, open"));

    static const std::vector<OttavaType> ottavasDefault {
        OttavaType::OTTAVA_8VA,
        OttavaType::OTTAVA_8VB,
        OttavaType::OTTAVA_15MA
    };
    static const std::vector<OttavaType> ottavasMaster {
        OttavaType::OTTAVA_8VA,
        OttavaType::OTTAVA_8VB,
        OttavaType::OTTAVA_15MA,
        OttavaType::OTTAVA_15MB,
        OttavaType::OTTAVA_22MA,
        OttavaType::OTTAVA_22MB
    };
    for (OttavaType ottavaType : defaultPalette ? ottavasDefault : ottavasMaster) {
        auto ottava = makeElement<Ottava>(gpaletteScore);
        ottava->setOttavaType(ottavaType);
        ottava->setLen(w);
        ottava->styleChanged();
        sp->appendElement(ottava, ottava->subtypeUserName());
    }

    auto pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setEndHookType(HookType::HOOK_90);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and line)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setLineVisible(false);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and asterisk)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_90);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (straight hooks)"));

    if (!defaultPalette) {
        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_90);
        pedal->setEndHookType(HookType::HOOK_45);
        pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
        pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
        pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled end hook)"));

        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_45);
        pedal->setEndHookType(HookType::HOOK_45);
        pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
        pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
        pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (both hooks angled)"));

        pedal = makeElement<Pedal>(gpaletteScore);
        pedal->setLen(w);
        pedal->setBeginHookType(HookType::HOOK_45);
        pedal->setEndHookType(HookType::HOOK_90);
        pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
        pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
        pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
        sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled start hook)"));
    }

    static const std::vector<TrillType> trillTypes = {
        TrillType::TRILL_LINE, TrillType::UPPRALL_LINE, TrillType::DOWNPRALL_LINE, TrillType::PRALLPRALL_LINE,
    };

    static const std::vector<TrillType> defaultTrills = {
        { TrillType::TRILL_LINE, TrillType::DOWNPRALL_LINE, TrillType::PRALLPRALL_LINE }
    };

    for (TrillType trillType : defaultPalette ? defaultTrills : trillTypes) {
        auto trill = makeElement<Trill>(gpaletteScore);
        trill->setTrillType(trillType);
        trill->setLen(w);
        sp->appendElement(trill, TConv::userName(trillType));
    }

    auto staffTextLine = makeElement<TextLine>(gpaletteScore);
    staffTextLine->setLen(w);
    staffTextLine->setBeginText(u"Staff");
    staffTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(staffTextLine, QT_TRANSLATE_NOOP("palette", "Staff text line"));

    auto systemTextLine = makeElement<TextLine>(gpaletteScore, true);
    systemTextLine->setLen(w * 1.5);
    systemTextLine->setBeginText(u"System");
    systemTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(systemTextLine, QT_TRANSLATE_NOOP("palette", "System text line"));

    if (!defaultPalette) {
        auto textLine = makeElement<TextLine>(gpaletteScore);
        textLine->setLen(w);
        textLine->setBeginText(u"VII");
        textLine->setEndHookType(HookType::HOOK_90);
        sp->appendElement(textLine, QT_TRANSLATE_NOOP("palette", "Text line"));
    }

    auto line = makeElement<TextLine>(gpaletteScore);
    line->setLen(w);
    line->setDiagonal(true);
    sp->appendElement(line, QT_TRANSLATE_NOOP("palette", "Line"));

    sp->appendActionIcon(ActionIconType::NOTE_ANCHORED_LINE, "add-noteline", 2);

    auto a = Factory::makeAmbitus(gpaletteScore->dummy()->segment());
    sp->appendElement(a, QT_TRANSLATE_NOOP("palette", "Ambitus"));

    auto letRing = makeElement<LetRing>(gpaletteScore);
    letRing->setLen(w);
    sp->appendElement(letRing, QT_TRANSLATE_NOOP("palette", "Let ring"));

    static const std::vector<VibratoType> vibratoTable = {
        { VibratoType::GUITAR_VIBRATO, VibratoType::GUITAR_VIBRATO_WIDE, VibratoType::VIBRATO_SAWTOOTH,
          VibratoType::VIBRATO_SAWTOOTH_WIDE },
    };

    static const std::vector<VibratoType> defaultVibratoTable = {
        { VibratoType::GUITAR_VIBRATO, VibratoType::VIBRATO_SAWTOOTH },
    };

    for (VibratoType vibratoType : defaultPalette ? defaultVibratoTable : vibratoTable) {
        auto vibrato = makeElement<Vibrato>(gpaletteScore);
        vibrato->setVibratoType(vibratoType);
        vibrato->setLen(w);
        sp->appendElement(vibrato, TConv::userName(vibratoType));
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
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: quarter note = dotted quarter note"),
                     3.0 / 2.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: dotted quarter note = quarter note"),
                     2.0 / 3.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteHalfUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: half note = quarter note"),
                     1.0 / 2.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteHalfUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: quarter note = half note"),
                     2.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNote8thUp</sym> = <sym>metNote8thUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: eighth note = eighth note"),
                     1.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNoteQuarterUp</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: quarter note = quarter note"),
                     1.0 / 1.0, true, false, true, false, false),
        TempoPattern("<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = <sym>metNoteQuarterUp</sym>",
                     QT_TRANSLATE_NOOP("palette", "Metric modulation: dotted eighth note = quarter note"),
                     4.0 / 3.0, true, false, true, false, false),
    };

    for (TempoPattern tp : tps) {
        auto tt = makeElement<TempoText>(gpaletteScore);
        tt->setFollowText(tp.followText);
        tt->setXmlText(tp.pattern);
        if (tp.relative) {
            tt->setRelative(tp.f);
            sp->appendElement(tt, tp.name, 1.5);
        } else if (tp.italian) {
            tt->setTempo(tp.f);
            sp->appendElement(tt, tp.name, 1.3);
        } else {
            tt->setTempo(tp.f);
            sp->appendElement(tt, tp.name, 1.5);
        }
    }

    static const std::map<GradualTempoChangeType, const char*> DEFAULT_TEMPO_CHANGE = {
        { GradualTempoChangeType::Accelerando, QT_TRANSLATE_NOOP("palette", "accel.") },
        { GradualTempoChangeType::Allargando,  QT_TRANSLATE_NOOP("palette", "allarg.") },
        { GradualTempoChangeType::Rallentando, QT_TRANSLATE_NOOP("palette", "rall.") },
        { GradualTempoChangeType::Ritardando,  QT_TRANSLATE_NOOP("palette", "rit.") },
    };

    static const std::map<GradualTempoChangeType, const char*> MASTER_TEMPO_CHANGE = {
        { GradualTempoChangeType::Accelerando,  QT_TRANSLATE_NOOP("palette", "accel.") },
        { GradualTempoChangeType::Allargando,   QT_TRANSLATE_NOOP("palette", "allarg.") },
        { GradualTempoChangeType::Calando,      QT_TRANSLATE_NOOP("palette", "calando") },
        { GradualTempoChangeType::Lentando,     QT_TRANSLATE_NOOP("palette", "lentando") },
        { GradualTempoChangeType::Morendo,      QT_TRANSLATE_NOOP("palette", "morendo") },
        { GradualTempoChangeType::Precipitando, QT_TRANSLATE_NOOP("palette", "precipitando") },
        { GradualTempoChangeType::Rallentando,  QT_TRANSLATE_NOOP("palette", "rall.") },
        { GradualTempoChangeType::Ritardando,   QT_TRANSLATE_NOOP("palette", "rit.") },
        { GradualTempoChangeType::Smorzando,    QT_TRANSLATE_NOOP("palette", "smorz.") },
        { GradualTempoChangeType::Sostenuto,    QT_TRANSLATE_NOOP("palette", "sost.") },
        { GradualTempoChangeType::Stringendo,   QT_TRANSLATE_NOOP("palette", "string.") }
    };

    for (const auto& pair : defaultPalette ? DEFAULT_TEMPO_CHANGE : MASTER_TEMPO_CHANGE) {
        auto item = makeElement<GradualTempoChange>(gpaletteScore);
        item->setTempoChangeType(pair.first);
        const String& text = String::fromUtf8(pair.second);
        item->setBeginText(text);
        item->setContinueText(String(u"(%1)").arg(text));
        sp->appendElement(item, pair.second, 1.3)->yoffset = 0.4;
    }

    const char* aTempoStr = QT_TRANSLATE_NOOP("palette", "a tempo");
    auto aTempoTxt = makeElement<TempoText>(gpaletteScore);
    aTempoTxt->setFollowText(true);
    aTempoTxt->setXmlText(aTempoStr);
    aTempoTxt->setATempo();
    sp->appendElement(aTempoTxt, aTempoStr, 1.3);

    const char* tempoPrimoStr = QT_TRANSLATE_NOOP("palette", "tempo primo");
    auto tempoPrimoTxt = makeElement<TempoText>(gpaletteScore);
    tempoPrimoTxt->setFollowText(true);
    tempoPrimoTxt->setXmlText(tempoPrimoStr);
    tempoPrimoTxt->setTempoPrimo();
    sp->appendElement(tempoPrimoTxt, tempoPrimoStr, 1.3);

    auto stxt = makeElement<SystemText>(gpaletteScore);
    stxt->initTextStyleType(TextStyleType::TEMPO);
    stxt->setXmlText(String::fromAscii(QT_TRANSLATE_NOOP("palette", "Swing")));
    stxt->setSwing(true);
    PaletteCellPtr cell = sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "Swing"), 1.3);
    cell->yoffset = 0.4;
    cell->setElementTranslated(true);

    stxt = makeElement<SystemText>(gpaletteScore);
    stxt->initTextStyleType(TextStyleType::TEMPO);
    /*: System text to switch from swing rhythm back to straight rhythm */
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "Straight"));
    // need to be true to enable the "Off" option
    stxt->setSwing(true);
    // 0 (swingUnit) turns of swing; swingRatio is set to default
    stxt->setSwingParameters(0, stxt->style().styleI(Sid::swingRatio));
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
    sp->setGridSize(100, 28);
    sp->setMag(0.85);
    sp->setDrawGrid(true);

    auto st = makeElement<StaffText>(gpaletteScore);
    st->setXmlText(QT_TRANSLATE_NOOP("palette", "Staff text"));
    sp->appendElement(st, QT_TRANSLATE_NOOP("palette", "Staff text"))->setElementTranslated(true);

    qreal w = gpaletteScore->style().spatium() * 8;
    auto staffTextLine = makeElement<TextLine>(gpaletteScore);
    staffTextLine->setLen(w);
    staffTextLine->setBeginText(u"Staff");
    staffTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(staffTextLine, QT_TRANSLATE_NOOP("palette", "Staff text line"));

    auto stxt = makeElement<SystemText>(gpaletteScore);
    stxt->setXmlText(QT_TRANSLATE_NOOP("palette", "System text"));
    sp->appendElement(stxt, QT_TRANSLATE_NOOP("palette", "System text"))->setElementTranslated(true);

    auto systemTextLine = makeElement<TextLine>(gpaletteScore, true);
    systemTextLine->setLen(w * 1.5);
    systemTextLine->setBeginText(u"System");
    systemTextLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(systemTextLine, QT_TRANSLATE_NOOP("palette", "System text line"));

    auto expressionText = makeElement<Expression>(gpaletteScore);
    expressionText->setTextStyleType(TextStyleType::EXPRESSION);
    expressionText->setXmlText(QT_TRANSLATE_NOOP("palette", "expression"));
    expressionText->setPlacement(PlacementV::BELOW);
    sp->appendElement(expressionText, QT_TRANSLATE_NOOP("palette", "Expression text"))->setElementTranslated(true);

    auto is = makeElement<InstrumentChange>(gpaletteScore);
    is->setXmlText(QT_TRANSLATE_NOOP("palette", "Change instr."));
    sp->appendElement(is, QT_TRANSLATE_NOOP("palette", "Instrument change"))->setElementTranslated(true);

    auto rhm = makeElement<RehearsalMark>(gpaletteScore);
    rhm->setXmlText("B1");
    sp->appendElement(rhm, QT_TRANSLATE_NOOP("palette", "Rehearsal mark"));

    struct PlayTechAnnotationInfo {
        const char* xmlText;
        PlayingTechniqueType playTechType;
        bool expressionType = false;
        muse::TranslatableString userName = TConv::userName(playTechType);
    };

    static const std::vector<PlayTechAnnotationInfo> playTechAnnotations = {
        { QT_TRANSLATE_NOOP("palette", "legato"),    PlayingTechniqueType::Legato },
        { QT_TRANSLATE_NOOP("palette", "pizz."),     PlayingTechniqueType::Pizzicato },
        { QT_TRANSLATE_NOOP("palette", "arco"),      PlayingTechniqueType::Natural, false, muse::TranslatableString("palette", "Arco") },
        { QT_TRANSLATE_NOOP("palette", "tremolo"),   PlayingTechniqueType::Tremolo, true },
        //: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music)
        { QT_TRANSLATE_NOOP("palette", "mute"),      PlayingTechniqueType::Mute },
        //: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music)
        { QT_TRANSLATE_NOOP("palette", "open"),      PlayingTechniqueType::Open },
        { QT_TRANSLATE_NOOP("palette", "distort"),   PlayingTechniqueType::Distortion },
        { QT_TRANSLATE_NOOP("palette", "overdrive"), PlayingTechniqueType::Overdrive },
        { QT_TRANSLATE_NOOP("palette", "harmonics"), PlayingTechniqueType::Harmonics },
        { QT_TRANSLATE_NOOP("palette", "jazz tone"), PlayingTechniqueType::JazzTone },
        { QT_TRANSLATE_NOOP("palette", "normal"),    PlayingTechniqueType::Natural },
    };

    for (PlayTechAnnotationInfo playTechAnnotation : playTechAnnotations) {
        auto pta = makeElement<PlayTechAnnotation>(gpaletteScore);
        if (playTechAnnotation.expressionType) {
            pta->setTextStyleType(TextStyleType::EXPRESSION);
        }
        pta->setXmlText(playTechAnnotation.xmlText);
        pta->setTechniqueType(playTechAnnotation.playTechType);
        sp->appendElement(pta, playTechAnnotation.userName)->setElementTranslated(true);
    }

    if (!defaultPalette) {
        // Measure numbers, unlike other elements (but like most text elements),
        // are not copied directly into the score when drop.
        // Instead, they simply set the corresponding measure's MeasureNumberMode to SHOW
        // Because of that, the element shown in the palettes does not have to have any particular formatting.
        auto meaNum = makeElement<MeasureNumber>(gpaletteScore);
        meaNum->setProperty(Pid::TEXT_STYLE, int(TextStyleType::STAFF));       // Make the element bigger in the palettes (using the default measure number style makes it too small)
        meaNum->setXmlText(QT_TRANSLATE_NOOP("palette", "Measure number"));
        sp->appendElement(meaNum, QT_TRANSLATE_NOOP("palette", "Measure number"))->setElementTranslated(true);

        static const std::vector<PlayTechAnnotationInfo> playTechAnnotationsMaster = {
            { QT_TRANSLATE_NOOP("palette", "détaché"),   PlayingTechniqueType::Detache },
            { QT_TRANSLATE_NOOP("palette", "martelé"),   PlayingTechniqueType::Martele },
            { QT_TRANSLATE_NOOP("palette", "col legno"), PlayingTechniqueType::ColLegno },
            { QT_TRANSLATE_NOOP("palette", "sul pont."), PlayingTechniqueType::SulPonticello },
            { QT_TRANSLATE_NOOP("palette", "sul tasto"), PlayingTechniqueType::SulTasto },
            { QT_TRANSLATE_NOOP("palette", "vibrato"),   PlayingTechniqueType::Vibrato },
        };

        for (PlayTechAnnotationInfo playTechAnnotation : playTechAnnotationsMaster) {
            auto pta = makeElement<PlayTechAnnotation>(gpaletteScore);
            pta->setXmlText(playTechAnnotation.xmlText);
            pta->setTechniqueType(playTechAnnotation.playTechType);
            sp->appendElement(pta, TConv::userName(playTechAnnotation.playTechType))->setElementTranslated(true);
        }
    }

    return sp;
}

PalettePtr PaletteCreator::newTimePalette(bool defaultPalette)
{
    struct TS {
        int numerator;
        int denominator;
        TimeSigType type;
        const char* name;
    };

    PalettePtr sp = std::make_shared<Palette>(Palette::Type::TimeSig);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Time signatures"));
    sp->setMag(.8);
    sp->setGridSize(42, 38);
    sp->setDrawGrid(true);

    static const std::vector<TS> defaultTimeSignatureList = {
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
        { 4,  4, TimeSigType::FOUR_FOUR,  QT_TRANSLATE_NOOP("engraving/timesig", "Common time") },
        { 2,  2, TimeSigType::ALLA_BREVE, QT_TRANSLATE_NOOP("engraving/timesig", "Cut time") },
        { 2,  2, TimeSigType::NORMAL, "2/2" },
        { 3,  2, TimeSigType::NORMAL, "3/2" }
    };

    static const std::vector<TS> masterTimeSignatureList = {
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
        { 4,  4, TimeSigType::FOUR_FOUR,  QT_TRANSLATE_NOOP("engraving/timesig", "Common time") },
        { 2,  2, TimeSigType::ALLA_BREVE, QT_TRANSLATE_NOOP("engraving/timesig", "Cut time") },
        { 2,  2, TimeSigType::NORMAL, "2/2" },
        { 3,  2, TimeSigType::NORMAL, "3/2" },
        { 4,  2, TimeSigType::NORMAL, "4/2" },
        { 2,  2, TimeSigType::CUT_BACH,   QT_TRANSLATE_NOOP("engraving/timesig", "Cut time (Bach)") },
        { 9,  8, TimeSigType::CUT_TRIPLE, QT_TRANSLATE_NOOP("engraving/timesig", "Cut triple time (9/8)") }
    };

    for (const TS& timeSignatureType : defaultPalette ? defaultTimeSignatureList : masterTimeSignatureList) {
        auto timeSignature = Factory::makeTimeSig(gpaletteScore->dummy()->segment());
        timeSignature->setSig(Fraction(timeSignatureType.numerator, timeSignatureType.denominator), timeSignatureType.type);
        sp->appendElement(timeSignature, timeSignatureType.name);
    }

    return sp;
}

PalettePtr PaletteCreator::newFretboardDiagramPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::FretboardDiagram);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Fretboard diagrams"));
    sp->setGridSize(42, 45);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    struct FretDiagramInfo {
        String diagram;
        String harmony;
        muse::TranslatableString userName;
    };

    static const std::vector<FretDiagramInfo> fretboardDiagrams = {
        { u"------", u"",  muse::TranslatableString("palette", "Blank") },

        { u"X32O1O", u"C",  muse::TranslatableString("palette", "C") },
        { u"X-554-", u"Cm", muse::TranslatableString("palette", "Cm") },
        { u"X3231O", u"C7", muse::TranslatableString("palette", "C7") },

        { u"XXO232", u"D",  muse::TranslatableString("palette", "D") },
        { u"XXO231", u"Dm", muse::TranslatableString("palette", "Dm") },
        { u"XXO212", u"D7", muse::TranslatableString("palette", "D7") },

        { u"O221OO", u"E",  muse::TranslatableString("palette", "E") },
        { u"O22OOO", u"Em", muse::TranslatableString("palette", "Em") },
        { u"O2O1OO", u"E7", muse::TranslatableString("palette", "E7") },

        { u"-332--", u"F",  muse::TranslatableString("palette", "F") },
        { u"-33---", u"Fm", muse::TranslatableString("palette", "Fm") },
        { u"-3-2--", u"F7", muse::TranslatableString("palette", "F7") },

        { u"32OOO3", u"G",  muse::TranslatableString("palette", "G") },
        { u"-55---", u"Gm", muse::TranslatableString("palette", "Gm") },
        { u"32OOO1", u"G7", muse::TranslatableString("palette", "G7") },

        { u"XO222O", u"A",  muse::TranslatableString("palette", "A") },
        { u"XO221O", u"Am", muse::TranslatableString("palette", "Am") },
        { u"XO2O2O", u"A7", muse::TranslatableString("palette", "A7") },

        { u"X-444-", u"B",  muse::TranslatableString("palette", "B") },
        { u"X-443-", u"Bm", muse::TranslatableString("palette", "Bm") },
        { u"X212O2", u"B7", muse::TranslatableString("palette", "B7") }
    };

    for (const FretDiagramInfo& fretboardDiagram : fretboardDiagrams) {
        auto fret = FretDiagram::createFromPattern(gpaletteScore, fretboardDiagram.diagram);
        fret->setHarmony(fretboardDiagram.harmony);

        if (fretboardDiagram.harmony.empty()) {
            fret->clear();
        }

        sp->appendElement(fret, fretboardDiagram.userName);
    }

    if (!defaultPalette) {
        sp->appendActionIcon(ActionIconType::FFRAME, "insert-fretframe");
    }

    return sp;
}

PalettePtr PaletteCreator::newGuitarPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Guitar);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Guitar"));
    sp->setGridSize(60, 33);
    sp->setMag(1.2);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    qreal w = gpaletteScore->style().spatium() * 8;

    auto capoLine = makeElement<TextLine>(gpaletteScore);
    capoLine->setLen(w);
    capoLine->setBeginText(u"VII");
    capoLine->setEndHookType(HookType::HOOK_90);
    sp->appendElement(capoLine, QT_TRANSLATE_NOOP("palette", "Barré line"), 0.8);

    auto pm = makeElement<PalmMute>(gpaletteScore);
    pm->setLen(w);
    sp->appendElement(pm, QT_TRANSLATE_NOOP("palette", "Palm mute"), 0.8);

    auto letRing = makeElement<LetRing>(gpaletteScore);
    letRing->setLen(w);
    sp->appendElement(letRing, QT_TRANSLATE_NOOP("palette", "Let ring"), 0.8);

    auto tb = Factory::makeTremoloBar(gpaletteScore->dummy());
    tb->points().push_back(PitchValue(0,     0, false));       // "Dip"
    tb->points().push_back(PitchValue(30, -100, false));
    tb->points().push_back(PitchValue(60,    0, false));
    sp->appendElement(tb, QT_TRANSLATE_NOOP("palette", "Tremolo bar"), 0.8);

    static const std::vector<VibratoType> vibratos = {
        { VibratoType::GUITAR_VIBRATO, VibratoType::GUITAR_VIBRATO_WIDE }
    };

    for (VibratoType vibratoType : vibratos) {
        auto vibrato = makeElement<Vibrato>(gpaletteScore);
        vibrato->setVibratoType(vibratoType);
        vibrato->setLen(gpaletteScore->style().spatium() * 8);
        sp->appendElement(vibrato, TConv::userName(vibratoType));
    }

    auto capo = makeElement<Capo>(gpaletteScore);
    capo->setXmlText(String::fromAscii(QT_TRANSLATE_NOOP("palette", "Capo")));
    sp->appendElement(capo, QT_TRANSLATE_NOOP("palette", "Capo"), 0.9)->setElementTranslated(true);

    auto stringTunings = makeElement<StringTunings>(gpaletteScore);
    stringTunings->setXmlText(u"<sym>guitarString6</sym> - D");
    stringTunings->initTextStyleType(TextStyleType::STRING_TUNINGS);
    sp->appendElement(stringTunings, QT_TRANSLATE_NOOP("palette", "String tunings"), 0.9)->setElementTranslated(true);

    sp->appendActionIcon(ActionIconType::STANDARD_BEND, "standard-bend", 1.5);
    sp->appendActionIcon(ActionIconType::PRE_BEND, "pre-bend", 1.5);
    sp->appendActionIcon(ActionIconType::GRACE_NOTE_BEND, "grace-note-bend", 1.4);
    sp->appendActionIcon(ActionIconType::SLIGHT_BEND, "slight-bend", 1.5);

    const char* finger = "pimac";
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
    finger = defaultPalette ? "123456" : "0123456789";
    for (unsigned i = 0; i < strlen(finger); ++i) {
        auto f = makeElement<Fingering>(gpaletteScore);
        f->initTextStyleType(TextStyleType::STRING_NUMBER);
        f->setXmlText(QString(finger[i]));
        sp->appendElement(f, QT_TRANSLATE_NOOP("palette", "String number %1"));
    }

    auto hopo = Factory::makeHammerOnPullOff(gpaletteScore->dummy());
    sp->appendElement(hopo, QT_TRANSLATE_NOOP("palette", "Hammer-on / pull-off"), 0.8);

    static const SymIdList luteSymbols {
        SymId::stringsThumbPosition,
        SymId::luteFingeringRHThumb, SymId::luteFingeringRHFirst,
        SymId::luteFingeringRHSecond, SymId::luteFingeringRHThird
    };

    for (auto i : luteSymbols) {
        auto s = Factory::makeArticulation(gpaletteScore->dummy()->chord());
        s->setSymId(i);
        sp->appendElement(s, s->subtypeUserName());
    }

    struct PlayTechAnnotationInfo {
        const char* xmlText;
        PlayingTechniqueType playTechType;
    };

    static const std::vector<PlayTechAnnotationInfo> playTechAnnotations = {
        { QT_TRANSLATE_NOOP("palette", "distort"),   PlayingTechniqueType::Distortion, },
        { QT_TRANSLATE_NOOP("palette", "overdrive"), PlayingTechniqueType::Overdrive, },
        { QT_TRANSLATE_NOOP("palette", "harmonics"), PlayingTechniqueType::Harmonics, },
        { QT_TRANSLATE_NOOP("palette", "jazz tone"), PlayingTechniqueType::JazzTone, },
        { QT_TRANSLATE_NOOP("palette", "normal"),    PlayingTechniqueType::Natural },
    };

    for (const PlayTechAnnotationInfo& playTechAnnotation : playTechAnnotations) {
        auto pta = makeElement<PlayTechAnnotation>(gpaletteScore);
        pta->setXmlText(playTechAnnotation.xmlText);
        pta->setTechniqueType(playTechAnnotation.playTechType);
        sp->appendElement(pta, TConv::userName(playTechAnnotation.playTechType), 0.8)->setElementTranslated(true);
    }

    return sp;
}

PalettePtr PaletteCreator::newKeyboardPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Keyboard);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Keyboard"));
    sp->setGridSize(73, 30);
    sp->setDrawGrid(true);

    qreal w = gpaletteScore->style().spatium() * 8;

    auto pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setLineVisible(false);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and asterisk)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setEndHookType(HookType::HOOK_90);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (with ped and line)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_90);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (straight hooks)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_90);
    pedal->setEndHookType(HookType::HOOK_45);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled end hook)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_45);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (both hooks angled)"));

    pedal = makeElement<Pedal>(gpaletteScore);
    pedal->setLen(w);
    pedal->setBeginHookType(HookType::HOOK_45);
    pedal->setEndHookType(HookType::HOOK_90);
    pedal->setBeginText(pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
    pedal->setContinueText(pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
    pedal->setEndText(pedal->propertyDefault(Pid::END_TEXT).value<String>());
    sp->appendElement(pedal, QT_TRANSLATE_NOOP("palette", "Pedal (angled start hook)"));

    return sp;
}

PalettePtr PaletteCreator::newPitchPalette(bool defaultPalette)
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Pitch);
    //: The name of a palette
    sp->setName(QT_TRANSLATE_NOOP("palette", "Pitch"));
    sp->setGridSize(100, 30);
    sp->setDrawGrid(true);
    sp->setMag(0.8);

    static const std::vector<OttavaType> ottavasDefault {
        OttavaType::OTTAVA_8VA,
        OttavaType::OTTAVA_8VB,
        OttavaType::OTTAVA_15MA,
        OttavaType::OTTAVA_15MB
    };

    static const std::vector<OttavaType> ottavasMaster {
        OttavaType::OTTAVA_8VA,
        OttavaType::OTTAVA_8VB,
        OttavaType::OTTAVA_15MA,
        OttavaType::OTTAVA_15MB,
        OttavaType::OTTAVA_22MA,
        OttavaType::OTTAVA_22MB
    };

    for (OttavaType ottavaType : defaultPalette ? ottavasDefault : ottavasMaster) {
        auto ottava = makeElement<Ottava>(gpaletteScore);
        ottava->setOttavaType(ottavaType);
        ottava->setLen(gpaletteScore->style().spatium() * 8);
        ottava->styleChanged();
        sp->appendElement(ottava, ottava->subtypeUserName());
    }

    auto a = Factory::makeAmbitus(gpaletteScore->dummy()->segment());
    sp->appendElement(a, QT_TRANSLATE_NOOP("palette", "Ambitus"));
    return sp;
}

PalettePtr PaletteCreator::newHarpPalette()
{
    PalettePtr sp = std::make_shared<Palette>(Palette::Type::Harp);
    sp->setName(QT_TRANSLATE_NOOP("palette", "Harp"));
    sp->setGridSize(90, 30);
    sp->setDrawGrid(true);
    sp->setVisible(false);

    auto pedalDiagram = Factory::makeHarpPedalDiagram(gpaletteScore->dummy()->segment());
    sp->appendElement(pedalDiagram, QT_TRANSLATE_NOOP("palette", "Harp pedal diagram"));

    auto pedalTextDiagram = Factory::makeHarpPedalDiagram(gpaletteScore->dummy()->segment());
    pedalTextDiagram->setIsDiagram(false);

    sp->appendElement(pedalTextDiagram, QT_TRANSLATE_NOOP("palette", "Harp pedal text diagram"));

    return sp;
}
