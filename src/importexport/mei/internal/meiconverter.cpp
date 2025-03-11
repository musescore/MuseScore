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

#include "meiconverter.h"

#include <iostream>
#include <sstream>
#include <string>
#include <valarray>

#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/slurtie.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

using namespace mu::iex::mei;
using namespace mu;
using namespace muse;

StringList Convert::logs;

engraving::ElementType Convert::elementTypeForDir(const libmei::Element& meiElement)
{
    engraving::ElementType dirType = engraving::ElementType::EXPRESSION;
    const libmei::AttTyped* typedAtt = dynamic_cast<const libmei::AttTyped*>(&meiElement);
    if (typedAtt) {
        if (Convert::hasTypeValue(typedAtt->GetType(), std::string(DIR_TYPE) + "playtech-annotation")) {
            dirType = engraving::ElementType::PLAYTECH_ANNOTATION;
        } else if (Convert::hasTypeValue(typedAtt->GetType(), std::string(DIR_TYPE) + "staff-text")) {
            dirType = engraving::ElementType::STAFF_TEXT;
        }
    }
    return dirType;
}

engraving::ElementType Convert::elementTypeForDirWithExt(const libmei::Element& meiElement)
{
    engraving::ElementType dirType = engraving::ElementType::TEXTLINE;
    const libmei::AttTyped* typedAtt = dynamic_cast<const libmei::AttTyped*>(&meiElement);
    if (typedAtt) {
        if (Convert::hasTypeValue(typedAtt->GetType(), std::string(DIR_TYPE) + "hairpin")) {
            dirType = engraving::ElementType::HAIRPIN;
        }
    }
    return dirType;
}

engraving::ElementType Convert::elementTypeFor(const libmei::RepeatMark& meiRepeatMark)
{
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_daCapo):
    case (libmei::repeatMarkLog_FUNC_dalSegno):
        return engraving::ElementType::JUMP;
    default:
        return engraving::ElementType::MARKER;
    }
}

bool Convert::isDirWithExt(const libmei::Dir& meiDir)
{
    return meiDir.HasExtender() && (meiDir.GetExtender() == libmei::BOOLEAN_true);
}

bool Convert::isMordent(const engraving::Ornament* ornament)
{
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentMordent):
    case (engraving::SymId::ornamentShortTrill):
    case (engraving::SymId::ornamentTremblement):
    case (engraving::SymId::ornamentPrallMordent):
    case (engraving::SymId::ornamentUpPrall):
    case (engraving::SymId::ornamentPrecompMordentUpperPrefix):
    case (engraving::SymId::ornamentUpMordent):
    case (engraving::SymId::ornamentDownMordent):
    case (engraving::SymId::ornamentPrallDown):
    case (engraving::SymId::ornamentPrallUp):
    case (engraving::SymId::ornamentLinePrall):
    case (engraving::SymId::ornamentPrecompSlide):
    case (engraving::SymId::ornamentTremblementCouperin):
    case (engraving::SymId::ornamentPinceCouperin):
        return true;
    default:
        return false;
    }
}

bool Convert::isTrill(const engraving::Ornament* ornament)
{
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentTrill):
    case (engraving::SymId::ornamentShake3):
    case (engraving::SymId::ornamentShakeMuffat1):
        return true;
    default:
        return false;
    }
}

bool Convert::isTurn(const engraving::Ornament* ornament)
{
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentTurnInverted):
    case (engraving::SymId::ornamentTurnSlash):
    case (engraving::SymId::ornamentTurn):
        return true;
    default:
        return false;
    }
}

engraving::AccidentalType Convert::accidFromMEI(const libmei::data_ACCIDENTAL_WRITTEN meiAccid, bool& warning)
{
    warning = false;
    switch (meiAccid) {
    case (libmei::ACCIDENTAL_WRITTEN_NONE): return engraving::AccidentalType::NONE;
    case (libmei::ACCIDENTAL_WRITTEN_n): return engraving::AccidentalType::NATURAL;
    case (libmei::ACCIDENTAL_WRITTEN_f): return engraving::AccidentalType::FLAT;
    case (libmei::ACCIDENTAL_WRITTEN_ff): return engraving::AccidentalType::FLAT2;
    case (libmei::ACCIDENTAL_WRITTEN_tf): return engraving::AccidentalType::FLAT3;
    case (libmei::ACCIDENTAL_WRITTEN_s): return engraving::AccidentalType::SHARP;
    case (libmei::ACCIDENTAL_WRITTEN_x): return engraving::AccidentalType::SHARP2;
    case (libmei::ACCIDENTAL_WRITTEN_ts): return engraving::AccidentalType::SHARP3;
    case (libmei::ACCIDENTAL_WRITTEN_nf): return engraving::AccidentalType::NATURAL_FLAT;
    case (libmei::ACCIDENTAL_WRITTEN_ns): return engraving::AccidentalType::NATURAL_SHARP;
    case (libmei::ACCIDENTAL_WRITTEN_ss): return engraving::AccidentalType::SHARP_SHARP;
    default:
        warning = true;
        return engraving::AccidentalType::NATURAL;
    }
}

libmei::data_ACCIDENTAL_WRITTEN Convert::accidToMEI(const engraving::AccidentalType accid)
{
    switch (accid) {
    case (engraving::AccidentalType::NONE): return libmei::ACCIDENTAL_WRITTEN_NONE;
    case (engraving::AccidentalType::NATURAL): return libmei::ACCIDENTAL_WRITTEN_n;
    case (engraving::AccidentalType::FLAT): return libmei::ACCIDENTAL_WRITTEN_f;
    case (engraving::AccidentalType::FLAT2): return libmei::ACCIDENTAL_WRITTEN_ff;
    case (engraving::AccidentalType::FLAT3): return libmei::ACCIDENTAL_WRITTEN_tf;
    case (engraving::AccidentalType::SHARP): return libmei::ACCIDENTAL_WRITTEN_s;
    case (engraving::AccidentalType::SHARP2): return libmei::ACCIDENTAL_WRITTEN_x;
    case (engraving::AccidentalType::SHARP3): return libmei::ACCIDENTAL_WRITTEN_ts;
    case (engraving::AccidentalType::NATURAL_FLAT): return libmei::ACCIDENTAL_WRITTEN_nf;
    case (engraving::AccidentalType::NATURAL_SHARP): return libmei::ACCIDENTAL_WRITTEN_ns;
    case (engraving::AccidentalType::SHARP_SHARP): return libmei::ACCIDENTAL_WRITTEN_ss;
    default:
        return libmei::ACCIDENTAL_WRITTEN_n;
    }
}

engraving::AccidentalVal Convert::accidGesFromMEI(const libmei::data_ACCIDENTAL_GESTURAL meiAccid, bool& warning)
{
    warning = false;
    switch (meiAccid) {
    case (libmei::ACCIDENTAL_GESTURAL_NONE): return engraving::AccidentalVal::NATURAL;
    case (libmei::ACCIDENTAL_GESTURAL_n): return engraving::AccidentalVal::NATURAL;
    case (libmei::ACCIDENTAL_GESTURAL_f): return engraving::AccidentalVal::FLAT;
    case (libmei::ACCIDENTAL_GESTURAL_ff): return engraving::AccidentalVal::FLAT2;
    case (libmei::ACCIDENTAL_GESTURAL_tf): return engraving::AccidentalVal::FLAT3;
    case (libmei::ACCIDENTAL_GESTURAL_s): return engraving::AccidentalVal::SHARP;
    case (libmei::ACCIDENTAL_GESTURAL_ss): return engraving::AccidentalVal::SHARP2;
    case (libmei::ACCIDENTAL_GESTURAL_ts): return engraving::AccidentalVal::SHARP3;
    default:
        warning = true;
        return engraving::AccidentalVal::NATURAL;
    }
}

libmei::data_ACCIDENTAL_GESTURAL Convert::accidGesToMEI(const engraving::AccidentalVal accid)
{
    switch (accid) {
    case (engraving::AccidentalVal::NATURAL): return libmei::ACCIDENTAL_GESTURAL_n;
    case (engraving::AccidentalVal::FLAT): return libmei::ACCIDENTAL_GESTURAL_f;
    case (engraving::AccidentalVal::FLAT2): return libmei::ACCIDENTAL_GESTURAL_ff;
    case (engraving::AccidentalVal::FLAT3): return libmei::ACCIDENTAL_GESTURAL_tf;
    case (engraving::AccidentalVal::SHARP): return libmei::ACCIDENTAL_GESTURAL_s;
    case (engraving::AccidentalVal::SHARP2): return libmei::ACCIDENTAL_GESTURAL_ss;
    case (engraving::AccidentalVal::SHARP3): return libmei::ACCIDENTAL_GESTURAL_ts;
    default:
        return libmei::ACCIDENTAL_GESTURAL_n;
    }
}

engraving::ArticulationAnchor Convert::anchorFromMEI(const libmei::data_STAFFREL meiPlace, bool& warning)
{
    warning = false;
    switch (meiPlace) {
    case (libmei::STAFFREL_above): return engraving::ArticulationAnchor::TOP;
    case (libmei::STAFFREL_below): return engraving::ArticulationAnchor::BOTTOM;
    default:
        return engraving::ArticulationAnchor::AUTO;
    }
}

libmei::data_STAFFREL Convert::anchorToMEI(engraving::ArticulationAnchor anchor)
{
    switch (anchor) {
    case (engraving::ArticulationAnchor::TOP): return libmei::STAFFREL_above;
    case (engraving::ArticulationAnchor::BOTTOM): return libmei::STAFFREL_below;
    default:
        return libmei::STAFFREL_NONE;
    }
}

void Convert::arpegFromMEI(engraving::Arpeggio* arpeggio, const libmei::Arpeg& meiArpeg, bool& warning)
{
    warning = false;

    // @arrow and @order
    if (meiArpeg.HasOrder()) {
        switch (meiArpeg.GetOrder()) {
        case (libmei::arpegLog_ORDER_down):
            arpeggio->setArpeggioType(engraving::ArpeggioType::DOWN);
            break;
        case (libmei::arpegLog_ORDER_nonarp):
            arpeggio->setArpeggioType(engraving::ArpeggioType::BRACKET);
            break;
        case (libmei::arpegLog_ORDER_up):
            arpeggio->setArpeggioType(engraving::ArpeggioType::UP);
            break;
        default:
            break;
        }
    }

    // @color
    Convert::colorFromMEI(arpeggio, meiArpeg);
}

libmei::Arpeg Convert::arpegToMEI(const engraving::Arpeggio* arpeggio)
{
    libmei::Arpeg meiArpeg;

    // @arrow and @order
    switch (arpeggio->arpeggioType()) {
    case (engraving::ArpeggioType::DOWN):
        meiArpeg.SetOrder(libmei::arpegLog_ORDER_down);
        meiArpeg.SetArrow(libmei::BOOLEAN_true);
        break;
    case (engraving::ArpeggioType::UP):
        meiArpeg.SetOrder(libmei::arpegLog_ORDER_up);
        meiArpeg.SetArrow(libmei::BOOLEAN_true);
        break;
    case (engraving::ArpeggioType::BRACKET):
        meiArpeg.SetOrder(libmei::arpegLog_ORDER_nonarp);
        break;
    default:
        break;
    }

    // @color
    Convert::colorToMEI(arpeggio, meiArpeg);

    return meiArpeg;
}

void Convert::articFromMEI(engraving::Articulation* articulation, const libmei::Artic& meiArtic, bool& warning)
{
    engraving::SymId symId = engraving::SymId::noSym;

    // @artic
    if (meiArtic.HasArtic() && (meiArtic.GetArtic().size() == 1)) {
        switch (meiArtic.GetArtic().at(0)) {
        // ordered by SymId
        case (libmei::ARTICULATION_acc): symId = engraving::SymId::articAccentAbove;
            break;
        case (libmei::ARTICULATION_marc): symId = engraving::SymId::articMarcatoAbove;
            break;
        case (libmei::ARTICULATION_acc_soft): symId = engraving::SymId::articSoftAccentAbove;
            break;
        case (libmei::ARTICULATION_stacciss): symId = engraving::SymId::articStaccatissimoAbove;
            break;
        case (libmei::ARTICULATION_stroke): symId = engraving::SymId::articStaccatissimoStrokeAbove;
            break;
        case (libmei::ARTICULATION_spicc): symId = engraving::SymId::articStaccatissimoWedgeAbove;
            break;
        case (libmei::ARTICULATION_stacc): symId = engraving::SymId::articStaccatoAbove;
            break;
        case (libmei::ARTICULATION_ten): symId = engraving::SymId::articTenutoAbove;
            break;
        case (libmei::ARTICULATION_stop): symId = engraving::SymId::brassMuteClosed;
            break;
        case (libmei::ARTICULATION_open): symId = engraving::SymId::brassMuteOpen;
            break;
        case (libmei::ARTICULATION_snap): symId = engraving::SymId::pluckedSnapPizzicatoAbove;
            break;
        case (libmei::ARTICULATION_dnbow): symId = engraving::SymId::stringsDownBow;
            break;
        case (libmei::ARTICULATION_harm): symId = engraving::SymId::stringsHarmonic;
            break;
        case (libmei::ARTICULATION_upbow): symId = engraving::SymId::stringsUpBow;
            break;
        default: break;
        }
    }
    if (meiArtic.HasArtic() && (meiArtic.GetArtic().size() == 2)) {
        if (meiArtic.GetArtic().at(0) == libmei::ARTICULATION_stacc) {
            switch (meiArtic.GetArtic().at(1)) {
            // ordered by @artic
            case (libmei::ARTICULATION_acc): symId = engraving::SymId::articAccentStaccatoAbove;
                break;
            case (libmei::ARTICULATION_marc): symId = engraving::SymId::articMarcatoStaccatoAbove;
                break;
            case (libmei::ARTICULATION_ten): symId = engraving::SymId::articTenutoStaccatoAbove;
                break;
            default: break;
            }
        } else if (meiArtic.GetArtic().at(0) == libmei::ARTICULATION_ten) {
            switch (meiArtic.GetArtic().at(1)) {
            // ordered by @artic
            case (libmei::ARTICULATION_acc): symId = engraving::SymId::articTenutoAccentAbove;
                break;
            case (libmei::ARTICULATION_marc): symId = engraving::SymId::articMarcatoTenutoAbove;
                break;
            default: break;
            }
        }
    }
    articulation->setSymId(symId);

    // @place
    if (meiArtic.HasPlace()) {
        bool placeWarning = false;
        articulation->setAnchor(Convert::anchorFromMEI(meiArtic.GetPlace(), placeWarning));
        articulation->setPropertyFlags(engraving::Pid::ARTICULATION_ANCHOR, engraving::PropertyFlags::UNSTYLED);
        warning = (warning || placeWarning);
    }

    // @color
    Convert::colorFromMEI(articulation, meiArtic);
}

