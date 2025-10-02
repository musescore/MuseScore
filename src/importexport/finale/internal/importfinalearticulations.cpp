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

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
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

void FinaleParser::importArticulations()
{
    // Layout score (needed for offset calculations)
    /// @todo offset calculations
    m_score->setLayoutAll();
    m_score->doLayout();

    auto calculateUp = [](const MusxInstance<details::ArticulationAssign> articAssign, others::ArticulationDef::AutoVerticalMode vm, ChordRest* cr) -> bool {
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
    };

    auto calculateAnchor = [](const MusxInstance<details::ArticulationAssign> articAssign, others::ArticulationDef::AutoVerticalMode vm, ChordRest* cr) -> ArticulationAnchor {
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
    };


    for (auto [entryNumber, cr] : m_entryNumber2CR) {
        MusxInstanceList<details::ArticulationAssign> articAssignList = m_doc->getDetails()->getArray<details::ArticulationAssign>(m_currentMusxPartId, entryNumber);
        for (const MusxInstance<details::ArticulationAssign> articAssign : articAssignList) {
            const MusxInstance<others::ArticulationDef> articDef = m_doc->getOthers()->get<others::ArticulationDef>(m_currentMusxPartId, articAssign->articDef);

            if (articDef->mainShape) {
                // shapes currently unsupported
                continue;
            }

            const MusxInstance<FontInfo> mainFont = articDef->fontMain ? articDef->fontMain : options::FontOptions::getFontInfo(m_doc, options::FontOptions::FontType::Articulation);
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
                continue;
            }

            if (!cr->isChord()) {
                // Rests can only have fermatas, no other articulations
                continue;
            }
            Chord* c = toChord(cr);

            // Notehead parentheses
            if (mainSym == SymId::noteheadParenthesisLeft || mainSym == SymId::noteheadParenthesisRight) {
                engraving::Note* n = c->upNote();
                if (c->notes().size() > 1) {
                    double referencePos = n->pos().y() - n->headHeight() / 2;
                    if (articAssign->overridePlacement) {
                        referencePos -= doubleFromEvpu(articAssign->vertOffset) * SPATIUM20;
                    }
                    referencePos -= doubleFromEvpu(articDef->yOffsetMain + articDef->defVertPos) * SPATIUM20;
                    double bestMatch = DBL_MAX;
                    for (engraving::Note* note : c->notes()) {
                        double noteDist = std::abs(note->pos().y() - referencePos);
                        if (noteDist < bestMatch) {
                            bestMatch = noteDist;
                            n = note;
                        }
                    }
                }
                n->setParenthesesMode(mainSym == SymId::noteheadParenthesisLeft ? ParenthesesMode::LEFT : ParenthesesMode::RIGHT);
                // if we use Element::add(parenthesis) instead, that should make repositioning easier.
                continue;
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
                continue;
            }

            /// @todo Arpeggios; Their symbol is not in SMuFL.

            // Other articulations
            Articulation* a = Factory::createArticulation(c);
            a->setTrack(c->track());
            a->setSymId(mainSym);
            a->setVisible(!articAssign->hide);
            a->setAnchor(calculateAnchor(articAssign, articDef->autoVertMode, cr));
            if (a->anchor() != ArticulationAnchor::AUTO) {
                a->setPropertyFlags(engraving::Pid::ARTICULATION_ANCHOR, engraving::PropertyFlags::UNSTYLED);
            }
            a->setPlayArticulation(articDef->playArtic);
            c->add(a);
        }
    }
}
}
