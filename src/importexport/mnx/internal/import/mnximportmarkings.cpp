/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <algorithm>

#include "engraving/dom/measure.h"
#include "internal/shared/mnxtypesconv.h"
#include "mnximporter.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/property.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/types/symid.h"
#include "log.h"

#include "mnxdom.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {

//---------------------------------------------------------
//   toArticulationAnchor
//   Convert MNX pointing to MuseScore articulation anchor.
//---------------------------------------------------------

static ArticulationAnchor toArticulationAnchor(mnx::MarkingUpDown pointing)
{
    return pointing == mnx::MarkingUpDown::Up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;
}

//---------------------------------------------------------
//   addArticulation
//   Helper to add a specific articulation to a chord.
//---------------------------------------------------------

template<typename Marking>
static Articulation* addArticulation(ChordRest* cr, const Marking& marking, SymId symId, const char* name)
{
    if (!cr->isChord()) {
        LOGW() << "Skipping MNX articulation \"" << name << "\" on rest at "
               << marking.pointer().to_string();
        return nullptr;
    }
    Articulation* articulation = Factory::createArticulation(cr);
    articulation->setSymId(symId);
    cr->add(articulation);
    return articulation;
}

//---------------------------------------------------------
//   importMarkings
//   Import all articulations/markings for an MNX event.
//---------------------------------------------------------

void MnxImporter::importMarkings(const mnx::sequence::Event& mnxEvent, ChordRest* cr)
{
    if (!mnxEvent.markings()) {
        return;
    }
    const auto markings = mnxEvent.markings().value();
    if (const auto accent = markings.accent()) {
        importAccent(accent.value(), cr);
    }
    if (const auto breath = markings.breath()) {
        importBreath(breath.value(), cr);
    }
    if (const auto softAccent = markings.softAccent()) {
        importSoftAccent(softAccent.value(), cr);
    }
    if (const auto spiccato = markings.spiccato()) {
        importSpiccato(spiccato.value(), cr);
    }
    if (const auto staccatissimo = markings.staccatissimo()) {
        importStaccatissimo(staccatissimo.value(), cr);
    }
    if (const auto staccato = markings.staccato()) {
        importStaccato(staccato.value(), cr);
    }
    if (const auto stress = markings.stress()) {
        importStress(stress.value(), cr);
    }
    if (const auto strongAccent = markings.strongAccent()) {
        importStrongAccent(strongAccent.value(), cr);
    }
    if (const auto tenuto = markings.tenuto()) {
        importTenuto(tenuto.value(), cr);
    }
    if (const auto tremolo = markings.tremolo()) {
        importTremolo(tremolo.value(), cr);
    }
    if (const auto unstress = markings.unstress()) {
        importUnstress(unstress.value(), cr);
    }
}

//---------------------------------------------------------
//   importAccent
//   Import MNX accent marking.
//---------------------------------------------------------

void MnxImporter::importAccent(const mnx::sequence::Accent& accent, ChordRest* cr)
{
    if (const auto pointing = accent.pointing()) {
        Articulation* articulation = addArticulation(
            cr, accent, pointing == mnx::MarkingUpDown::Up ? SymId::articAccentAbove : SymId::articAccentBelow,
            "accent");
        if (articulation) {
            /// @todo MNX "pointing" may only describe glyph orientation; confirm whether anchor should be forced.
            setAndStyleProperty(articulation, Pid::ARTICULATION_ANCHOR, int(toArticulationAnchor(*pointing)));
        }
    } else {
        addArticulation(cr, accent, SymId::articAccentAbove, "accent");
    }
}

//---------------------------------------------------------
//   importBreath
//   Import MNX breath mark.
//---------------------------------------------------------

void MnxImporter::importBreath(const mnx::sequence::BreathMark& breath, ChordRest* cr)
{
    Segment* segment = cr->measure()->getSegment(SegmentType::Breath, cr->tick() + cr->ticks());
    Breath* breathMark = Factory::createBreath(segment);
    breathMark->setTrack(cr->track());
    breathMark->setSymId(toMuseScoreBreathMarkSym(breath.symbol()));
    segment->add(breathMark);
}

//---------------------------------------------------------
//   importSoftAccent
//   Import MNX soft accent marking.
//---------------------------------------------------------