libmei::Artic Convert::articToMEI(const engraving::Articulation* articulation)
{
    libmei::Artic meiArtic;

    // @artic
    switch (articulation->symId()) {
    case (engraving::SymId::articAccentAbove):
    case (engraving::SymId::articAccentBelow): meiArtic.SetArtic({ libmei::ARTICULATION_acc });
        break;
    case (engraving::SymId::articAccentStaccatoAbove):
    case (engraving::SymId::articAccentStaccatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stacc, libmei::ARTICULATION_acc });
        break;
    case (engraving::SymId::articMarcatoAbove):
    case (engraving::SymId::articMarcatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_marc });
        break;
    case (engraving::SymId::articMarcatoStaccatoAbove):
    case (engraving::SymId::articMarcatoStaccatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stacc, libmei::ARTICULATION_marc });
        break;
    case (engraving::SymId::articMarcatoTenutoAbove):
    case (engraving::SymId::articMarcatoTenutoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_ten, libmei::ARTICULATION_marc });
        break;
    case (engraving::SymId::articSoftAccentAbove):
    case (engraving::SymId::articSoftAccentBelow): meiArtic.SetArtic({ libmei::ARTICULATION_acc_soft });
        break;
    case (engraving::SymId::articStaccatissimoAbove):
    case (engraving::SymId::articStaccatissimoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stacciss });
        break;
    case (engraving::SymId::articStaccatissimoStrokeAbove):
    case (engraving::SymId::articStaccatissimoStrokeBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stroke });
        break;
    case (engraving::SymId::articStaccatissimoWedgeAbove):
    case (engraving::SymId::articStaccatissimoWedgeBelow): meiArtic.SetArtic({ libmei::ARTICULATION_spicc });
        break;
    case (engraving::SymId::articStaccatoAbove):
    case (engraving::SymId::articStaccatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stacc });
        break;
    case (engraving::SymId::articTenutoAbove):
    case (engraving::SymId::articTenutoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_ten });
        break;
    case (engraving::SymId::articTenutoAccentAbove):
    case (engraving::SymId::articTenutoAccentBelow): meiArtic.SetArtic({ libmei::ARTICULATION_ten, libmei::ARTICULATION_acc });
        break;
    case (engraving::SymId::articTenutoStaccatoAbove):
    case (engraving::SymId::articTenutoStaccatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_stacc, libmei::ARTICULATION_ten });
        break;
    case (engraving::SymId::pluckedSnapPizzicatoAbove):
    case (engraving::SymId::pluckedSnapPizzicatoBelow): meiArtic.SetArtic({ libmei::ARTICULATION_snap });
        break;
    case (engraving::SymId::stringsDownBow):
    case (engraving::SymId::stringsDownBowTurned): meiArtic.SetArtic({ libmei::ARTICULATION_dnbow });
        break;
    case (engraving::SymId::stringsUpBow):
    case (engraving::SymId::stringsUpBowTurned): meiArtic.SetArtic({ libmei::ARTICULATION_upbow });
        break;
    // Values without down or invert
    case (engraving::SymId::brassMuteClosed): meiArtic.SetArtic({ libmei::ARTICULATION_stop });
        break;
    case (engraving::SymId::brassMuteOpen): meiArtic.SetArtic({ libmei::ARTICULATION_open });
        break;
    case (engraving::SymId::stringsHarmonic): meiArtic.SetArtic({ libmei::ARTICULATION_harm });
        break;

    default:
        break;
    }

    // @place
    if (articulation->propertyFlags(engraving::Pid::ARTICULATION_ANCHOR) == engraving::PropertyFlags::UNSTYLED) {
        meiArtic.SetPlace(Convert::anchorToMEI(articulation->anchor()));
    }

    // @color
    Convert::colorToMEI(articulation, meiArtic);

    return meiArtic;
}

engraving::BarLineType Convert::barlineFromMEI(const libmei::data_BARRENDITION meiBarline, bool& warning)
{
    warning = false;
    switch (meiBarline) {
    case (libmei::BARRENDITION_single): return engraving::BarLineType::NORMAL;
    case (libmei::BARRENDITION_dbl): return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_rptstart): return engraving::BarLineType::START_REPEAT;
    case (libmei::BARRENDITION_rptend): return engraving::BarLineType::END_REPEAT;
    case (libmei::BARRENDITION_dashed): return engraving::BarLineType::BROKEN;
    case (libmei::BARRENDITION_end): return engraving::BarLineType::END;
    case (libmei::BARRENDITION_rptboth): return engraving::BarLineType::END_START_REPEAT;
    case (libmei::BARRENDITION_dotted): return engraving::BarLineType::DOTTED;
    case (libmei::BARRENDITION_heavy): return engraving::BarLineType::HEAVY;
    case (libmei::BARRENDITION_dblheavy): return engraving::BarLineType::DOUBLE_HEAVY;
    case (libmei::BARRENDITION_dbldashed):
    case (libmei::BARRENDITION_dbldotted):
        warning = true;
        return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_dblsegno):
        warning = true;
        return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_segno):
    case (libmei::BARRENDITION_invis):
        warning = true;
        return engraving::BarLineType::SINGLE;
    default:
        return engraving::BarLineType::SINGLE;
    }
}

libmei::data_BARRENDITION Convert::barlineToMEI(const engraving::BarLineType barline)
{
    switch (barline) {
    case (engraving::BarLineType::NORMAL): return libmei::BARRENDITION_NONE;
    case (engraving::BarLineType::DOUBLE): return libmei::BARRENDITION_dbl;
    case (engraving::BarLineType::START_REPEAT): return libmei::BARRENDITION_rptstart;
    case (engraving::BarLineType::END_REPEAT): return libmei::BARRENDITION_rptend;
    case (engraving::BarLineType::BROKEN): return libmei::BARRENDITION_dashed;
    case (engraving::BarLineType::END): return libmei::BARRENDITION_end;
    case (engraving::BarLineType::END_START_REPEAT):
        return libmei::BARRENDITION_rptboth;
    case (engraving::BarLineType::DOTTED): return libmei::BARRENDITION_dotted;
    case (engraving::BarLineType::HEAVY): return libmei::BARRENDITION_heavy;
    case (engraving::BarLineType::DOUBLE_HEAVY): return libmei::BARRENDITION_dblheavy;
    case (engraving::BarLineType::REVERSE_END):
        LOGD() << "Unsupported engraving::BarLineType::REVERSE_END";
        return libmei::BARRENDITION_dbl;
    default:
        return libmei::BARRENDITION_single;
    }
}

engraving::BeamMode Convert::beamFromMEI(const std::string& typeAtt, const std::string& prefix, bool& warning)
{
    warning = false;

    engraving::BeamMode beamMode = engraving::BeamMode::AUTO;
    if (Convert::hasTypeValue(typeAtt, prefix + "-begin")) {
        beamMode = engraving::BeamMode::BEGIN;
    } else if (Convert::hasTypeValue(typeAtt, prefix + "-mid")) {
        beamMode = engraving::BeamMode::MID;
    } else if (Convert::hasTypeValue(typeAtt, prefix + "-none")) {
        beamMode = engraving::BeamMode::NONE;
    }

    return beamMode;
}

std::string Convert::beamToMEI(engraving::BeamMode beamMode, const std::string& prefix)
{
    std::string beamType;

    switch (beamMode) {
    case (engraving::BeamMode::BEGIN):
        beamType = prefix + "-begin";
        break;
    case (engraving::BeamMode::MID):
        beamType = prefix + "-mid";
        break;
    case (engraving::BeamMode::NONE):
        beamType = prefix + "-none";
        break;
    default: break;
    }

    return beamType;
}

engraving::BeamMode Convert::breaksecFromMEI(int breaksec, bool& warning)
{
    warning = false;

    switch (breaksec) {
    case (1): return engraving::BeamMode::BEGIN16;
    case (2): return engraving::BeamMode::BEGIN32;
    default:
        warning = true;
        break;
    }

    return engraving::BeamMode::AUTO;
}

int Convert::breaksecToMEI(engraving::BeamMode beamMode)
{
    int breaksec = MEI_UNSET;

    switch (beamMode) {
    case (engraving::BeamMode::BEGIN16): breaksec = 1;
        break;
    case (engraving::BeamMode::BEGIN32): breaksec = 2;
        break;
    default: break;
    }

    return breaksec;
}

Convert::BracketStruct Convert::bracketFromMEI(const libmei::StaffGrp& meiStaffGrp)
{
    Convert::BracketStruct bracketSt;

    switch (meiStaffGrp.GetSymbol()) {
    case (libmei::staffGroupingSym_SYMBOL_bracket): bracketSt.bracketType = engraving::BracketType::NORMAL;
        break;
    case (libmei::staffGroupingSym_SYMBOL_brace): bracketSt.bracketType = engraving::BracketType::BRACE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_bracketsq): bracketSt.bracketType = engraving::BracketType::SQUARE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_line): bracketSt.bracketType = engraving::BracketType::LINE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_none): bracketSt.bracketType = engraving::BracketType::NO_BRACKET;
        break;
    default: break;
    }

    if (meiStaffGrp.HasBarThru() && meiStaffGrp.GetBarThru() == libmei::BOOLEAN_true) {
        bracketSt.barLineSpan = 1;
    }

    return bracketSt;
}

libmei::StaffGrp Convert::bracketToMEI(const engraving::BracketType bracket, int barLineSpan)
{
    libmei::StaffGrp meiStaffGrp;
    // @symbol
    switch (bracket) {
    case (engraving::BracketType::NORMAL): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_bracket);
        break;
    case (engraving::BracketType::BRACE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_brace);
        break;
    case (engraving::BracketType::SQUARE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_bracketsq);
        break;
    case (engraving::BracketType::LINE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_line);
        break;
    case (engraving::BracketType::NO_BRACKET): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_none);
        break;
    default: break;
    }
    // @bar.thru
    if (barLineSpan > 0) {
        meiStaffGrp.SetBarThru(libmei::BOOLEAN_true);
    }

    return meiStaffGrp;
}

void Convert::breathFromMEI(engraving::Breath* breath, const libmei::Breath& meiBreath, bool& warning)
{
    warning = false;

    // @glyph.name
    bool smufl = (meiBreath.HasGlyphAuth() && (meiBreath.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::breathMarkComma;

    if (smufl && meiBreath.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiBreath.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiBreath.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiBreath.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }

    // @color
    Convert::colorFromMEI(breath, meiBreath);

    breath->setSymId(symId);
}

libmei::Breath Convert::breathToMEI(const engraving::Breath* breath)
{
    libmei::Breath meiBreath;

    bool smufl = false;
    // @glyph.name
    switch (breath->symId()) {
    case (engraving::SymId::breathMarkTick):
    case (engraving::SymId::breathMarkSalzedo):
    case (engraving::SymId::breathMarkUpbow): smufl = true;
    default:
        break;
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(breath->symId());
        meiBreath.SetGlyphName(glyphName.ascii());
        meiBreath.SetGlyphAuth(SMUFL_AUTH);
    }

    // @color
    Convert::colorToMEI(breath, meiBreath);

    return meiBreath;
}

void Convert::caesuraFromMEI(engraving::Breath* breath, const libmei::Caesura& meiCeasura, bool& warning)
{
    warning = false;

    // @glyph.name
    bool smufl = (meiCeasura.HasGlyphAuth() && (meiCeasura.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::caesura;

    if (smufl && meiCeasura.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiCeasura.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiCeasura.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiCeasura.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }

    // @color
    Convert::colorFromMEI(breath, meiCeasura);

    breath->setSymId(symId);
}

libmei::Caesura Convert::caesuraToMEI(const engraving::Breath* breath)
{
    libmei::Caesura meiCaesura;

    bool smufl = false;
    // @glyph.name
    switch (breath->symId()) {
    case (engraving::SymId::caesuraCurved):
    case (engraving::SymId::caesuraShort):
    case (engraving::SymId::caesuraThick):
    case (engraving::SymId::chantCaesura):
    case (engraving::SymId::caesuraSingleStroke): smufl = true;
    default:
        break;
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(breath->symId());
        meiCaesura.SetGlyphName(glyphName.ascii());
        meiCaesura.SetGlyphAuth(SMUFL_AUTH);
    }

    // @color
    Convert::colorToMEI(breath, meiCaesura);

    return meiCaesura;
}

engraving::ClefType Convert::clefFromMEI(const libmei::Clef& meiClef, bool& warning)
{
    warning = false;

    // @glyph.name
    bool smufl = (meiClef.HasGlyphAuth() && (meiClef.GetGlyphAuth() == SMUFL_AUTH));

    // We give @glyph.name priority over @shape
    if (smufl && meiClef.HasGlyphName()) {
        if (meiClef.GetGlyphName() == "cClefFrench") {
            switch (meiClef.GetLine()) {
            case 1: return engraving::ClefType::C1_F18C;
            case 3: return engraving::ClefType::C3_F18C;
            case 4: return engraving::ClefType::C4_F18C;
            default:
                break;
            }
        } else if (meiClef.GetGlyphName() == "cClefFrench20C") {
            switch (meiClef.GetLine()) {
            case 1: return engraving::ClefType::C1_F20C;
            case 3: return engraving::ClefType::C3_F20C;
            case 4: return engraving::ClefType::C4_F20C;
            default:
                break;
            }
        } else if (meiClef.GetGlyphName() == "cClefSquare") {
            switch (meiClef.GetLine()) {
            case 2: return engraving::ClefType::C_19C;
            default:
                break;
            }
        } else if (meiClef.GetGlyphName() == "fClefFrench") {
            switch (meiClef.GetLine()) {
            case 4: return engraving::ClefType::F_F18C;
            default:
                break;
            }
        } else if (meiClef.GetGlyphName() == "fClef19thCentury") {
            switch (meiClef.GetLine()) {
            case 4: return engraving::ClefType::F_19C;
            default:
                break;
            }
        } else if (meiClef.GetGlyphName() == "gClef8vbParens") {
            switch (meiClef.GetLine()) {
            case 2: return engraving::ClefType::G8_VB_P;
            default:
                break;
            }
        } else {
            LOGD() << "Unsupported clef@glyph.name";
            // try to find a proper replacement from other attributes
        }
    }

    // @shape
    if (meiClef.GetShape() == libmei::CLEFSHAPE_G) {
        if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_below) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::G8_VB;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::G15_MB;
            default:
                break;
            }
        } else if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_above) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::G8_VA;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::G15_MA;
            default:
                break;
            }
        }
        if (meiClef.GetLine() == 2) {
            return engraving::ClefType::G;
        } else if (meiClef.GetLine() == 1) {
            return engraving::ClefType::G_1;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_C) {
        switch (meiClef.GetLine()) {
        case (1): return engraving::ClefType::C1;
        case (2): return engraving::ClefType::C2;
        case (3): return engraving::ClefType::C3;
        case (4):
            if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_below && meiClef.GetDis() == libmei::OCTAVE_DIS_8) {
                return engraving::ClefType::C4_8VB;
            }
            return engraving::ClefType::C4;
        case (5): return engraving::ClefType::C5;
        default:
            break;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_F) {
        if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_below) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::F8_VB;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::F15_MB;
            default:
                break;
            }
        } else if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_above) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::F_8VA;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::F_15MA;
            default:
                break;
            }
        }
        switch (meiClef.GetLine()) {
        case (3): return engraving::ClefType::F_B;
        case (4): return engraving::ClefType::F;
        case (5): return engraving::ClefType::F_C;
        default:
            break;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_perc) {
        return engraving::ClefType::PERC;
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_GG && meiClef.GetLine() == 2) {
        return engraving::ClefType::G8_VB_O;
    }
    warning = true;
    return engraving::ClefType::G;
}

libmei::Clef Convert::clefToMEI(engraving::ClefType clef)
{
    libmei::Clef meiClef;
    // @shape
    switch (clef) {
    case (engraving::ClefType::G):
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::G_1):
        meiClef.SetShape(libmei::CLEFSHAPE_G);
        break;
    case (engraving::ClefType::C1):
    case (engraving::ClefType::C2):
    case (engraving::ClefType::C3):
    case (engraving::ClefType::C4):
    case (engraving::ClefType::C4_8VB):
    case (engraving::ClefType::C5):
        meiClef.SetShape(libmei::CLEFSHAPE_C);
        break;
    case (engraving::ClefType::F):
    case (engraving::ClefType::F15_MB):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::F_8VA):
    case (engraving::ClefType::F_15MA):
    case (engraving::ClefType::F_B):
    case (engraving::ClefType::F_C):
        meiClef.SetShape(libmei::CLEFSHAPE_F);
        break;
    case (engraving::ClefType::G8_VB_O):
        meiClef.SetShape(libmei::CLEFSHAPE_GG);
        break;
    case (engraving::ClefType::PERC):
    case (engraving::ClefType::PERC2):
        meiClef.SetShape(libmei::CLEFSHAPE_perc);
        break;
    default:
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(engraving::ClefInfo::symId(clef));
        meiClef.SetGlyphName(glyphName.ascii());
        meiClef.SetGlyphAuth(SMUFL_AUTH);
        switch (glyphName.at(0).unicode()) {
        case 'c':
            meiClef.SetShape(libmei::CLEFSHAPE_C);
            break;
        case 'f':
            meiClef.SetShape(libmei::CLEFSHAPE_F);
            break;
        case 'g':
            meiClef.SetShape(libmei::CLEFSHAPE_G);
            break;
        default:
            LOGD() << "Unsupported engraving::ClefType";
            meiClef.SetShape(libmei::CLEFSHAPE_NONE);
            break;
        }
    }

    if (meiClef.GetShape() != libmei::CLEFSHAPE_perc) {
        const int line = engraving::ClefInfo::line(clef);
        meiClef.SetLine(line);
    }

    // @dis and @dis.place
    switch (clef) {
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::F_8VA):
    case (engraving::ClefType::C4_8VB):
        meiClef.SetDis(libmei::OCTAVE_DIS_8);
        break;
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::F15_MB):
    case (engraving::ClefType::F_15MA):
        meiClef.SetDis(libmei::OCTAVE_DIS_15);
        break;
    default:
        break;
    }

    // @dis.place
    switch (clef) {
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::C4_8VB):
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::F15_MB):
        meiClef.SetDisPlace(libmei::STAFFREL_basic_below);
        break;
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::F_8VA):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::F_15MA):
        meiClef.SetDisPlace(libmei::STAFFREL_basic_above);
        break;
    default:
        break;
    }

    return meiClef;
}

