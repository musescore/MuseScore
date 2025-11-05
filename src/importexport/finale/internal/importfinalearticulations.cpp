/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"
#include "internal/text/finaletextconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/parenthesis.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
// #include "engraving/dom/stem.h"
// #include "engraving/dom/tie.h"
#include "engraving/dom/tremolosinglechord.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

static engraving::Note* findClosestNote(const MusxInstance<details::ArticulationAssign>& articAssign, const MusxInstance<others::ArticulationDef>& articDef, Chord* c)
{
    engraving::Note* n = c->upNote();
    // Attach to correct note based on vertical position
    if (c->notes().size() > 1) {
        // This is a nonsense calculation atm
        double referencePos = n->y() + n->headHeight() / 2;
        if (articAssign->overridePlacement) {
            referencePos -= doubleFromEvpu(articAssign->vertOffset) * SPATIUM20;
        }
        referencePos -= doubleFromEvpu(articDef->yOffsetMain + articDef->defVertPos) * SPATIUM20;
        double bestMatch = DBL_MAX;
        for (engraving::Note* note : c->notes()) {
            double noteDist = std::abs(note->y() - referencePos);
            if (noteDist < bestMatch) {
                bestMatch = noteDist;
                n = note;
            }
        }
    }
    return n;
}

static bool calculateUp(const MusxInstance<details::ArticulationAssign>& articAssign, others::ArticulationDef::AutoVerticalMode vm, ChordRest* cr)
{
    if (articAssign->overridePlacement) {
        return articAssign->aboveEntry;
    }

    switch (vm) {
        /// @todo Rests can be affected by beams, but should otherwise be treated like a note on their given line.
        case others::ArticulationDef::AutoVerticalMode::AutoNoteStem:
            // On notehead side (no voices) or stem side (voices);
            if (cr->measure()->hasVoices(cr->vStaffIdx(), cr->tick(), cr->actualTicks())) {
                return !(cr->track() & 1);
            }
            [[fallthrough]];

        case others::ArticulationDef::AutoVerticalMode::AlwaysNoteheadSide:
            return !cr->ldata()->up;
        case others::ArticulationDef::AutoVerticalMode::StemSide:
            return cr->ldata()->up;

        case others::ArticulationDef::AutoVerticalMode::AboveEntry:
            return true;
        case others::ArticulationDef::AutoVerticalMode::BelowEntry:
            return false;

        default:
            // Unhandled case: AlwaysOnStem - Placement here shouldn't matter in practice
            break;
    }
    return true;
}

static ArticulationAnchor calculateAnchor(const MusxInstance<details::ArticulationAssign>& articAssign, others::ArticulationDef::AutoVerticalMode vm, ChordRest* cr)
{
    if (articAssign->overridePlacement) {
        return articAssign->aboveEntry ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;
    }

    switch (vm) {
        /// @todo Rests can be affected by beams, but should otherwise be treated like a note on their given line.
        case others::ArticulationDef::AutoVerticalMode::AutoNoteStem:
            return ArticulationAnchor::AUTO;
        case others::ArticulationDef::AutoVerticalMode::AlwaysNoteheadSide:
            return !cr->ldata()->up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;
        case others::ArticulationDef::AutoVerticalMode::StemSide:
            return cr->ldata()->up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;

        case others::ArticulationDef::AutoVerticalMode::AboveEntry:
            return ArticulationAnchor::TOP;
        case others::ArticulationDef::AutoVerticalMode::BelowEntry:
            return ArticulationAnchor::BOTTOM;

        default:
            // Unhandled case: AlwaysOnStem - Placement here shouldn't matter in practice
            break;
    }
    return ArticulationAnchor::AUTO;
}