void MnxImporter::importSoftAccent(const mnx::sequence::SoftAccent& softAccent, ChordRest* cr)
{
    addArticulation(cr, softAccent, SymId::articSoftAccentAbove, "softAccent");
}

//---------------------------------------------------------
//   importSpiccato
//   Import MNX spiccato marking.
//---------------------------------------------------------

void MnxImporter::importSpiccato(const mnx::sequence::Spiccato& spiccato, ChordRest* cr)
{
    addArticulation(cr, spiccato, SymId::articStaccatissimoStrokeAbove, "spiccato");
}

//---------------------------------------------------------
//   importStaccatissimo
//   Import MNX staccatissimo marking.
//---------------------------------------------------------

void MnxImporter::importStaccatissimo(const mnx::sequence::Staccatissimo& staccatissimo, ChordRest* cr)
{
    addArticulation(cr, staccatissimo, SymId::articStaccatissimoAbove, "staccatissimo");
}

//---------------------------------------------------------
//   importStaccato
//   Import MNX staccato marking.
//---------------------------------------------------------

void MnxImporter::importStaccato(const mnx::sequence::Staccato& staccato, ChordRest* cr)
{
    addArticulation(cr, staccato, SymId::articStaccatoAbove, "staccato");
}

//---------------------------------------------------------
//   importStress
//   Import MNX stress marking.
//---------------------------------------------------------

void MnxImporter::importStress(const mnx::sequence::Stress& stress, ChordRest* cr)
{
    addArticulation(cr, stress, SymId::articStressAbove, "stress");
}

//---------------------------------------------------------
//   importStrongAccent
//   Import MNX strong accent marking.
//---------------------------------------------------------

void MnxImporter::importStrongAccent(const mnx::sequence::StrongAccent& strongAccent, ChordRest* cr)
{
    if (const auto pointing = strongAccent.pointing()) {
        Articulation* articulation = addArticulation(
            cr, strongAccent, pointing == mnx::MarkingUpDown::Up ? SymId::articMarcatoAbove : SymId::articMarcatoBelow,
            "strongAccent");
        if (articulation) {
            /// @todo MNX "pointing" may only describe glyph orientation; confirm whether anchor should be forced.
            setAndStyleProperty(articulation, Pid::ARTICULATION_ANCHOR, int(toArticulationAnchor(*pointing)));
        }
    } else {
        addArticulation(cr, strongAccent, SymId::articMarcatoAbove, "strongAccent");
    }
}

//---------------------------------------------------------
//   importTenuto
//   Import MNX tenuto marking.
//---------------------------------------------------------

void MnxImporter::importTenuto(const mnx::sequence::Tenuto& tenuto, ChordRest* cr)
{
    addArticulation(cr, tenuto, SymId::articTenutoAbove, "tenuto");
}

//---------------------------------------------------------
//   importTremolo
//   Import MNX single-note tremolo marking.
//---------------------------------------------------------

void MnxImporter::importTremolo(const mnx::sequence::SingleNoteTremolo& tremolo, ChordRest* cr)
{
    if (!cr->isChord()) {
        LOGW() << "Skipping MNX tremolo on rest at " << tremolo.pointer().to_string();
        return;
    }
    const int marks = static_cast<int>(tremolo.marks());
    if (marks <= 0) {
        return;
    }
    int tremoloTypeValue = int(TremoloType::R8) - 1 + marks;
    tremoloTypeValue = std::clamp(tremoloTypeValue, int(TremoloType::R8), int(TremoloType::R64));
    TremoloType type = TremoloType(tremoloTypeValue);
    if (type == TremoloType::INVALID_TREMOLO) {
        return;
    }
    TremoloSingleChord* tremoloMark = Factory::createTremoloSingleChord(toChord(cr));
    tremoloMark->setTremoloType(type);
    cr->add(tremoloMark);
}

//---------------------------------------------------------
//   importUnstress
//   Import MNX unstress marking.
//---------------------------------------------------------

void MnxImporter::importUnstress(const mnx::sequence::Unstress& unstress, ChordRest* cr)
{
    addArticulation(cr, unstress, SymId::articUnstressAbove, "unstress");
}
} // namespace mu::iex::mnxio