engraving::ClefType Convert::clefFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    libmei::Clef meiClef;
    meiClef.SetLine(meiStaffDef.GetClefLine());
    meiClef.SetShape(meiStaffDef.GetClefShape());
    meiClef.SetDis(meiStaffDef.GetClefDis());
    meiClef.SetDisPlace(meiStaffDef.GetClefDisPlace());
    return Convert::clefFromMEI(meiClef, warning);
}

void Convert::colorFromMEI(engraving::EngravingItem* item, const libmei::Element& meiElement)
{
    const libmei::AttColor* colorAtt = dynamic_cast<const libmei::AttColor*>(&meiElement);

    IF_ASSERT_FAILED(colorAtt) {
        return;
    }

    // @color
    if (colorAtt->HasColor()) {
        engraving::Color color = engraving::Color::fromString(colorAtt->GetColor());
        if (color.isValid()) {
            item->setColor(color);
            item->setPropertyFlags(engraving::Pid::COLOR, engraving::PropertyFlags::UNSTYLED);
        }
    }
}

void Convert::colorToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement)
{
    libmei::AttColor* colorAtt = dynamic_cast<libmei::AttColor*>(&meiElement);

    IF_ASSERT_FAILED(colorAtt) {
        return;
    }

    // @color
    if (item->color() != engravingConfiguration()->defaultColor()) {
        colorAtt->SetColor(item->color().toString());
    }
}

void Convert::colorlineFromMEI(engraving::SLine* line, const libmei::Element& meiElement)
{
    const libmei::AttColor* colorAtt = dynamic_cast<const libmei::AttColor*>(&meiElement);

    IF_ASSERT_FAILED(colorAtt) {
        return;
    }

    // @color
    if (colorAtt->HasColor()) {
        engraving::Color color = engraving::Color::fromString(colorAtt->GetColor());
        if (color.isValid()) {
            line->setLineColor(color);
            line->setPropertyFlags(engraving::Pid::COLOR, engraving::PropertyFlags::UNSTYLED);
        }
    }
}

void Convert::colorlineToMEI(const engraving::SLine* line, libmei::Element& meiElement)
{
    libmei::AttColor* colorAtt = dynamic_cast<libmei::AttColor*>(&meiElement);

    IF_ASSERT_FAILED(colorAtt) {
        return;
    }

    // @color
    if (line->lineColor() != engravingConfiguration()->defaultColor()) {
        colorAtt->SetColor(line->lineColor().toString());
    }
}

engraving::DirectionV Convert::curvedirFromMEI(const libmei::curvature_CURVEDIR meiCurvedir, bool& warning)
{
    warning = false;
    switch (meiCurvedir) {
    case (libmei::curvature_CURVEDIR_above): return engraving::DirectionV::UP;
    case (libmei::curvature_CURVEDIR_below): return engraving::DirectionV::DOWN;
    default:
        return engraving::DirectionV::AUTO;
    }
}

libmei::curvature_CURVEDIR Convert::curvedirToMEI(engraving::DirectionV curvedir)
{
    switch (curvedir) {
    case (engraving::DirectionV::UP): return libmei::curvature_CURVEDIR_above;
    case (engraving::DirectionV::DOWN): return libmei::curvature_CURVEDIR_below;
    default:
        return libmei::curvature_CURVEDIR_NONE;
    }
}