void FinaleParser::importArticulations()
{
    /// @todo offset calculations
    for (auto [entryNumber, cr] : m_entryNumber2CR) {
        MusxInstanceList<details::ArticulationAssign> articAssignList = m_doc->getDetails()->getArray<details::ArticulationAssign>(m_currentMusxPartId, entryNumber);
        for (const MusxInstance<details::ArticulationAssign>& articAssign : articAssignList) {
            const MusxInstance<others::ArticulationDef>& articDef = m_doc->getOthers()->get<others::ArticulationDef>(m_currentMusxPartId, articAssign->articDef);

            if (articDef->mainShape) {
                // shapes currently unsupported
                continue;
            }

            const MusxInstance<FontInfo>& mainFont = articDef->fontMain ? articDef->fontMain : options::FontOptions::getFontInfo(m_doc, options::FontOptions::FontType::Articulation);
            SymId mainSym = FinaleTextConv::symIdFromFinaleChar(articDef->charMain, mainFont);
            // const MusxInstance<FontInfo> altFont = articDef->fontAlt ? articDef->fontAlt : options::FontOptions::getFontInfo(m_doc, options::FontOptions::FontType::Articulation);
            // SymId altSym = FinaleTextConv::symIdFromFinaleChar(articDef->charAlt, altFont);

            // Fermatas
            /// @todo table copied from engraving/dom/fermata.cpp, maybe expose?
            static const std::unordered_map<SymId, FermataType> FERMATA_TYPES = {
                { SymId::fermataAbove, FermataType::Normal },
                { SymId::fermataBelow, FermataType::Normal },
                { SymId::fermataLongAbove, FermataType::Long },
                { SymId::fermataLongBelow, FermataType::Long },
                { SymId::fermataLongHenzeAbove, FermataType::LongHenze },
                { SymId::fermataLongHenzeBelow, FermataType::LongHenze },
                { SymId::fermataVeryLongAbove, FermataType::VeryLong },
                { SymId::fermataVeryLongBelow, FermataType::VeryLong },
                { SymId::fermataShortHenzeAbove, FermataType::ShortHenze },
                { SymId::fermataShortHenzeBelow, FermataType::ShortHenze },
                { SymId::fermataVeryShortAbove, FermataType::VeryShort },
                { SymId::fermataVeryShortBelow, FermataType::VeryShort },
                { SymId::fermataShortAbove, FermataType::Short },
                { SymId::fermataShortBelow, FermataType::Short },
            };
            FermataType ft = muse::value(FERMATA_TYPES, mainSym, FermataType::Undefined);
            if (ft != FermataType::Undefined) {
                Fermata* fermata = Factory::createFermata(cr->segment());
                fermata->setTrack(cr->track());
                fermata->setPlacement(calculateUp(articAssign, articDef->autoVertMode, cr) ? PlacementV::ABOVE : PlacementV::BELOW);
                /// @todo Verify that fermatas have no playback effect in Finale.
                fermata->setSymIdAndTimeStretch(mainSym);
                fermata->setPlay(false);
                // fermata->setSymId(mainSym);
                // fermata->setPlay(articDef->playArtic);
                fermata->setVisible(!articAssign->hide);
                cr->segment()->add(fermata);
                collectElementStyle(fermata);
                continue;
            }

            // Breaths and pauses
            bool breathCreated = false;
            for (BreathType breathType : Breath::BREATH_LIST) {
                if (mainSym == breathType.id) {
                    Segment* breathSegment = cr->measure()->getSegment(SegmentType::Breath, cr->endTick());
                    Breath* breath = Factory::createBreath(breathSegment);
                    breath->setTrack(cr->track());
                    breath->setPlacement(breath->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
                    breath->setSymId(breathType.id);
                    // breath->setPause(breathType.pause); until there is a toggleable play property, leave unset
                    breath->setVisible(!articAssign->hide);
                    breathSegment->add(breath);
                    collectElementStyle(breath);
                    breathCreated = true;
                    break;
                }
            }
            if (breathCreated) {
                continue;
            }

            if (!cr->isChord()) {
                // Rests can only have fermatas or breaths, no other articulations
                continue;
            }
            Chord* c = toChord(cr);

            // Notehead parentheses
            if (mainSym == SymId::noteheadParenthesisLeft || (articDef->charMain == U'(' && !mainFont->calcIsSMuFL())) {
                engraving::Note* n = findClosestNote(articAssign, articDef, c);
                if (!n->leftParen()) {
                    Parenthesis* p = Factory::createParenthesis(n);
                    p->setParent(n);
                    p->setTrack(n->track());
                    p->setVisible(!articAssign->hide);
                    p->setDirection(DirectionH::LEFT);
                    n->add(p);
                    collectElementStyle(p);
                    continue;
                }
            }
            if (mainSym == SymId::noteheadParenthesisRight || (articDef->charMain == U')' && !mainFont->calcIsSMuFL())) {
                engraving::Note* n = findClosestNote(articAssign, articDef, c);
                if (!n->rightParen()) {
                    Parenthesis* p = Factory::createParenthesis(n);
                    p->setParent(n);
                    p->setTrack(n->track());
                    p->setVisible(!articAssign->hide);
                    p->setDirection(DirectionH::RIGHT);
                    n->add(p);
                    collectElementStyle(p);
                    continue;
                }
            }

            // Single-chord tremolos
            TremoloType tt = tremoloTypeFromSymId(mainSym);
            if (tt != TremoloType::INVALID_TREMOLO) {
                TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(c);
                tremolo->setTrack(c->track());
                tremolo->setTremoloType(tt);
                tremolo->setParent(c);
                tremolo->setDurationType(c->durationType());
                tremolo->setPlayTremolo(articDef->playArtic);
                tremolo->setVisible(!articAssign->hide);
                c->setTremoloSingleChord(tremolo);
                collectElementStyle(tremolo);
                continue;
            }

            // Arpeggios
            if (mainSym == SymId::noSym && !c->arpeggio()) {
                std::optional<char32_t> maybeArpeggio = FinaleTextConv::mappedChar(articDef->charMain, mainFont);
                // The Finale symbol is an optional character and not in SMuFL
                if (maybeArpeggio.has_value() && maybeArpeggio.value() == 0xF700u) {
                    Arpeggio* arpeggio = Factory::createArpeggio(c);
                    arpeggio->setTrack(c->track());
                    arpeggio->setArpeggioType(ArpeggioType::NORMAL);
                    // arpeggio->setUserLen1(double * SPATIUM20);
                    // arpeggio->setUserLen2(double * SPATIUM20);
                    // arpeggio->setSpan(int);
                    arpeggio->setPlayArpeggio(articDef->playArtic);
                    // Playback values in finale are EDUs by default, or in % by non-default (exact workings needs to be investigated)
                    // MuseScore is relative to BPM 120 (8 notes take the spread time beats).
                    Fraction totalArpDuration = eduToFraction(articDef->startTopNoteDelta - articDef->startBotNoteDelta);
                    double beatsPerSecondRatio = m_score->tempo(c->segment()->tick()).val / 2.0; // We are this much faster/slower than 120 bpm
                    double timeIn120BPM = totalArpDuration.toDouble() / beatsPerSecondRatio;
                    arpeggio->setStretch(timeIn120BPM * 8.0 / c->notes().size());
                    arpeggio->setParent(c);
                    c->setArpeggio(arpeggio);
                }
            }

            /// @todo Ornament properties, chordlines, fingerings, trills, figured bass?, pedal lines?
            Articulation* a;
            switch (mainSym) {
                case SymId::ornamentTurnInverted:
                case SymId::ornamentMordent:
                case SymId::ornamentTrill:
                case SymId::ornamentTurn:
                case SymId::ornamentShortTrill:
                case SymId::ornamentTremblement:
                case SymId::ornamentUpPrall:
                case SymId::ornamentPrallUp:
                case SymId::ornamentPrallDown:
                case SymId::ornamentPrallMordent:
                case SymId::ornamentUpMordent:
                case SymId::ornamentDownMordent:
                case SymId::ornamentPrecompMordentUpperPrefix:
                case SymId::ornamentTurnSlash:
                    a = toArticulation(Factory::createOrnament(c));
                    break;
                default:
                    a = Factory::createArticulation(c);
                    break;
            }

            // Other articulations
            a->setTrack(c->track());
            a->setSymId(mainSym);
            a->setVisible(!articAssign->hide);
            setAndStyleProperty(a, Pid::ARTICULATION_ANCHOR, int(calculateAnchor(articAssign, articDef->autoVertMode, cr)), true);
            a->setPlayArticulation(articDef->playArtic);
            c->add(a);
            collectElementStyle(a);
        }
    }
}
}