void Convert::dirFromMEI(engraving::TextBase* textBase, const StringList& meiLines, const libmei::Dir& meiDir, bool& warning)
{
    IF_ASSERT_FAILED(textBase) {
        return;
    }

    warning = false;

    // @place
    if (meiDir.HasPlace()) {
        textBase->setPlacement(meiDir.GetPlace()
                               == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        textBase->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @type
    // already process in Convert::elementTypeFor called for determining the factory to call in MeiImporter

    // text
    textBase->setXmlText(meiLines.join(u"\n"));
}

void Convert::dirFromMEI(engraving::TextLineBase* textLineBase, const StringList& meiLines, const libmei::Dir& meiDir, bool& warning)
{
    IF_ASSERT_FAILED(textLineBase) {
        return;
    }

    warning = false;

    // @place
    if (meiDir.HasPlace()) {
        textLineBase->setPlacement(meiDir.GetPlace()
                                   == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        textLineBase->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @type
    if (textLineBase->isHairpin()) {
        engraving::Hairpin* hairpin = engraving::toHairpin(textLineBase);
        engraving::HairpinType hairpinType = engraving::HairpinType::DECRESC_LINE;
        if (Convert::hasTypeValue(meiDir.GetType(), std::string(DIR_TYPE) + "cresc")) {
            hairpinType = engraving::HairpinType::CRESC_LINE;
        }
        hairpin->setHairpinType(hairpinType);
    }

    // @lform
    if (meiDir.HasLform()) {
        bool lformWarning = false;
        textLineBase->setLineStyle(Convert::lineFromMEI(meiDir.GetLform(), lformWarning));
        textLineBase->setPropertyFlags(engraving::Pid::LINE_STYLE, engraving::PropertyFlags::UNSTYLED);
        warning = (warning || lformWarning);
    }

    // text
    textLineBase->setBeginText(meiLines.join(u"\n"));
    textLineBase->setPropertyFlags(engraving::Pid::BEGIN_TEXT, engraving::PropertyFlags::UNSTYLED);
}

libmei::Dir Convert::dirToMEI(const engraving::TextBase* textBase, StringList& meiLines)
{
    libmei::Dir meiDir;

    // @place
    if (textBase->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiDir.SetPlace(Convert::placeToMEI(textBase->placement()));
    }

    // @type
    if (textBase->type() != engraving::ElementType::EXPRESSION) {
        std::string dirType = DIR_TYPE;
        switch (textBase->type()) {
        case (engraving::ElementType::PLAYTECH_ANNOTATION):
            dirType = std::string(DIR_TYPE) + "playtech-annotation";
            break;
        case (engraving::ElementType::STAFF_TEXT):
            dirType = std::string(DIR_TYPE) + "staff-text";
            break;
        default: break;
        }
        meiDir.SetType(dirType);
    }

    // text content - only split lines
    meiLines = String(textBase->plainText()).split(u"\n");

    return meiDir;
}

libmei::Dir Convert::dirToMEI(const engraving::TextLineBase* textLineBase, StringList& meiLines)
{
    libmei::Dir meiDir;

    // @place
    if (textLineBase->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiDir.SetPlace(Convert::placeToMEI(textLineBase->placement()));
    }

    // @type
    std::string dirType;
    if (textLineBase->isHairpin()) {
        // Two type values: one for hairpin, one for the hairpin type
        dirType = std::string(DIR_TYPE) + "hairpin " + std::string(DIR_TYPE);
        const engraving::Hairpin* hairpin = engraving::toHairpin(textLineBase);
        if (hairpin->hairpinType() == engraving::HairpinType::CRESC_LINE) {
            dirType += "cresc";
        } else {
            dirType += "decresc";
        }
    }
    meiDir.SetType(dirType);

    // @extender
    meiDir.SetExtender(libmei::BOOLEAN_true);

    // @lform
    if (textLineBase->propertyFlags(engraving::Pid::LINE_STYLE) == engraving::PropertyFlags::UNSTYLED) {
        meiDir.SetLform(Convert::lineToMEI(textLineBase->lineStyle()));
    }

    // text content - only split lines
    meiLines = String(textLineBase->beginText()).split(u"\n");

    return meiDir;
}

engraving::DurationType Convert::durFromMEI(const libmei::data_DURATION meiDuration, bool& warning)
{
    warning = false;
    switch (meiDuration) {
    case (libmei::DURATION_long): return engraving::DurationType::V_LONG;
    case (libmei::DURATION_breve): return engraving::DurationType::V_BREVE;
    case (libmei::DURATION_1): return engraving::DurationType::V_WHOLE;
    case (libmei::DURATION_2): return engraving::DurationType::V_HALF;
    case (libmei::DURATION_4): return engraving::DurationType::V_QUARTER;
    case (libmei::DURATION_8): return engraving::DurationType::V_EIGHTH;
    case (libmei::DURATION_16): return engraving::DurationType::V_16TH;
    case (libmei::DURATION_32): return engraving::DurationType::V_32ND;
    case (libmei::DURATION_64): return engraving::DurationType::V_64TH;
    case (libmei::DURATION_128): return engraving::DurationType::V_128TH;
    case (libmei::DURATION_256): return engraving::DurationType::V_256TH;
    case (libmei::DURATION_512): return engraving::DurationType::V_512TH;
    case (libmei::DURATION_1024): return engraving::DurationType::V_1024TH;
    default:
        warning = true;
        return engraving::DurationType::V_QUARTER;
    }
}

libmei::data_DURATION Convert::durToMEI(const engraving::DurationType duration)
{
    switch (duration) {
    case (engraving::DurationType::V_LONG): return libmei::DURATION_long;
    case (engraving::DurationType::V_BREVE): return libmei::DURATION_breve;
    case (engraving::DurationType::V_WHOLE): return libmei::DURATION_1;
    case (engraving::DurationType::V_HALF): return libmei::DURATION_2;
    case (engraving::DurationType::V_QUARTER): return libmei::DURATION_4;
    case (engraving::DurationType::V_EIGHTH): return libmei::DURATION_8;
    case (engraving::DurationType::V_16TH): return libmei::DURATION_16;
    case (engraving::DurationType::V_32ND): return libmei::DURATION_32;
    case (engraving::DurationType::V_64TH): return libmei::DURATION_64;
    case (engraving::DurationType::V_128TH): return libmei::DURATION_128;
    case (engraving::DurationType::V_256TH): return libmei::DURATION_256;
    case (engraving::DurationType::V_512TH): return libmei::DURATION_512;
    case (engraving::DurationType::V_1024TH): return libmei::DURATION_1024;
    default:
        return libmei::DURATION_4;
    }
}

void Convert::dynamFromMEI(engraving::Dynamic* dynamic, const StringList& meiLines, const libmei::Dynam& meiDynam, bool& warning)
{
    // The letters in the MEI to be mapped to SMuFL symbols in the xmlText
    static const std::map<Char, engraving::SymId> DYN_MAP = {
        { 'p', engraving::SymId::dynamicPiano },
        { 'm', engraving::SymId::dynamicMezzo },
        { 'f', engraving::SymId::dynamicForte },
        { 'r', engraving::SymId::dynamicRinforzando },
        { 's', engraving::SymId::dynamicSforzando },
        { 'z', engraving::SymId::dynamicZ },
        { 'n', engraving::SymId::dynamicNiente },
    };

    IF_ASSERT_FAILED(dynamic) {
        return;
    }

    warning = false;

    // @place
    dynamic->setProperty(engraving::Pid::DIRECTION, Convert::directionFromMEI(meiDynam.GetPlace()));

    // @label
    if (meiDynam.HasLabel()) {
        dynamic->setDynamicType(String::fromStdString(meiDynam.GetLabel()));
    }
    // If no @label, try to use the text
    else if (meiLines.size() > 0 && !meiLines.at(0).contains(' ')) {
        dynamic->setDynamicType(meiLines.at(0));
    }

    // @layer
    Convert::layerIdentFromMEI(dynamic, meiDynam);

    // text content
    StringList lines;
    // For each line in the dynamic text
    for (const String& meiline : meiLines) {
        String line;
        StringList words = meiline.split(u" ");
        bool isFirst = true;
        // For each word in the line
        for (const String& word : words) {
            if (!isFirst) {
                line += u" ";
            }
            // If the word is only dynamic letters, convert them to SMuFL symbols one by one
            if (word.toStdString().find_first_not_of("fpmrszn") == std::string::npos) {
                for (size_t i = 0; i < word.size(); i++) {
                    line += u"<sym>" + String::fromAscii(engraving::SymNames::nameForSymId(DYN_MAP.at(word.at(i))).ascii()) + u"</sym>";
                }
            }
            // Otherwise keep it as is
            else {
                line += word;
            }
            isFirst = false;
        }
        lines.push_back(line);
    }
    dynamic->setXmlText(lines.join(u"\n"));

    // @color
    Convert::colorFromMEI(dynamic, meiDynam);
}

libmei::Dynam Convert::dynamToMEI(const engraving::Dynamic* dynamic, StringList& meiLines)
{
    // The SMuFL unicode points in the plainText to be mapped to letters in the MEI
    static const std::map<char16_t, Char> DYN_MAP = {
        { u'\uE520', 'p' },
        { u'\uE521', 'm' },
        { u'\uE522', 'f' },
        { u'\uE523', 'z' },
        { u'\uE524', 's' },
        { u'\uE525', 'z' },
        { u'\uE526', 'n' }
    };

    libmei::Dynam meiDynam;

    // @place
    meiDynam.SetPlace(Convert::directionToMEI(dynamic->direction()));

    // @layer
    Convert::layerIdentToMEI(dynamic, meiDynam);

    // @staff
    Convert::staffIdentToMEI(dynamic, meiDynam);

    // @label
    if (dynamic->dynamicType() != engraving::DynamicType::OTHER) {
        meiDynam.SetLabel(engraving::TConv::toXml(dynamic->dynamicType()).ascii());
    }

    // text content
    String meiText;
    String plainText = dynamic->plainText();
    for (size_t i = 0; i < plainText.size(); i++) {
        char16_t c = plainText.at(i).unicode();
        if (c < u'\uE000' || c > u'\uF8FF' || !DYN_MAP.count(c)) {
            meiText += c;
            continue;
        }
        meiText += DYN_MAP.at(c);

        /*
        char16_t c = plainText.at(i).unicode();
        if (c < u'\uE000' || c > u'\uF8FF') {
            meiText += c;
            continue;
        }
        SymId symId = m_score->engravingFont()->fromCode(c);
        if (symId == SymId::noSym) continue;
        String letter = u"<sym>" + String::fromAscii(SymNames::nameForSymId(symId).ascii()) + u"</sym>";
        for (auto dyn : Dynamic::dynamicList()) {
            if ((dyn.text == letter) && (dyn.type != DynamicType::OTHER)) {
                meiText += String::fromAscii(TConv::toXml(dyn.type).ascii());
                break;
            }
        }
        */
    }
    meiLines = String(meiText).split(u"\n");

    // @color
    Convert::colorToMEI(dynamic, meiDynam);

    return meiDynam;
}

void Convert::endingFromMEI(engraving::Volta* volta, const libmei::Ending& meiEnding, bool& warning)
{
    IF_ASSERT_FAILED(volta) {
        return;
    }

    warning = false;
    // @type used for storing endings
    std::list<std::string> endings = Convert::getTypeValuesWithPrefix(meiEnding.GetType(), ENDING_REPEAT_TYPE);
    for (std::string ending : endings) {
        volta->endings().push_back(String(ending.c_str()).toInt());
    }

    // @label
    if (meiEnding.HasLabel()) {
        volta->setText(String(meiEnding.GetLabel().c_str()));
    }

    // @lform
    if (meiEnding.HasLform() && meiEnding.GetLform() != libmei::LINEFORM_solid) {
        bool lineWarning;
        engraving::LineType lineType = Convert::lineFromMEI(meiEnding.GetLform(), lineWarning);
        if (!lineWarning) {
            volta->setLineStyle(lineType);
            volta->setPropertyFlags(engraving::Pid::LINE_STYLE, engraving::PropertyFlags::UNSTYLED);
        }
        warning = (warning || lineWarning);
    }

    // @lendsym
    if (meiEnding.HasLendsym() && (meiEnding.GetLendsym() == libmei::LINESTARTENDSYMBOL_none)) {
        volta->setVoltaType(engraving::Volta::Type::OPEN);
        volta->setPropertyFlags(engraving::Pid::VOLTA_ENDING, engraving::PropertyFlags::UNSTYLED);
    } else {
        volta->setVoltaType(engraving::Volta::Type::CLOSED);
    }
}

libmei::Ending Convert::endingToMEI(const engraving::Volta* volta)
{
    libmei::Ending meiEnding;

    // @type used for storing endings
    StringList endings;
    for (int ending : volta->endings()) {
        endings << String("%1%2").arg(String(ENDING_REPEAT_TYPE)).arg(ending);
    }
    meiEnding.SetType(endings.join(u" ").toStdString());

    // @label used for text
    meiEnding.SetLabel(volta->text().toStdString());

    // @lform
    if (volta->lineStyle() != engraving::LineType::SOLID) {
        meiEnding.SetLform(Convert::lineToMEI(volta->lineStyle()));
    }

    // @lendsym
    if (volta->voltaType() == engraving::Volta::Type::OPEN) {
        meiEnding.SetLendsym(libmei::LINESTARTENDSYMBOL_none);
    }

    return meiEnding;
}

void Convert::fFromMEI(engraving::FiguredBassItem* figuredBassItem, const StringList& meiLines, const libmei::F& meiF, bool& warning)
{
    warning = false;

    // content
    String text = meiLines.join(u"\n");
    figuredBassItem->parse(text);

    UNUSED(meiF);
}

libmei::F Convert::fToMEI(const engraving::FiguredBassItem* figuredBassItem, StringList& meiLines)
{
    libmei::F meiF;

    // content - to be improved
    String plainText = figuredBassItem->normalizedText();
    meiLines = plainText.split(u"\n");

    return meiF;
}

void Convert::fbFromMEI(engraving::FiguredBass* figuredBass, const libmei::Harm& meiHarm, const libmei::Fb& meiFb, bool& warning)
{
    warning = false;

    UNUSED(figuredBass);
    UNUSED(meiHarm);
    UNUSED(meiFb);
}

std::pair<libmei::Harm, libmei::Fb> Convert::fbToMEI(const engraving::FiguredBass* figuredBass)
{
    libmei::Harm meiHarm;
    libmei::Fb meiFb;

    UNUSED(figuredBass);

    return { meiHarm, meiFb };
}

void Convert::fermataFromMEI(engraving::Fermata* fermata, const libmei::Fermata& meiFermata, bool& warning)
{
    warning = false;

    // @place
    bool below = (meiFermata.HasPlace() && (meiFermata.GetPlace() == libmei::STAFFREL_below));

    // @glyph.name
    bool smufl = (meiFermata.HasGlyphAuth() && (meiFermata.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::fermataAbove;

    if (smufl && meiFermata.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiFermata.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    } else if (meiFermata.HasShape()) {
        if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_square) {
            symId = (below) ? engraving::SymId::fermataLongBelow : engraving::SymId::fermataLongAbove;
        } else if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_angular) {
            symId = (below) ? engraving::SymId::fermataShortBelow : engraving::SymId::fermataShortAbove;
        } else if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_curved) {
            symId = (below) ? engraving::SymId::fermataBelow : engraving::SymId::fermataAbove;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiFermata.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiFermata.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    } else if (below) {
        symId = engraving::SymId::fermataBelow;
    }

    fermata->setSymId(symId);
    if (below) {
        fermata->setPlacement(engraving::PlacementV::BELOW);
    }

    // @color
    Convert::colorFromMEI(fermata, meiFermata);
}

libmei::Fermata Convert::fermataToMEI(const engraving::Fermata* fermata)
{
    libmei::Fermata meiFermata;

    bool below = false;
    bool smufl = false;
    // @shape and @glyph.name
    switch (fermata->symId()) {
    case (engraving::SymId::fermataAbove): break;
    case (engraving::SymId::fermataBelow): below = true;
        break;
    case (engraving::SymId::fermataLongAbove): meiFermata.SetShape(libmei::fermataVis_SHAPE_square);
        break;
    case (engraving::SymId::fermataLongBelow): meiFermata.SetShape(libmei::fermataVis_SHAPE_square);
        below = true;
        break;
    case (engraving::SymId::fermataShortAbove): meiFermata.SetShape(libmei::fermataVis_SHAPE_angular);
        break;
    case (engraving::SymId::fermataShortBelow): meiFermata.SetShape(libmei::fermataVis_SHAPE_angular);
        below = true;
        break;
    case (engraving::SymId::fermataLongHenzeAbove):
    case (engraving::SymId::fermataVeryLongAbove):
    case (engraving::SymId::fermataShortHenzeAbove):
    case (engraving::SymId::fermataVeryShortAbove): smufl = true;
        break;
    case (engraving::SymId::fermataLongHenzeBelow):
    case (engraving::SymId::fermataVeryLongBelow):
    case (engraving::SymId::fermataShortHenzeBelow):
    case (engraving::SymId::fermataVeryShortBelow): smufl = true;
        below = true;
        break;
    default:
        break;
    }
    // @place
    if (below) {
        meiFermata.SetPlace(libmei::STAFFREL_below);
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(fermata->symId());
        meiFermata.SetGlyphName(glyphName.ascii());
        meiFermata.SetGlyphAuth(SMUFL_AUTH);
    }

    // @color
    Convert::colorToMEI(fermata, meiFermata);

    return meiFermata;
}

void Convert::fingFromMEI(engraving::Fingering* fing, const StringList& meiLines, const libmei::Fing& meiFing, bool& warning)
{
    IF_ASSERT_FAILED(fing) {
        return;
    }

    warning = false;

    // text content
    fing->setPlainText(meiLines.join(u"\n"));

    // @place
    if (meiFing.HasPlace()) {
        fing->setPlacement(meiFing.GetPlace() == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        fing->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @color
    Convert::colorFromMEI(fing, meiFing);
}

libmei::Fing Convert::fingToMEI(const engraving::Fingering* fing, StringList& meiLines)
{
    libmei::Fing meiFing;

    // content
    String plainText = fing->plainText();
    meiLines = plainText.split(u"\n");

    // @place
    if (fing->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiFing.SetPlace(Convert::placeToMEI(fing->placement()));
    }

    // @color
    Convert::colorToMEI(fing, meiFing);

    return meiFing;
}

std::pair<bool, engraving::NoteType> Convert::gracegrpFromMEI(const libmei::graceGrpLog_ATTACH meiAttach, const libmei::data_GRACE meiGrace,
                                                              bool& warning)
{
    warning = false;
    bool isAfter = (meiAttach == libmei::graceGrpLog_ATTACH_pre);
    engraving::NoteType noteType = engraving::NoteType::APPOGGIATURA;

    if (isAfter) {
        noteType = engraving::NoteType::GRACE8_AFTER;
    } else if (meiGrace == libmei::GRACE_unacc) {
        noteType = engraving::NoteType::ACCIACCATURA;
    }

    return { isAfter, noteType };
}

std::pair<libmei::graceGrpLog_ATTACH, libmei::data_GRACE> Convert::gracegrpToMEI(bool isAfter, engraving::NoteType noteType)
{
    libmei::graceGrpLog_ATTACH meiAttach = (isAfter) ? libmei::graceGrpLog_ATTACH_pre : libmei::graceGrpLog_ATTACH_NONE;
    libmei::data_GRACE meiGrace = libmei::GRACE_acc;

    if (isAfter) {
        meiGrace = libmei::GRACE_unknown;
    } else if (noteType == engraving::NoteType::ACCIACCATURA) {
        meiGrace = libmei::GRACE_unacc;
    }

    return { meiAttach, meiGrace };
}

void Convert::hairpinFromMEI(engraving::Hairpin* hairpin, const libmei::Hairpin& meiHairpin, bool& warning)
{
    warning = false;

    // @place
    hairpin->setProperty(engraving::Pid::DIRECTION, Convert::directionFromMEI(meiHairpin.GetPlace()));

    // @form
    if (meiHairpin.GetForm() == libmei::hairpinLog_FORM_cres) {
        hairpin->setHairpinType(engraving::HairpinType::CRESC_HAIRPIN);
    } else {
        hairpin->setHairpinType(engraving::HairpinType::DECRESC_HAIRPIN);
    }

    // @lform
    if (meiHairpin.HasLform()) {
        bool lformWarning = false;
        hairpin->setLineStyle(Convert::lineFromMEI(meiHairpin.GetLform(), lformWarning));
        hairpin->setPropertyFlags(engraving::Pid::LINE_STYLE, engraving::PropertyFlags::UNSTYLED);
        warning = (warning || lformWarning);
    }

    // @color
    Convert::colorlineFromMEI(hairpin, meiHairpin);

    // @layer
    Convert::layerIdentFromMEI(hairpin, meiHairpin);
}

libmei::Hairpin Convert::hairpinToMEI(const engraving::Hairpin* hairpin)
{
    libmei::Hairpin meiHairpin;

    // @place
    meiHairpin.SetPlace(Convert::directionToMEI(hairpin->direction()));

    // @form
    if (hairpin->hairpinType() == engraving::HairpinType::CRESC_HAIRPIN) {
        meiHairpin.SetForm(libmei::hairpinLog_FORM_cres);
    } else {
        meiHairpin.SetForm(libmei::hairpinLog_FORM_dim);
    }

    // @lform
    if (hairpin->lineStyle() != engraving::LineType::SOLID) {
        meiHairpin.SetLform(Convert::lineToMEI(hairpin->lineStyle()));
    }

    // @color
    Convert::colorlineToMEI(hairpin, meiHairpin);

    // @layer
    Convert::layerIdentToMEI(hairpin, meiHairpin);

    // @staff
    Convert::staffIdentToMEI(hairpin, meiHairpin);

    return meiHairpin;
}

void Convert::harmFromMEI(engraving::Harmony* harmony, const StringList& meiLines, const libmei::Harm& meiHarm, bool& warning)
{
    IF_ASSERT_FAILED(harmony) {
        return;
    }

    warning = false;

    // @type
    engraving::HarmonyType harmonyType = engraving::HarmonyType::STANDARD;
    if (Convert::hasTypeValue(meiHarm.GetType(), std::string(HARMONY_TYPE) + "roman")) {
        harmonyType = engraving::HarmonyType::ROMAN;
    }

    // text content
    harmony->setHarmonyType(harmonyType);
    harmony->setHarmony(meiLines.join(u"\n"));
    harmony->setPlainText(harmony->harmonyName());

    // @place
    if (meiHarm.HasPlace()) {
        harmony->setPlacement(meiHarm.GetPlace() == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        harmony->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @color
    Convert::colorFromMEI(harmony, meiHarm);
}

libmei::Harm Convert::harmToMEI(const engraving::Harmony* harmony, StringList& meiLines)
{
    libmei::Harm meiHarm;

    // @type
    if (harmony->harmonyType() != engraving::HarmonyType::STANDARD) {
        std::string harmonyType = HARMONY_TYPE;

        switch (harmony->harmonyType()) {
        case (engraving::HarmonyType::ROMAN):
            harmonyType = std::string(HARMONY_TYPE) + "roman";
            break;
        default: break;
        }
        meiHarm.SetType(harmonyType);
    }

    // content
    String plainText = harmony->plainText();
    meiLines = plainText.split(u"\n");

    // @place
    if (harmony->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiHarm.SetPlace(Convert::placeToMEI(harmony->placement()));
    }

    // @color
    Convert::colorToMEI(harmony, meiHarm);

    return meiHarm;
}

void Convert::harpPedalFromMEI(engraving::HarpPedalDiagram* harpPedalDiagram, const libmei::HarpPedal& meiHarpPedal, bool& warning)
{
    IF_ASSERT_FAILED(harpPedalDiagram) {
        return;
    }

    warning = false;

    harpPedalDiagram->setIsDiagram(true);

    // @d
    harpPedalDiagram->setPedal(engraving::HarpStringType::D, harpPedalPositionFromMEI(meiHarpPedal.GetD()));
    // @c
    harpPedalDiagram->setPedal(engraving::HarpStringType::C, harpPedalPositionFromMEI(meiHarpPedal.GetC()));
    // @b
    harpPedalDiagram->setPedal(engraving::HarpStringType::B, harpPedalPositionFromMEI(meiHarpPedal.GetB()));
    // @e
    harpPedalDiagram->setPedal(engraving::HarpStringType::E, harpPedalPositionFromMEI(meiHarpPedal.GetE()));
    // @f
    harpPedalDiagram->setPedal(engraving::HarpStringType::F, harpPedalPositionFromMEI(meiHarpPedal.GetF()));
    // @g
    harpPedalDiagram->setPedal(engraving::HarpStringType::G, harpPedalPositionFromMEI(meiHarpPedal.GetG()));
    // @a
    harpPedalDiagram->setPedal(engraving::HarpStringType::A, harpPedalPositionFromMEI(meiHarpPedal.GetA()));

    // @place
    if (meiHarpPedal.HasPlace()) {
        harpPedalDiagram->setPlacement(meiHarpPedal.GetPlace()
                                       == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        harpPedalDiagram->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @color
    Convert::colorFromMEI(harpPedalDiagram, meiHarpPedal);
}

libmei::HarpPedal Convert::harpPedalToMEI(const engraving::HarpPedalDiagram* harpPedalDiagram)
{
    libmei::HarpPedal meiHarpPedal;

    // @d
    meiHarpPedal.SetD(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(0)));
    // @c
    meiHarpPedal.SetC(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(1)));
    // @b
    meiHarpPedal.SetB(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(2)));
    // @e
    meiHarpPedal.SetE(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(3)));
    // @f
    meiHarpPedal.SetF(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(4)));
    // @g
    meiHarpPedal.SetG(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(5)));
    // @a
    meiHarpPedal.SetA(harpPedalPositionToMEI(harpPedalDiagram->getPedalState().at(6)));

    // @place
    if (harpPedalDiagram->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiHarpPedal.SetPlace(Convert::placeToMEI(harpPedalDiagram->placement()));
    }

    // @color
    Convert::colorToMEI(harpPedalDiagram, meiHarpPedal);

    // @staff
    Convert::staffIdentToMEI(harpPedalDiagram, meiHarpPedal);

    return meiHarpPedal;
}

libmei::data_HARPPEDALPOSITION Convert::harpPedalPositionToMEI(const engraving::PedalPosition& pedalPosition)
{
    switch (pedalPosition) {
    case engraving::PedalPosition::FLAT:
        return libmei::HARPPEDALPOSITION_f;
    case engraving::PedalPosition::NATURAL:
        return libmei::HARPPEDALPOSITION_n;
    case engraving::PedalPosition::SHARP:
        return libmei::HARPPEDALPOSITION_s;
    case engraving::PedalPosition::UNSET:
    default:
        return libmei::HARPPEDALPOSITION_NONE;
    }
}

engraving::PedalPosition Convert::harpPedalPositionFromMEI(const libmei::data_HARPPEDALPOSITION& pedalPosition)
{
    switch (pedalPosition) {
    case libmei::HARPPEDALPOSITION_f:
        return engraving::PedalPosition::FLAT;
    case libmei::HARPPEDALPOSITION_n:
        return engraving::PedalPosition::NATURAL;
    case libmei::HARPPEDALPOSITION_s:
        return engraving::PedalPosition::SHARP;
    default:
        return engraving::PedalPosition::UNSET;
    }
}

void Convert::lvFromMEI(engraving::LaissezVib* lv, const libmei::Lv& meiLv, bool& warning)
{
    warning = false;

    // @curvedir
    if (meiLv.HasCurvedir()) {
        lv->setSlurDirection(Convert::curvedirFromMEI(meiLv.GetCurvedir(), warning));
    }

    // @lform
    if (meiLv.HasLform()) {
        bool typeWarning = false;
        lv->setStyleType(Convert::slurstyleFromMEI(meiLv.GetLform(), typeWarning));
        warning = (warning || typeWarning);
    }

    // @color
    Convert::colorFromMEI(lv, meiLv);
}

void Convert::jumpFromMEI(engraving::Jump* jump, const libmei::RepeatMark& meiRepeatMark, bool& warning)
{
    warning = false;

    engraving::JumpType jumpType;

    // @func
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_daCapo): jumpType = engraving::JumpType::DC;
        break;
    case (libmei::repeatMarkLog_FUNC_dalSegno): jumpType = engraving::JumpType::DS;
        break;
    default:
        jumpType = engraving::JumpType::DC;
    }

    // @type
    if (meiRepeatMark.HasType()) {
        std::list<std::string> jumpTypes = Convert::getTypeValuesWithPrefix(meiRepeatMark.GetType(), JUMP_TYPE);
        if (jumpTypes.size() > 0) {
            std::string value = jumpTypes.front();
            auto result = std::find_if(Convert::s_jumpTypes.begin(), Convert::s_jumpTypes.end(),
                                       [value](const auto& entry) { return entry.second == value; });

            if (result != Convert::s_jumpTypes.end()) {
                jumpType = result->first;
            } else {
                warning = true;
            }
        }
    }

    // @color
    Convert::colorFromMEI(jump, meiRepeatMark);

    jump->setJumpType(jumpType);
}

libmei::RepeatMark Convert::jumpToMEI(const engraving::Jump* jump, String& text)
{
    libmei::RepeatMark meiRepeatMark;

    // @func
    switch (jump->jumpType()) {
    case (engraving::JumpType::DC):
    case (engraving::JumpType::DC_AL_FINE):
    case (engraving::JumpType::DC_AL_CODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
        break;
    case (engraving::JumpType::DS_AL_CODA):
    case (engraving::JumpType::DS_AL_FINE):
    case (engraving::JumpType::DS): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_dalSegno);
        break;
    case (engraving::JumpType::DC_AL_DBLCODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
        break;
    case (engraving::JumpType::DS_AL_DBLCODA):
    case (engraving::JumpType::DSS):
    case (engraving::JumpType::DSS_AL_CODA):
    case (engraving::JumpType::DSS_AL_DBLCODA):
    case (engraving::JumpType::DSS_AL_FINE): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_dalSegno);
        break;
    default:
        meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
    }

    // @type
    if (Convert::s_jumpTypes.count(jump->jumpType())) {
        meiRepeatMark.SetType(JUMP_TYPE + Convert::s_jumpTypes.at(jump->jumpType()));
    }

    switch (jump->jumpType()) {
    // No text for the default symbols
    case (engraving::JumpType::DC):
    case (engraving::JumpType::DS):
        break;
    default:
        text = jump->plainText();
    }

    // @color
    Convert::colorToMEI(jump, meiRepeatMark);

    return meiRepeatMark;
}

engraving::Key Convert::keyFromMEI(const libmei::data_KEYSIGNATURE& meiKeysig, bool& warning)
{
    warning = false;
    if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_s) {
        return engraving::Key(meiKeysig.first);
    } else if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_f) {
        return engraving::Key(-meiKeysig.first);
    } else if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_n) {
        return engraving::Key(0);
    }
    warning = true;
    return engraving::Key(0);
}

libmei::data_KEYSIGNATURE Convert::keyToMEI(const engraving::Key key)
{
    if (key > 0) {
        return std::make_pair(std::min(static_cast<int>(key), 7), libmei::ACCIDENTAL_WRITTEN_s);
    } else if (key < 0) {
        return std::make_pair(std::abs(std::max(static_cast<int>(key), -7)), libmei::ACCIDENTAL_WRITTEN_f);
    } else {
        return std::make_pair(0, libmei::ACCIDENTAL_WRITTEN_n);
    }
}

engraving::LineType Convert::lineFromMEI(const libmei::data_LINEFORM meiLine, bool& warning)
{
    warning = false;
    switch (meiLine) {
    case (libmei::LINEFORM_solid): return engraving::LineType::SOLID;
    case (libmei::LINEFORM_dashed): return engraving::LineType::DASHED;
    case (libmei::LINEFORM_dotted): return engraving::LineType::DOTTED;
    case (libmei::LINEFORM_wavy):
        warning = true;
        return engraving::LineType::SOLID;
    default:
        return engraving::LineType::SOLID;
    }
}

libmei::data_LINEFORM Convert::lineToMEI(engraving::LineType line)
{
    switch (line) {
    case (engraving::LineType::SOLID): return libmei::LINEFORM_solid;
    case (engraving::LineType::DASHED): return libmei::LINEFORM_dashed;
    case (engraving::LineType::DOTTED): return libmei::LINEFORM_dotted;
    default:
        return libmei::LINEFORM_NONE;
    }
}

void Convert::markerFromMEI(engraving::Marker* marker, const libmei::RepeatMark& meiRepeatMark, bool& warning)
{
    warning = false;

    engraving::MarkerType markerType;

    // @func
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_segno): markerType = engraving::MarkerType::SEGNO;
        break;
    case (libmei::repeatMarkLog_FUNC_coda): markerType = engraving::MarkerType::CODA;
        break;
    case (libmei::repeatMarkLog_FUNC_fine): markerType = engraving::MarkerType::FINE;
        break;
    default:
        markerType = engraving::MarkerType::SEGNO;
    }

    // @type
    if (meiRepeatMark.HasType()) {
        std::list<std::string> markerTypes = Convert::getTypeValuesWithPrefix(meiRepeatMark.GetType(), MARKER_TYPE);
        if (markerTypes.size() > 0) {
            std::string value = markerTypes.front();
            auto result = std::find_if(Convert::s_markerTypes.begin(), Convert::s_markerTypes.end(),
                                       [value](const auto& entry) { return entry.second == value; });

            if (result != Convert::s_markerTypes.end()) {
                markerType = result->first;
            } else {
                warning = true;
            }
        }
    }

    marker->setMarkerType(markerType);

    // @color
    Convert::colorFromMEI(marker, meiRepeatMark);
}

libmei::RepeatMark Convert::markerToMEI(const engraving::Marker* marker, String& text)
{
    libmei::RepeatMark meiRepeatMark;

    // @func
    switch (marker->markerType()) {
    case (engraving::MarkerType::SEGNO):
    case (engraving::MarkerType::VARSEGNO): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_segno);
        break;
    case (engraving::MarkerType::CODA):
    case (engraving::MarkerType::VARCODA):
    case (engraving::MarkerType::CODETTA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_coda);
        break;
    case (engraving::MarkerType::FINE): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_fine);
        break;
    case (engraving::MarkerType::TOCODA):
    case (engraving::MarkerType::TOCODASYM):
    case (engraving::MarkerType::DA_CODA):
    case (engraving::MarkerType::DA_DBLCODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_coda);
        break;
    default:
        meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_segno);
    }

    // @type
    if (Convert::s_markerTypes.count(marker->markerType())) {
        meiRepeatMark.SetType(MARKER_TYPE + Convert::s_markerTypes.at(marker->markerType()));
    }

    switch (marker->markerType()) {
    // No text for the default symbols
    case (engraving::MarkerType::SEGNO):
    case (engraving::MarkerType::CODA):
    case (engraving::MarkerType::CODETTA):
    case (engraving::MarkerType::FINE):
        break;
    case (engraving::MarkerType::VARCODA):
    case (engraving::MarkerType::VARSEGNO):
        // Here we could use @glyph.auth and @glyph.name, but they are not included in MEI Basic
        break;
    case (engraving::MarkerType::TOCODASYM):
        text = "To 𝄌";
        break;
    default:
        text = marker->plainText();
    }

    // @color
    Convert::colorToMEI(marker, meiRepeatMark);

    return meiRepeatMark;
}

Convert::MeasureStruct Convert::measureFromMEI(const libmei::Measure& meiMeasure, bool& warning)
{
    warning = false;
    MeasureStruct measureSt;

    // for now only rely on the presence of @n
    measureSt.irregular = (!meiMeasure.HasN());

    if (meiMeasure.HasN()) {
        measureSt.n = std::stoi(meiMeasure.GetN()) - 1;
        // Make sure we have no measure number below 0;
        measureSt.n = std::max(0, measureSt.n);
    }

    if (meiMeasure.HasLeft()) {
        bool barlineWarning = false;
        engraving::BarLineType leftBarlineType = Convert::barlineFromMEI(meiMeasure.GetLeft(), barlineWarning);
        measureSt.repeatStart = (leftBarlineType == engraving::BarLineType::START_REPEAT);
        warning = (warning || barlineWarning);
    }

    if (meiMeasure.HasRight()) {
        bool barlineWarning = false;
        measureSt.endBarLineType = Convert::barlineFromMEI(meiMeasure.GetRight(), barlineWarning);
        warning = (warning || barlineWarning);
        // the measure will be flagged as repeatEnd and the BarLineType will not have to be set
        measureSt.repeatEnd
            = (std::set<engraving::BarLineType> { engraving::BarLineType::END_REPEAT, engraving::BarLineType::END_START_REPEAT }).count(
                  measureSt.endBarLineType) > 0;
    }

    if (meiMeasure.HasType()) {
        std::list<std::string> repeats = Convert::getTypeValuesWithPrefix(meiMeasure.GetType(), MEASURE_REPEAT_TYPE);
        if (repeats.size() > 0) {
            measureSt.repeatCount = String(repeats.front().c_str()).toInt();
            // make sure it is not smaller than 0
            measureSt.repeatCount = std::max(0, measureSt.repeatCount);
        }
    }

    return measureSt;
}

libmei::Measure Convert::measureToMEI(const engraving::Measure* measure, int& measureN, bool& wasPreviousIrregular)
{
    libmei::Measure meiMeasure;

    // @metcon
    if (measure->ticks() != measure->timesig()) {
        meiMeasure.SetMetcon(libmei::BOOLEAN_false);
    }
    // @n
    if (!measure->irregular()) {
        measureN++;
        meiMeasure.SetN(String::number(measure->no() + 1).toStdString());
    }
    // @left
    if (measure->repeatStart()) {
        meiMeasure.SetLeft(libmei::BARRENDITION_rptstart);
    }

    // @type used for storing measure repeat count
    if (measure->repeatEnd() && (measure->repeatCount() != 0)) {
        meiMeasure.SetType(String(u"%1%2").arg(String(MEASURE_REPEAT_TYPE)).arg(measure->repeatCount()).toStdString());
    }

    // @right
    meiMeasure.SetRight(Convert::barlineToMEI(measure->endBarLineType()));

    // update the flag for the next measure
    wasPreviousIrregular = (measure->irregular());

    return meiMeasure;
}

std::pair<engraving::Fraction, engraving::TimeSigType> Convert::meterFromMEI(const libmei::ScoreDef& meiScoreDef, bool& warning)
{
    libmei::StaffDef meiStaffDef;
    meiStaffDef.SetMeterSym(meiScoreDef.GetMeterSym());
    meiStaffDef.SetMeterUnit(meiScoreDef.GetMeterUnit());
    meiStaffDef.SetMeterCount(meiScoreDef.GetMeterCount());
    return meterFromMEI(meiStaffDef, warning);
}

std::pair<engraving::Fraction, engraving::TimeSigType> Convert::meterFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    warning = false;
    engraving::Fraction fraction;
    engraving::TimeSigType tsType = engraving::TimeSigType::NORMAL;
    if (meiStaffDef.HasMeterCount() && meiStaffDef.HasMeterUnit()) {
        auto [counts, sign] = meiStaffDef.GetMeterCount();
        int numerator = !counts.empty() ? counts.front() : (meiStaffDef.GetMeterSym() == libmei::METERSIGN_common) ? 4 : 2;
        fraction.set(numerator, meiStaffDef.GetMeterUnit());
    }
    if (meiStaffDef.HasMeterSym()) {
        if (meiStaffDef.GetMeterSym() == libmei::METERSIGN_common) {
            tsType = engraving::TimeSigType::FOUR_FOUR;
            fraction.set(4, 4);
        } else {
            tsType = engraving::TimeSigType::ALLA_BREVE;
            fraction.set(2, 2);
        }
    }
    if (fraction.isZero()) {
        fraction.set(4, 4);
        warning = true;
    }
    return { fraction, tsType };
}

libmei::StaffDef Convert::meterToMEI(const engraving::Fraction& fraction, engraving::TimeSigType tsType)
{
    libmei::StaffDef meiStaffDef;
    if (tsType == engraving::TimeSigType::FOUR_FOUR) {
        meiStaffDef.SetMeterSym(libmei::METERSIGN_common);
    } else if (tsType == engraving::TimeSigType::ALLA_BREVE) {
        meiStaffDef.SetMeterSym(libmei::METERSIGN_cut);
    } else {
        meiStaffDef.SetMeterCount({ { fraction.numerator() }, libmei::MeterCountSign::None });
        meiStaffDef.SetMeterUnit(fraction.denominator());
    }
    return meiStaffDef;
}

Convert::OrnamStruct Convert::mordentFromMEI(engraving::Ornament* ornament, const libmei::Mordent& meiMordent, bool& warning)
{
    // @glyph.name
    bool smufl = (meiMordent.HasGlyphAuth() && (meiMordent.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::ornamentMordent;

    // We give @glyph.name (or @glyph.num) priority over @form and @long
    if (smufl && meiMordent.HasGlyphName()) {
        // Check if the @glyph.name needs to be mapped to a composite glyph
        std::string value = meiMordent.GetGlyphName();
        auto result = std::find_if(Convert::s_mordentGlyphs.begin(), Convert::s_mordentGlyphs.end(),
                                   [value](const auto& entry) { return entry.second == value; });
        if (result != Convert::s_mordentGlyphs.end()) {
            symId = result->first;
            // Otherwise get the symId from the @glyph.name
        } else {
            symId = engraving::SymNames::symIdByName(String(meiMordent.GetGlyphName().c_str()));
        }
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiMordent.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiMordent.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // @form and @long only
    else {
        bool isLower = (meiMordent.HasForm() && (meiMordent.GetForm() == libmei::mordentLog_FORM_lower));
        bool isLong = (meiMordent.HasLong() && (meiMordent.GetLong() == libmei::BOOLEAN_true));
        if (isLower && isLong) {
            symId = engraving::SymId::ornamentPrallMordent;
        } else if (isLower && !isLong) {
            symId = engraving::SymId::ornamentMordent;
        } else if (!isLower && isLong) {
            symId = engraving::SymId::ornamentTremblement;
        } else {
            symId = engraving::SymId::ornamentShortTrill;
        }
    }

    ornament->setSymId(symId);

    // @color
    Convert::colorFromMEI(ornament, meiMordent);

    // Other attributes
    return Convert::ornamFromMEI(ornament, meiMordent, warning);
}

libmei::Mordent Convert::mordentToMEI(const engraving::Ornament* ornament)
{
    libmei::Mordent meiMordent;

    Convert::ornamToMEI(ornament, meiMordent);

    // @form
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentMordent):
    case (engraving::SymId::ornamentPrallMordent):
    case (engraving::SymId::ornamentUpMordent):
    case (engraving::SymId::ornamentDownMordent):
        meiMordent.SetForm(libmei::mordentLog_FORM_lower);
        break;
    case (engraving::SymId::ornamentShortTrill):
    case (engraving::SymId::ornamentTremblement):
    case (engraving::SymId::ornamentUpPrall):
    case (engraving::SymId::ornamentPrecompMordentUpperPrefix):
    case (engraving::SymId::ornamentPrallDown):
    case (engraving::SymId::ornamentPrallUp):
    case (engraving::SymId::ornamentLinePrall):
        meiMordent.SetForm(libmei::mordentLog_FORM_upper);
        break;
    default:
        break;
    }

    // @long
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentTremblement):
    case (engraving::SymId::ornamentPrallMordent):
    case (engraving::SymId::ornamentUpPrall):
    case (engraving::SymId::ornamentPrecompMordentUpperPrefix):
    case (engraving::SymId::ornamentUpMordent):
    case (engraving::SymId::ornamentDownMordent):
    case (engraving::SymId::ornamentPrallDown):
    case (engraving::SymId::ornamentPrallUp):
    case (engraving::SymId::ornamentLinePrall):
        meiMordent.SetLong(libmei::BOOLEAN_true);
        break;
    default:
        break;
    }

    // glyph.name
    bool smufl = true;
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentMordent):
    case (engraving::SymId::ornamentShortTrill):
    case (engraving::SymId::ornamentTremblement):
    case (engraving::SymId::ornamentPrallMordent):
        smufl = false;
        break;
    default:
        break;
    }
    if (smufl) {
        // For glyph with composite glyphs in MuseScore use the values in the Convert map
        if (s_mordentGlyphs.count(ornament->symId())) {
            meiMordent.SetGlyphName(s_mordentGlyphs.at(ornament->symId()));
        } else {
            AsciiStringView glyphName = engraving::SymNames::nameForSymId(ornament->symId());
            meiMordent.SetGlyphName(glyphName.ascii());
        }
        meiMordent.SetGlyphAuth(SMUFL_AUTH);
    }

    // @color
    Convert::colorToMEI(ornament, meiMordent);

    return meiMordent;
}

void Convert::octaveFromMEI(engraving::Ottava* ottava, const libmei::Octave& meiOctave, bool& warning)
{
    warning = false;

    engraving::OttavaType ottavaType;
    if (meiOctave.GetDisPlace() == libmei::STAFFREL_basic_below) {
        switch (meiOctave.GetDis()) {
        case (libmei::OCTAVE_DIS_22): ottavaType = engraving::OttavaType::OTTAVA_22MB;
            break;
        case (libmei::OCTAVE_DIS_15): ottavaType = engraving::OttavaType::OTTAVA_15MB;
            break;
        default:
            ottavaType = engraving::OttavaType::OTTAVA_8VB;
        }
    } else {
        switch (meiOctave.GetDis()) {
        case (libmei::OCTAVE_DIS_22): ottavaType = engraving::OttavaType::OTTAVA_22MA;
            break;
        case (libmei::OCTAVE_DIS_15): ottavaType = engraving::OttavaType::OTTAVA_15MA;
            break;
        default:
            ottavaType = engraving::OttavaType::OTTAVA_8VA;
        }
    }
    ottava->setOttavaType(ottavaType);

    // @extender
    if (meiOctave.HasExtender() && (meiOctave.GetExtender() == libmei::BOOLEAN_false)) {
        ottava->setLineVisible(false);
    }

    // @lform
    if (meiOctave.HasLform()) {
        bool lformWarning = false;
        ottava->setLineStyle(Convert::lineFromMEI(meiOctave.GetLform(), lformWarning));
        ottava->setPropertyFlags(engraving::Pid::LINE_STYLE, engraving::PropertyFlags::UNSTYLED);
        warning = (warning || lformWarning);
    }

    // @lendsym
    if (meiOctave.HasLendsym() && (meiOctave.GetLendsym() == libmei::LINESTARTENDSYMBOL_none)) {
        ottava->setEndHookType(engraving::HookType::NONE);
    }

    // @color
    Convert::colorlineFromMEI(ottava, meiOctave);
}

libmei::Octave Convert::octaveToMEI(const engraving::Ottava* ottava)
{
    libmei::Octave meiOctave;

    // @dis
    switch (ottava->ottavaType()) {
    case (engraving::OttavaType::OTTAVA_8VA):
    case (engraving::OttavaType::OTTAVA_8VB):
        meiOctave.SetDis(libmei::OCTAVE_DIS_8);
        break;
    case (engraving::OttavaType::OTTAVA_15MA):
    case (engraving::OttavaType::OTTAVA_15MB):
        meiOctave.SetDis(libmei::OCTAVE_DIS_15);
        break;
    case (engraving::OttavaType::OTTAVA_22MA):
    case (engraving::OttavaType::OTTAVA_22MB):
        meiOctave.SetDis(libmei::OCTAVE_DIS_22);
        break;
    }

    // @dis.place
    switch (ottava->ottavaType()) {
    case (engraving::OttavaType::OTTAVA_8VA):
    case (engraving::OttavaType::OTTAVA_15MA):
    case (engraving::OttavaType::OTTAVA_22MA):
        meiOctave.SetDisPlace(libmei::STAFFREL_basic_above);
        break;
    case (engraving::OttavaType::OTTAVA_8VB):
    case (engraving::OttavaType::OTTAVA_15MB):
    case (engraving::OttavaType::OTTAVA_22MB):
        meiOctave.SetDisPlace(libmei::STAFFREL_basic_below);
        break;
    }

    // @extender
    if (!ottava->lineVisible()) {
        meiOctave.SetExtender(libmei::BOOLEAN_false);
    }

    // @lform
    if (ottava->lineStyle() != engraving::LineType::DASHED) {
        meiOctave.SetLform(Convert::lineToMEI(ottava->lineStyle()));
    }

    // @lendsym
    if (ottava->endHookType() == engraving::HookType::NONE) {
        meiOctave.SetLendsym(libmei::LINESTARTENDSYMBOL_none);
    }

    // @color
    Convert::colorlineToMEI(ottava, meiOctave);

    // @staff
    Convert::staffIdentToMEI(ottava, meiOctave);

    return meiOctave;
}

Convert::OrnamStruct Convert::ornamFromMEI(engraving::Ornament* ornament, const libmei::Element& meiElement, bool& warning)
{
    OrnamStruct ornamSt;

    const libmei::AttOrnamentAccid* ornamAccidAtt = dynamic_cast<const libmei::AttOrnamentAccid*>(&meiElement);
    const libmei::AttPlacementRelStaff* placementRelStaffAtt = dynamic_cast<const libmei::AttPlacementRelStaff*>(&meiElement);
    const libmei::AttTyped* typedAtt = dynamic_cast<const libmei::AttTyped*>(&meiElement);

    IF_ASSERT_FAILED(ornamAccidAtt && placementRelStaffAtt && typedAtt) {
        return ornamSt;
    }

    // @accidlower
    if (ornamAccidAtt->HasAccidlower()) {
        bool accidWarning = false;
        ornamSt.accidTypeBelow = Convert::accidFromMEI(ornamAccidAtt->GetAccidlower(), accidWarning);
        warning = (warning || accidWarning);
    }

    // @accidupper
    if (ornamAccidAtt->HasAccidupper()) {
        bool accidWarning = false;
        ornamSt.accidTypeAbove = Convert::accidFromMEI(ornamAccidAtt->GetAccidupper(), accidWarning);
        warning = (warning || accidWarning);
    }

    // @place
    if (placementRelStaffAtt->HasPlace()) {
        bool placeWarning = false;
        ornament->setAnchor(Convert::anchorFromMEI(placementRelStaffAtt->GetPlace(), placeWarning));
        ornament->setPropertyFlags(engraving::Pid::ARTICULATION_ANCHOR, engraving::PropertyFlags::UNSTYLED);
        warning = (warning || placeWarning);
    }

    // @type
    if (typedAtt->HasType()) {
        Convert::ornamintervaleFromMEI(ornament, typedAtt->GetType());
    }

    // @color
    Convert::colorFromMEI(ornament, meiElement);

    return ornamSt;
}

libmei::Ornam Convert::ornamToMEI(const engraving::Ornament* ornament)
{
    libmei::Ornam meiOrnam;

    Convert::ornamToMEI(ornament, meiOrnam);

    return meiOrnam;
}

void Convert::ornamToMEI(const engraving::Ornament* ornament, libmei::Element& meiElement)
{
    libmei::AttOrnamentAccid* ornamAccidAtt = dynamic_cast<libmei::AttOrnamentAccid*>(&meiElement);
    libmei::AttPlacementRelStaff* placementRelStaffAtt = dynamic_cast<libmei::AttPlacementRelStaff*>(&meiElement);
    libmei::AttTyped* typedAtt = dynamic_cast<libmei::AttTyped*>(&meiElement);

    IF_ASSERT_FAILED(ornamAccidAtt && placementRelStaffAtt && typedAtt) {
        return;
    }

    // @accidlower
    if (ornament->accidentalBelow()) {
        ornamAccidAtt->SetAccidlower(Convert::accidToMEI(ornament->accidentalBelow()->accidentalType()));
    }

    // @accidupper
    if (ornament->accidentalAbove()) {
        ornamAccidAtt->SetAccidupper(Convert::accidToMEI(ornament->accidentalAbove()->accidentalType()));
    }

    // @place
    if (ornament->propertyFlags(engraving::Pid::ARTICULATION_ANCHOR) == engraving::PropertyFlags::UNSTYLED) {
        placementRelStaffAtt->SetPlace(Convert::anchorToMEI(ornament->anchor()));
    }

    // @type
    typedAtt->SetType(Convert::ornamintervalToMEI(ornament).toStdString());

    // @color
    Convert::colorToMEI(ornament, meiElement);
}

void Convert::ornamintervaleFromMEI(engraving::Ornament* ornament, const std::string& meiType)
{
    // @type
    std::string value;

    if (Convert::getTypeValueWithPrefix(meiType, INTERVAL_ABOVE, value)) {
        engraving::OrnamentInterval interval = engraving::TConv::fromXml(String(value.c_str()).replace(u':',
                                                                                                       u','),
                                                                         engraving::DEFAULT_ORNAMENT_INTERVAL);
        ornament->setIntervalAbove(interval);
    }

    if (Convert::getTypeValueWithPrefix(meiType, INTERVAL_BELOW, value)) {
        engraving::OrnamentInterval interval = engraving::TConv::fromXml(String(value.c_str()).replace(u':',
                                                                                                       u','),
                                                                         engraving::DEFAULT_ORNAMENT_INTERVAL);
        ornament->setIntervalBelow(interval);
    }
}

String Convert::ornamintervalToMEI(const engraving::Ornament* ornament)
{
    // @type
    StringList typeList;

    if (ornament->hasIntervalAbove() && (ornament->intervalAbove().type != engraving::IntervalType::AUTO)) {
        // Since a , is not valid in a type attribute, replace with :
        String intervalStr = engraving::TConv::toXml(ornament->intervalAbove()).replace(u',', u':');
        typeList << String("%1%2").arg(String(INTERVAL_ABOVE)).arg(intervalStr);
    }

    if (ornament->hasIntervalBelow() && (ornament->intervalBelow().type != engraving::IntervalType::AUTO)) {
        String intervalStr = engraving::TConv::toXml(ornament->intervalBelow()).replace(u',', u':');
        typeList << String("%1%2").arg(String(INTERVAL_BELOW)).arg(intervalStr);
    }

    return typeList.join(u" ");
}

void Convert::pedalFromMEI(engraving::Pedal* pedal, const libmei::Pedal& meiPedal, bool& warning)
{
    warning = false;

    // @dir - ignore, dealing with "down" only

    // @form
    if (meiPedal.GetForm() == libmei::PEDALSTYLE_pedstar) {
        pedal->setBeginText(engraving::Pedal::PEDAL_SYMBOL);
        pedal->setPropertyFlags(engraving::Pid::BEGIN_TEXT, engraving::PropertyFlags::UNSTYLED);
        pedal->setEndText(engraving::Pedal::STAR_SYMBOL);
        pedal->setPropertyFlags(engraving::Pid::END_TEXT, engraving::PropertyFlags::UNSTYLED);
        pedal->setLineVisible(false);
    } else if (meiPedal.GetForm() == libmei::PEDALSTYLE_pedline) {
        pedal->setBeginText(engraving::Pedal::PEDAL_SYMBOL);
        pedal->setPropertyFlags(engraving::Pid::BEGIN_TEXT, engraving::PropertyFlags::UNSTYLED);
        pedal->setEndHookType(engraving::HookType::HOOK_90);
    } else {
        pedal->setBeginHookType(engraving::HookType::HOOK_90);
        pedal->setBeginText(String());
        pedal->setContinueText(String());
        pedal->setEndHookType(engraving::HookType::HOOK_90);
    }

    // @func
    if (meiPedal.GetFunc() == "soft") {
        // This is just for importing MEI files and will not be exported
        if (pedal->beginText() != "") {
            pedal->setBeginText(u"una corda");
            pedal->setContinueText(u"");
        }
    } else if (meiPedal.GetFunc() == "sostenuto") {
        if (pedal->beginText() != "") {
            pedal->setBeginText(u"<sym>keyboardPedalSost</sym>");
            pedal->setContinueText(u"(<sym>keyboardPedalSost</sym>)");
        }
    }

    // @color
    Convert::colorlineFromMEI(pedal, meiPedal);
}

libmei::Pedal Convert::pedalToMEI(const engraving::Pedal* pedal)
{
    libmei::Pedal meiPedal;

    // @dir
    meiPedal.SetDir(libmei::pedalLog_DIR_down);

    bool symbol = (!pedal->beginText().isEmpty());
    bool star = (pedal->endText() == engraving::Pedal::STAR_SYMBOL);

    // @form
    if (symbol && star) {
        meiPedal.SetForm(libmei::PEDALSTYLE_pedstar);
    } else if (symbol && !star) {
        meiPedal.SetForm(libmei::PEDALSTYLE_pedline);
    } else {
        meiPedal.SetForm(libmei::PEDALSTYLE_line);
    }

    // @func
    if (pedal->beginText() == u"<sym>keyboardPedalSost</sym>" || pedal->beginText() == u"<sym>keyboardPedalS</sym>") {
        meiPedal.SetFunc("sostenuto");
    }

    // @color
    Convert::colorlineToMEI(pedal, meiPedal);

    return meiPedal;
}

Convert::PitchStruct Convert::pitchFromMEI(const libmei::Note& meiNote, const libmei::Accid& meiAccid,
                                           const engraving::Interval& interval,
                                           bool& warning)
{
    // The mapping from pitch name to step
    static int pitchMap[7] = { 0, 2, 4, 5, 7, 9, 11 };
    //                         c  d  e  f  g  a   b

    warning = false;
    PitchStruct pitchSt;

    int step = meiNote.HasPname() ? meiNote.GetPname() - 1 : 0;
    // It should never be necessary, but just in case
    step = std::clamp(step, 0, 6);

    int oct = meiNote.HasOct() ? meiNote.GetOct() : 3;

    engraving::AccidentalVal alter = engraving::AccidentalVal::NATURAL;
    if (meiAccid.HasAccid() || meiAccid.HasAccidGes()) {
        bool accidWarning = false;
        libmei::data_ACCIDENTAL_GESTURAL meiAccidGes
            = (meiAccid.HasAccidGes()) ? meiAccid.GetAccidGes() : libmei::Att::AccidentalWrittenToGestural(meiAccid.GetAccid());
        alter = Convert::accidGesFromMEI(meiAccidGes, accidWarning);
        warning = (warning || accidWarning);
    }
    int alterInt = static_cast<int>(alter);

    if (meiAccid.HasEnclose()) {
        switch (meiAccid.GetEnclose()) {
        case libmei::ENCLOSURE_brack:
            pitchSt.accidBracket = engraving::AccidentalBracket::BRACKET;
            break;
        case libmei::ENCLOSURE_paren:
            pitchSt.accidBracket = engraving::AccidentalBracket::PARENTHESIS;
            break;
        default:
            pitchSt.accidBracket = engraving::AccidentalBracket::NONE;
            break;
        }
    }

    bool accidWarning = false;
    if (meiAccid.HasAccid()) {
        pitchSt.accidRole = engraving::AccidentalRole::USER;
    }
    pitchSt.accidType = Convert::accidFromMEI(meiAccid.GetAccid(), accidWarning);
    warning = (warning || accidWarning);
    pitchSt.pitch = pitchMap[step] + alterInt + (oct + 1) * 12;
    pitchSt.tpc2 = engraving::step2tpc(step, alter);
    // The pitch retrieved from the MEI is the written pitch and we need to transpose it
    pitchSt.pitch += interval.chromatic;

    return pitchSt;
}

std::pair<libmei::Note, libmei::Accid> Convert::pitchToMEI(const engraving::Note* note, const engraving::Accidental* accid,
                                                           const engraving::Interval& interval)
{
    libmei::Note meiNote;
    libmei::Accid meiAccid;

    IF_ASSERT_FAILED(note) {
        return { meiNote, meiAccid };
    }

    Convert::PitchStruct pitch;
    pitch.pitch = note->pitch();
    pitch.tpc2 = note->tpc2();
    if (accid) {
        pitch.accidType = accid->accidentalType();
        pitch.accidBracket = accid->bracket();
        // Not needed because relying on accidType
        // pitch.accidRole = accid->role();
    }

    // @pname
    meiNote.SetPname(static_cast<libmei::data_PITCHNAME>(engraving::tpc2step(pitch.tpc2) + 1));

    int writtenAlterInt = static_cast<int>(engraving::Accidental::subtype2value(pitch.accidType));
    int alterInt = tpc2alterByKey(pitch.tpc2, engraving::Key::C);

    // @oct
    // We need to adjust the pitch to its transposed value for the octave calculation
    int oct = ((pitch.pitch - interval.chromatic - alterInt) / 12) - 1;
    meiNote.SetOct(oct);

    // @oct.ges
    int octGes = ((note->ppitch() - interval.chromatic - alterInt) / 12) - 1;
    if (octGes != oct) {
        meiNote.SetOctGes(octGes);
    }

    // @accid
    if (pitch.accidType != engraving::AccidentalType::NONE) {
        meiAccid.SetAccid(Convert::accidToMEI(pitch.accidType));
        if (pitch.accidBracket == engraving::AccidentalBracket::BRACKET) {
            meiAccid.SetEnclose(libmei::ENCLOSURE_brack);
        } else if (pitch.accidBracket == engraving::AccidentalBracket::PARENTHESIS) {
            meiAccid.SetEnclose(libmei::ENCLOSURE_paren);
        }
    }

    // @accid.ges
    if (alterInt && (alterInt != writtenAlterInt)) {
        meiAccid.SetAccidGes(Convert::accidGesToMEI(static_cast<engraving::AccidentalVal>(alterInt)));
    }

    return { meiNote, meiAccid };
}

engraving::PlacementV Convert::placeFromMEI(const libmei::data_STAFFREL meiPlace, bool& warning)
{
    warning = false;
    switch (meiPlace) {
    case (libmei::STAFFREL_above): return engraving::PlacementV::ABOVE;
    case (libmei::STAFFREL_below): return engraving::PlacementV::BELOW;
    default:
        return engraving::PlacementV::ABOVE;
    }
}

libmei::data_STAFFREL Convert::placeToMEI(engraving::PlacementV place)
{
    switch (place) {
    case (engraving::PlacementV::ABOVE): return libmei::STAFFREL_above;
    case (engraving::PlacementV::BELOW): return libmei::STAFFREL_below;
    default:
        return libmei::STAFFREL_above;
    }
}

engraving::DirectionV Convert::directionFromMEI(const libmei::data_STAFFREL meiPlace)
{
    switch (meiPlace) {
    case (libmei::STAFFREL_above): return engraving::DirectionV::UP;
    case (libmei::STAFFREL_below): return engraving::DirectionV::DOWN;
    default:
        return engraving::DirectionV::AUTO;
    }
}

libmei::data_STAFFREL Convert::directionToMEI(engraving::DirectionV direction)
{
    switch (direction) {
    case (engraving::DirectionV::UP): return libmei::STAFFREL_above;
    case (engraving::DirectionV::DOWN): return libmei::STAFFREL_below;
    default:
        return libmei::STAFFREL_NONE;
    }
}

void Convert::slurFromMEI(engraving::SlurTie* slur, const libmei::Slur& meiSlur, bool& warning)
{
    warning = false;

    // @curvedir
    if (meiSlur.HasCurvedir()) {
        slur->setSlurDirection(Convert::curvedirFromMEI(meiSlur.GetCurvedir(), warning));
    }

    // @lform
    if (meiSlur.HasLform()) {
        bool typeWarning = false;
        slur->setStyleType(Convert::slurstyleFromMEI(meiSlur.GetLform(), typeWarning));
        warning = (warning || typeWarning);
    }

    // @color
    Convert::colorFromMEI(slur, meiSlur);
}

libmei::Slur Convert::slurToMEI(const engraving::SlurTie* slur)
{
    libmei::Slur meiSlur;

    // @place
    if (slur->slurDirection() != engraving::DirectionV::AUTO) {
        meiSlur.SetCurvedir(Convert::curvedirToMEI(slur->slurDirection()));
    }

    // @lform
    if (slur->styleType() != engraving::SlurStyleType::Solid) {
        meiSlur.SetLform(Convert::slurstyleToMEI(slur->styleType()));
    }

    // @color
    Convert::colorToMEI(slur, meiSlur);

    return meiSlur;
}

engraving::SlurStyleType Convert::slurstyleFromMEI(const libmei::data_LINEFORM meiLine, bool& warning)
{
    warning = false;
    switch (meiLine) {
    case (libmei::LINEFORM_solid): return engraving::SlurStyleType::Solid;
    case (libmei::LINEFORM_dashed): return engraving::SlurStyleType::Dashed;
    case (libmei::LINEFORM_dotted): return engraving::SlurStyleType::Dotted;
    case (libmei::LINEFORM_wavy):
        warning = true;
        return engraving::SlurStyleType::Solid;
    default:
        return engraving::SlurStyleType::Solid;
    }
}

libmei::data_LINEFORM Convert::slurstyleToMEI(engraving::SlurStyleType slurstyle)
{
    switch (slurstyle) {
    case (engraving::SlurStyleType::Solid): return libmei::LINEFORM_solid;
    case (engraving::SlurStyleType::Dashed): return libmei::LINEFORM_dashed;
    case (engraving::SlurStyleType::WideDashed): return libmei::LINEFORM_dashed;
    case (engraving::SlurStyleType::Dotted): return libmei::LINEFORM_dotted;
    default:
        return libmei::LINEFORM_NONE;
    }
}

Convert::StaffStruct Convert::staffFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    warning = false;
    StaffStruct staffSt;

    if (meiStaffDef.HasLines()) {
        staffSt.lines = meiStaffDef.GetLines();
    }
    staffSt.invisible = meiStaffDef.GetLinesVisible() == libmei::BOOLEAN_false;
    staffSt.scale = meiStaffDef.HasScale() ? meiStaffDef.GetScale() : 100;
    staffSt.color = engraving::Color::fromString(meiStaffDef.GetLinesColor());

    // Set it only if both are given
    if (meiStaffDef.HasTransDiat() && meiStaffDef.HasTransSemi()) {
        staffSt.interval.diatonic = meiStaffDef.GetTransDiat();
        staffSt.interval.chromatic = meiStaffDef.GetTransSemi();
    }

    return staffSt;
}

libmei::StaffDef Convert::staffToMEI(const engraving::Staff* staff)
{
    libmei::StaffDef meiStaffDef;

    IF_ASSERT_FAILED(staff) {
        return meiStaffDef;
    }

    // @n
    meiStaffDef.SetN(static_cast<int>(staff->idx() + 1));
    // @trans.*
    const engraving::Interval& interval = staff->part()->instrument()->transpose();
    if (!interval.isZero()) {
        meiStaffDef.SetTransDiat(interval.diatonic);
        meiStaffDef.SetTransSemi(interval.chromatic);
    }
    // @lines
    const engraving::StaffType* staffType = staff->staffType(engraving::Fraction(0, 1));
    if (staffType) {
        meiStaffDef.SetLines(staffType->lines());
    }
    // @lines.color
    if (staffType->color() != engravingConfiguration()->defaultColor()) {
        meiStaffDef.SetLinesColor(staffType->color().toString());
    }
    // @lines.visible
    if (staff->isLinesInvisible(engraving::Fraction(0, 1))) {
        meiStaffDef.SetLinesVisible(libmei::BOOLEAN_false);
    }
    // @scale
    const double scale = staff->staffMag(engraving::Fraction(0, 1));
    if (!muse::RealIsEqual(scale, 1.0)) {
        meiStaffDef.SetScale(scale * 100);
    }
    return meiStaffDef;
}

std::pair<engraving::DirectionV, bool> Convert::stemFromMEI(const libmei::AttStems& meiStemsAtt, bool& warning)
{
    warning = false;

    engraving::DirectionV direction = engraving::DirectionV::AUTO;
    bool noStem = false;

    switch (meiStemsAtt.GetStemDir()) {
    case (libmei::STEMDIRECTION_up): direction = engraving::DirectionV::UP;
        break;
    case (libmei::STEMDIRECTION_down): direction = engraving::DirectionV::DOWN;
        break;
    default:
        break;
    }
    if (RealIsNull(meiStemsAtt.GetStemLen())) {
        noStem = true;
    }

    return { direction, noStem };
}

std::pair<libmei::data_STEMDIRECTION, double> Convert::stemToMEI(const engraving::DirectionV direction, bool noStem)
{
    libmei::data_STEMDIRECTION meiStemDir = libmei::STEMDIRECTION_NONE;
    double meiStemLen = -1.0;

    switch (direction) {
    case (engraving::DirectionV::UP): meiStemDir = libmei::STEMDIRECTION_up;
        break;
    case (engraving::DirectionV::DOWN): meiStemDir = libmei::STEMDIRECTION_down;
        break;
    default:
        break;
    }
    if (noStem) {
        meiStemDir = libmei::STEMDIRECTION_NONE;
        meiStemLen = 0.0;
    }

    return { meiStemDir, meiStemLen };
}

engraving::TremoloType Convert::stemModFromMEI(const libmei::data_STEMMODIFIER meiStemMod)
{
    switch (meiStemMod) {
    case (libmei::STEMMODIFIER_1slash): return engraving::TremoloType::R8;
    case (libmei::STEMMODIFIER_2slash): return engraving::TremoloType::R16;
    case (libmei::STEMMODIFIER_3slash): return engraving::TremoloType::R32;
    case (libmei::STEMMODIFIER_4slash): return engraving::TremoloType::R64;
    case (libmei::STEMMODIFIER_z): return engraving::TremoloType::BUZZ_ROLL;
    default:
        return engraving::TremoloType::INVALID_TREMOLO;
    }
}

libmei::data_STEMMODIFIER Convert::stemModToMEI(const engraving::TremoloSingleChord* tremolo)
{
    switch (tremolo->tremoloType()) {
    case (engraving::TremoloType::R8):  return libmei::STEMMODIFIER_1slash;
    case (engraving::TremoloType::R16): return libmei::STEMMODIFIER_2slash;
    case (engraving::TremoloType::R32): return libmei::STEMMODIFIER_3slash;
    case (engraving::TremoloType::R64): return libmei::STEMMODIFIER_4slash;
    case (engraving::TremoloType::BUZZ_ROLL): return libmei::STEMMODIFIER_z;
    default:
        return libmei::STEMMODIFIER_NONE;
    }
}

void Convert::sylFromMEI(engraving::Lyrics* lyrics, const libmei::Syl& meiSyl, ElisionType elision, bool& warning)
{
    warning = false;

    if (elision == ElisionLast) {
        if (meiSyl.GetWordpos() == libmei::sylLog_WORDPOS_i) {
            // We had previous (the first) syl with a @wordpos terminal
            if (lyrics->syllabic() == engraving::LyricsSyllabic::END) {
                lyrics->setSyllabic(engraving::LyricsSyllabic::MIDDLE);
            } else {
                lyrics->setSyllabic(engraving::LyricsSyllabic::BEGIN);
            }
        }
    } else if ((elision == ElisionFirst) || (elision == ElisionNone)) {
        switch (meiSyl.GetWordpos()) {
        case (libmei::sylLog_WORDPOS_i): lyrics->setSyllabic(engraving::LyricsSyllabic::BEGIN);
            break;
        case (libmei::sylLog_WORDPOS_m): lyrics->setSyllabic(engraving::LyricsSyllabic::MIDDLE);
            break;
        case (libmei::sylLog_WORDPOS_t): lyrics->setSyllabic(engraving::LyricsSyllabic::END);
            break;
        default: break;
        }
    }
}

libmei::Syl Convert::sylToMEI(const engraving::Lyrics* lyrics, ElisionType elision)
{
    libmei::Syl meiSyl;

    switch (lyrics->syllabic()) {
    case (engraving::LyricsSyllabic::BEGIN):
        meiSyl.SetWordpos(libmei::sylLog_WORDPOS_i);
        meiSyl.SetCon(libmei::sylLog_CON_d);
        break;
    case (engraving::LyricsSyllabic::MIDDLE):
        meiSyl.SetWordpos(libmei::sylLog_WORDPOS_m);
        meiSyl.SetCon(libmei::sylLog_CON_d);
        break;
    case (engraving::LyricsSyllabic::END):
        meiSyl.SetWordpos(libmei::sylLog_WORDPOS_t);
        break;
    default:
        break;
    }

    // Adjust @wordpos and @con with elisions
    if (elision == ElisionFirst) {
        // Set the elision connector
        meiSyl.SetCon(libmei::sylLog_CON_b);
        // Make middle a terminal and remove initial word position
        if (meiSyl.GetWordpos() == libmei::sylLog_WORDPOS_m) {
            meiSyl.SetWordpos(libmei::sylLog_WORDPOS_t);
        } else if (meiSyl.GetWordpos() == libmei::sylLog_WORDPOS_i) {
            meiSyl.SetWordpos(libmei::sylLog_WORDPOS_NONE);
        }
    } else if (elision == ElisionMiddle) {
        // Set the elision connector and remove any word postion
        meiSyl.SetCon(libmei::sylLog_CON_b);
        meiSyl.SetWordpos(libmei::sylLog_WORDPOS_NONE);
    } else if (elision == ElisionLast) {
        // Make middle an initial and remove terminal word position
        if (meiSyl.GetWordpos() == libmei::sylLog_WORDPOS_m) {
            meiSyl.SetWordpos(libmei::sylLog_WORDPOS_i);
        } else if (meiSyl.GetWordpos() == libmei::sylLog_WORDPOS_t) {
            meiSyl.SetWordpos(libmei::sylLog_WORDPOS_NONE);
        }
    }

    // Add extender connector
    if ((elision == ElisionNone) || (elision == ElisionLast)) {
        if ((meiSyl.GetCon() == libmei::sylLog_CON_NONE) && (lyrics->ticks() != engraving::Fraction(0, 1))) {
            meiSyl.SetCon(libmei::sylLog_CON_u);
        }
    }

    return meiSyl;
}

void Convert::tempoFromMEI(engraving::TempoText* tempoText, const StringList& meiLines, const libmei::Tempo& meiTempo, bool& warning)
{
    IF_ASSERT_FAILED(tempoText) {
        return;
    }

    warning = false;

    // @place
    if (meiTempo.HasPlace()) {
        tempoText->setPlacement(meiTempo.GetPlace()
                                == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        tempoText->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @bmp.midi
    if (meiTempo.HasMidiBpm()) {
        double tempoValue = meiTempo.GetMidiBpm() / 60;
        tempoText->setTempo(tempoValue);
    }

    // @type
    if (meiTempo.HasType() && Convert::hasTypeValue(meiTempo.GetType(), TEMPO_INFER_FROM_TEXT)) {
        tempoText->setFollowText(true);
    }

    // text
    tempoText->setXmlText(meiLines.join(u"\n"));
}

libmei::Tempo Convert::tempoToMEI(const engraving::TempoText* tempoText, StringList& meiLines)
{
    libmei::Tempo meiTempo;

    // @midi.bpm
    engraving::BeatsPerMinute bpm = tempoText->tempo().toBPM();
    double bpmRounded = round(bpm.val * 100) / 100;
    meiTempo.SetMidiBpm(bpmRounded);

    // @place
    if (tempoText->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiTempo.SetPlace(Convert::placeToMEI(tempoText->placement()));
    }

    // @type
    if (tempoText->followText()) {
        meiTempo.SetType(TEMPO_INFER_FROM_TEXT);
    }

    // text content - only split lines
    meiLines = String(tempoText->plainText()).split(u"\n");

    // @staff
    Convert::staffIdentToMEI(tempoText, meiTempo);

    return meiTempo;
}

engraving::TextStyleType Convert::textFromMEI(const libmei::Rend& meiRend, bool& warning)
{
    warning = false;

    std::string rendType = meiRend.GetType();
    engraving::TextStyleType textStyle = engraving::TextStyleType::DEFAULT;
    if (rendType.size() > 0) {
        textStyle = engraving::TConv::fromXml(rendType, engraving::TextStyleType::DEFAULT);
    }

    if (textStyle == engraving::TextStyleType::DEFAULT) {
        warning = true;
        textStyle = engraving::TextStyleType::TITLE;
    }

    return textStyle;
}

std::tuple<libmei::Rend, TextCell, String> Convert::textToMEI(const engraving::Text* text)
{
    libmei::Rend meiRend;
    TextCell cell = MiddleCenter;
    String string = text->plainText();

    libmei::data_FONTSIZE fontsize;

    /**
     Use default style placements.
     We could use at the style actual values eventually, even though that would be more complicated to get back in
     */
    switch (text->textStyleType()) {
    case (engraving::TextStyleType::TITLE):
        cell = TopCenter;
        fontsize.SetTerm(libmei::FONTSIZETERM_x_large);
        break;
    case (engraving::TextStyleType::SUBTITLE):
        cell = TopCenter;
        fontsize.SetTerm(libmei::FONTSIZETERM_large);
        break;
    case (engraving::TextStyleType::COMPOSER):
        cell = BottomRight;
        break;
    case (engraving::TextStyleType::LYRICIST):
        cell = BottomLeft;
        break;
    case (engraving::TextStyleType::INSTRUMENT_EXCERPT):
        cell = TopLeft;
        break;
    default: cell = MiddleCenter;
    }

    meiRend.SetFontsize(fontsize);

    // @label (@lang because not available in MEI Basic
    // This is what allows for reading pgHead back as VBox
    AsciiStringView rendType = engraving::TConv::toXml(text->textStyleType());
    meiRend.SetType(rendType.ascii());

    return { meiRend, cell, string };
}

/**
 * Convert the segmented text block into MuseScore xmlText (with <sym>)
 * The text block are {bool, string} pairs with true for smufl and false for plain text
 */

void Convert::textFromMEI(String& text, const textWithSmufl& textBlocks)
{
    text.clear();

    for (auto& block : textBlocks) {
        if (!block.first) {
            text += block.second;
        } else {
            for (size_t i = 0; i < block.second.size(); i++) {
                char16_t c = block.second.at(i).unicode();
                if (c < u'\uE000' || c > u'\uF8FF') {
                    continue; // this should not happen because the char should be smufl
                }
                engraving::SymId symId = engravingFonts()->fallbackFont()->fromCode(c);
                if (symId == engraving::SymId::noSym) {
                    continue; // smufl code not found
                }
                text += u"<sym>" + String::fromAscii(engraving::SymNames::nameForSymId(symId).ascii()) + u"</sym>";
            }
        }
    }
}

/**
 * Convert a MuseScore plainText (without <sym>) into text blocks
 * The text block are {bool, string} pairs with true for smufl and false for plain text
 */

void Convert::textToMEI(textWithSmufl& textBlocks, const String& text)
{
    // Status of what is being parsed
    bool isSmufl = false;

    String smuflBlock;
    String textBlock;

    // Go through the text char by char and build blocks of plain text / smufl text
    for (size_t i = 0; i < text.size(); i++) {
        char16_t c = text.at(i).unicode();
        // Not SMuFL
        if (c < u'\uE000' || c > u'\uF8FF') {
            // Changing to plain text, add the current smufl block if something in it
            if (isSmufl && smuflBlock.size() > 0) {
                textBlocks.push_back(std::make_pair(true, smuflBlock));
                smuflBlock.clear();
            }
            textBlock += c;
            isSmufl = false;
        }
        // SMuFL
        else {
            // Changing to smufl, add the current plain text block if something in in
            if (!isSmufl && textBlock.size() > 0) {
                textBlocks.push_back(std::make_pair(false, textBlock));
                textBlock.clear();
            }
            smuflBlock += c;
            isSmufl = true;
        }
    }
    // Add the last block
    if (textBlock.size() > 0) {
        textBlocks.push_back(std::make_pair(false, textBlock));
    }
    if (smuflBlock.size() > 0) {
        textBlocks.push_back(std::make_pair(true, smuflBlock));
    }
}

void Convert::tieFromMEI(engraving::SlurTie* tie, const libmei::Tie& meiTie, bool& warning)
{
    libmei::Slur meiSlur;
    meiSlur.SetCurvedir(meiTie.GetCurvedir());
    meiSlur.SetLform(meiTie.GetLform());
    meiSlur.SetColor(meiTie.GetColor());
    Convert::slurFromMEI(tie, meiSlur, warning);
}

libmei::Tie Convert::tieToMEI(const engraving::SlurTie* tie)
{
    libmei::Slur meiSlur = Convert::slurToMEI(tie);
    libmei::Tie meiTie;
    meiTie.SetCurvedir(meiSlur.GetCurvedir());
    meiTie.SetLform(meiSlur.GetLform());
    meiTie.SetColor(meiSlur.GetColor());
    return meiTie;
}

Convert::OrnamStruct Convert::trillFromMEI(engraving::Ornament* ornament, const libmei::Trill& meiTrill, bool& warning)
{
    // @glyph.name
    bool smufl = (meiTrill.HasGlyphAuth() && (meiTrill.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::ornamentTrill;

    // We give @glyph.name (or @glyph.num) priority over @form and @long
    if (smufl && meiTrill.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiTrill.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiTrill.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiTrill.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }

    ornament->setSymId(symId);

    // @color
    Convert::colorFromMEI(ornament, meiTrill);

    // Other attributes
    return Convert::ornamFromMEI(ornament, meiTrill, warning);
}

libmei::Trill Convert::trillToMEI(const engraving::Ornament* ornament)
{
    libmei::Trill meiTrill;

    Convert::ornamToMEI(ornament, meiTrill);

    // @glyph.name
    if (ornament->symId() != engraving::SymId::ornamentTrill) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(ornament->symId());
        meiTrill.SetGlyphName(glyphName.ascii());
        meiTrill.SetGlyphAuth(SMUFL_AUTH);
    }

    // @color
    Convert::colorToMEI(ornament, meiTrill);

    return meiTrill;
}

void Convert::tupletFromMEI(engraving::Tuplet* tuplet, const libmei::Tuplet& meiTuplet, bool& warning)
{
    IF_ASSERT_FAILED(tuplet) {
        return;
    }

    warning = false;
    if (!meiTuplet.HasNum() || !meiTuplet.HasNumbase()) {
        warning = true;
    } else {
        tuplet->setRatio(engraving::Fraction(meiTuplet.GetNum(), meiTuplet.GetNumbase()));
    }

    if (meiTuplet.GetNumFormat() == libmei::tupletVis_NUMFORMAT_ratio) {
        tuplet->setNumberType(engraving::TupletNumberType::SHOW_RELATION);
        tuplet->setPropertyFlags(engraving::Pid::NUMBER_TYPE, engraving::PropertyFlags::UNSTYLED);
    } else if (meiTuplet.GetNumVisible() == libmei::BOOLEAN_false) {
        tuplet->setNumberType(engraving::TupletNumberType::NO_TEXT);
        tuplet->setPropertyFlags(engraving::Pid::NUMBER_TYPE, engraving::PropertyFlags::UNSTYLED);
    }

    if (meiTuplet.GetBracketVisible() == libmei::BOOLEAN_true) {
        tuplet->setBracketType(engraving::TupletBracketType::SHOW_BRACKET);
        tuplet->setPropertyFlags(engraving::Pid::BRACKET_TYPE, engraving::PropertyFlags::UNSTYLED);
    } else if (meiTuplet.GetBracketVisible() == libmei::BOOLEAN_false) {
        tuplet->setBracketType(engraving::TupletBracketType::SHOW_NO_BRACKET);
        tuplet->setPropertyFlags(engraving::Pid::BRACKET_TYPE, engraving::PropertyFlags::UNSTYLED);
    }

    libmei::data_STAFFREL_basic bracketPlace = meiTuplet.GetBracketPlace();
    libmei::data_STAFFREL_basic numPlace = meiTuplet.GetNumPlace();
    if ((bracketPlace == libmei::STAFFREL_basic_above) || (numPlace == libmei::STAFFREL_basic_above)) {
        tuplet->setDirection(engraving::DirectionV::UP);
        tuplet->setPropertyFlags(engraving::Pid::DIRECTION, engraving::PropertyFlags::UNSTYLED);
    } else if ((bracketPlace == libmei::STAFFREL_basic_below) || (numPlace == libmei::STAFFREL_basic_below)) {
        tuplet->setDirection(engraving::DirectionV::DOWN);
        tuplet->setPropertyFlags(engraving::Pid::DIRECTION, engraving::PropertyFlags::UNSTYLED);
    }

    // @color
    Convert::colorFromMEI(tuplet, meiTuplet);
}

libmei::Tuplet Convert::tupletToMEI(const engraving::Tuplet* tuplet)
{
    libmei::Tuplet meiTuplet;

    meiTuplet.SetNum(tuplet->ratio().numerator());
    meiTuplet.SetNumbase(tuplet->ratio().denominator());

    if (tuplet->numberType() == engraving::TupletNumberType::SHOW_RELATION) {
        meiTuplet.SetNumFormat(libmei::tupletVis_NUMFORMAT_ratio);
    } else if (tuplet->numberType() == engraving::TupletNumberType::NO_TEXT) {
        meiTuplet.SetNumVisible(libmei::BOOLEAN_false);
    }

    if (tuplet->bracketType() == engraving::TupletBracketType::SHOW_NO_BRACKET) {
        meiTuplet.SetBracketVisible(libmei::BOOLEAN_false);
    } else if (tuplet->bracketType() == engraving::TupletBracketType::SHOW_BRACKET) {
        meiTuplet.SetBracketVisible(libmei::BOOLEAN_true);
    }

    if (tuplet->direction() == engraving::DirectionV::UP) {
        meiTuplet.SetBracketPlace(libmei::STAFFREL_basic_above);
        meiTuplet.SetNumPlace(libmei::STAFFREL_basic_above);
    } else if (tuplet->direction() == engraving::DirectionV::DOWN) {
        meiTuplet.SetBracketPlace(libmei::STAFFREL_basic_below);
        meiTuplet.SetNumPlace(libmei::STAFFREL_basic_below);
    }

    // @color
    Convert::colorToMEI(tuplet, meiTuplet);

    return meiTuplet;
}

Convert::OrnamStruct Convert::turnFromMEI(engraving::Ornament* ornament, const libmei::Turn& meiTurn, bool& warning)
{
    // @glyph.name
    bool smufl = (meiTurn.HasGlyphAuth() && (meiTurn.GetGlyphAuth() == SMUFL_AUTH));

    engraving::SymId symId = engraving::SymId::ornamentTurn;

    // We give @glyph.name (or @glyph.num) priority over @form and @long
    if (smufl && meiTurn.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiTurn.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiTurn.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiTurn.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    } else {
        if (meiTurn.HasForm()) {
            if (meiTurn.GetForm() == libmei::turnLog_FORM_lower) {
                symId = engraving::SymId::ornamentTurnInverted;
            }
        }
    }

    ornament->setSymId(symId);

    // Other attributes
    return Convert::ornamFromMEI(ornament, meiTurn, warning);
}

libmei::Turn Convert::turnToMEI(const engraving::Ornament* ornament)
{
    libmei::Turn meiTurn;

    Convert::ornamToMEI(ornament, meiTurn);

    // @form
    switch (ornament->symId()) {
    case (engraving::SymId::ornamentTurn):
    case (engraving::SymId::ornamentTurnSlash): meiTurn.SetForm(libmei::turnLog_FORM_upper);
        break;
    case (engraving::SymId::ornamentTurnInverted): meiTurn.SetForm(libmei::turnLog_FORM_lower);
        break;
    default: break;
    }

    // @glyph.name
    if (ornament->symId() == engraving::SymId::ornamentTurnSlash) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(ornament->symId());
        meiTurn.SetGlyphName(glyphName.ascii());
        meiTurn.SetGlyphAuth(SMUFL_AUTH);
    }

    return meiTurn;
}

bool Convert::hasTypeValue(const std::string& typeStr, const std::string& value)
{
    std::istringstream iss(typeStr);

    std::string token;
    while (std::getline(iss, token, ' ')) {
        if (token == value) {
            return true;  // value found
        }
    }

    return false;
}

bool Convert::getTypeValueWithPrefix(const std::string& typeStr, const std::string& prefix, std::string& value)
{
    std::istringstream iss(typeStr);
    std::list<std::string> values;

    std::string token;
    while (std::getline(iss, token, ' ')) {
        if ((token.rfind(prefix, 0) == 0) && (prefix.size() < token.size())) {
            value = token.erase(0, prefix.length());
            return true;
        }
    }

    return false;
}

/**
 * Extract the list of values in a @type attribute having a prefix string
 * For example, "mscore-ending-1 primary mscore-ending-3" return ["1", "3"] with prefix "mscore-ending-"
 */

std::list<std::string> Convert::getTypeValuesWithPrefix(const std::string& typeStr, const std::string& prefix)
{
    std::istringstream iss(typeStr);
    std::list<std::string> values;

    std::string token;
    while (std::getline(iss, token, ' ')) {
        if ((token.rfind(prefix, 0) == 0) && (prefix.size() < token.size())) {
            values.push_back(token.erase(0, prefix.length()));
        }
    }

    return values;
}

void Convert::layerIdentFromMEI(engraving::EngravingItem* item, const libmei::Element& meiElement)
{
    if (!item->hasVoiceAssignmentProperties()) {
        return;
    }

    const libmei::AttLayerIdent* layerAtt = dynamic_cast<const libmei::AttLayerIdent*>(&meiElement);

    IF_ASSERT_FAILED(layerAtt) {
        return;
    }

    if (layerAtt->HasLayer()) {
        // without further check we assume the layer to match
        item->setProperty(engraving::Pid::VOICE_ASSIGNMENT, engraving::VoiceAssignment::CURRENT_VOICE_ONLY);
    }
}

void Convert::layerIdentToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement)
{
    libmei::AttLayerIdent* layerAtt = dynamic_cast<libmei::AttLayerIdent*>(&meiElement);

    IF_ASSERT_FAILED(layerAtt) {
        return;
    }

    if (item->hasVoiceAssignmentProperties()
        && (item->getProperty(engraving::Pid::VOICE_ASSIGNMENT).value<engraving::VoiceAssignment>()
            == engraving::VoiceAssignment::CURRENT_VOICE_ONLY)) {
        layerAtt->SetLayer(static_cast<int>(item->voice()) + 1);
    }
}

void Convert::staffIdentToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement)
{
    libmei::AttStaffIdent* staffAtt = dynamic_cast<libmei::AttStaffIdent*>(&meiElement);

    IF_ASSERT_FAILED(staffAtt) {
        return;
    }

    libmei::xsdPositiveInteger_List staffList;
    staffList.push_back(static_cast<int>(item->staff()->idx()) + 1);
    // TODO: add staff number if centered between staves
    staffAtt->SetStaff(staffList);
}

double Convert::tstampFromFraction(const engraving::Fraction& fraction, const engraving::Fraction& timesig)
{
    return (double)fraction.numerator() / fraction.denominator() * timesig.denominator() + 1.0;
}

/**
 * Approximate a fraction for an MEI timestamp (double) values
 * From https://stackoverflow.com/questions/26643695/
 */

engraving::Fraction Convert::tstampToFraction(double tstamp, const engraving::Fraction& timesig)
{
    // Value is -1.0 because MEI time stamps are one-based
    // However, make sure it is not smaller than 0.0, which means that a tstamp 0.0 is moved to 1.0 (the first note)
    tstamp = std::max(tstamp - 1.0, 0.0);

    const int cycles = 10;
    const double precision = 0.01;

    int sign = tstamp > 0 ? 1 : -1;
    tstamp = tstamp * sign; //abs(number);
    double new_number, whole_part;
    double decimal_part = tstamp - (int)tstamp;
    int counter = 0;

    std::valarray<double> vec_1{ double((int)tstamp), 1 }, vec_2{ 1, 0 }, temporary;

    while (decimal_part > precision && counter < cycles) {
        new_number = 1 / decimal_part;
        whole_part = (int)new_number;

        temporary = vec_1;
        vec_1 = whole_part * vec_1 + vec_2;
        vec_2 = temporary;

        decimal_part = new_number - whole_part;
        counter += 1;
    }

    return engraving::Fraction(sign * vec_1[0], vec_1[1]) / timesig.denominator();
}

/**
 * Return a static instance of the register.
 */

UIDRegister* UIDRegister::instance()
{
    static UIDRegister uidRegister;
    return &uidRegister;
}
