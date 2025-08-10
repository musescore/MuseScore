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

#ifndef MU_IMPORTEXPORT_MEICONVERTER_H
#define MU_IMPORTEXPORT_MEICONVERTER_H

#include "engraving/dom/articulation.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/volta.h"

#include "iengravingfontsprovider.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/cmnornaments.h"
#include "thirdparty/libmei/fingering.h"
#include "thirdparty/libmei/harmony.h"
#include "thirdparty/libmei/shared.h"

namespace mu::iex::mei {
enum TextCell {
    TopLeft = 0,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    CellCount
};

enum ElisionType {
    ElisionNone = 0,
    ElisionFirst,
    ElisionMiddle,
    ElisionLast
};

// SMuFL in @glyph.auth
#define SMUFL_AUTH "smufl"

// The @type attribute for <pb> and <sb>
#define BREAK_TYPE "mscore-manual"
// The @type attribute prefix for indicating beam in <chord>, <note> or <rest>
#define BEAM_ELEMENT_TYPE "mscore-beam"
// The @type attribute prefix for expression and playtechannotation type in <dir>
#define DIR_TYPE "mscore-"
// The @type attribute prefix for volta repeat ending in <ending>
#define ENDING_REPEAT_TYPE "mscore-ending-"
// The @type attribute prefix for harmony type in <harm>
#define HARMONY_TYPE "mscore-"
// The @type attribute prefix for jump type in <repeatMark>
#define JUMP_TYPE "mscore-jump-"
// The @type attribute prefix for measure repeat count in <measure>
#define MEASURE_REPEAT_TYPE "mscore-repeat-"
// The @type attribute prefix for marker type in <repeatMark>
#define MARKER_TYPE "mscore-marker-"
// The @type attribute prefix for tempo inferring type in <tempo>
#define TEMPO_INFER_FROM_TEXT "mscore-infer-from-text"
// The @type attribute prefixes for ornament interval (above/below)
#define INTERVAL_ABOVE "mscore-above-"
#define INTERVAL_BELOW "mscore-below-"

class Convert
{
    // The fallback font is used to convert smufl codes (char32_t) to engraving::SymId
    INJECT_STATIC(engraving::IEngravingFontsProvider, engravingFonts)
    INJECT_STATIC(engraving::IEngravingConfiguration, engravingConfiguration)
public:

    /**
     * Structures for methods needing to return a group of variables
     */

    struct BracketStruct {
        engraving::BracketType bracketType = engraving::BracketType::NO_BRACKET;
        int barLineSpan = 0;
    };

    struct MeasureStruct {
        bool irregular = false;
        int n = 0;
        bool repeatStart = false;
        engraving::BarLineType endBarLineType = engraving::BarLineType::NORMAL;
        bool repeatEnd = false;
        int repeatCount = 0;
    };

    struct OrnamStruct {
        engraving::AccidentalType accidTypeAbove = engraving::AccidentalType::NONE;
        engraving::AccidentalType accidTypeBelow = engraving::AccidentalType::NONE;
    };

    struct StaffStruct {
        int lines;
        bool invisible;
        double scale;
        engraving::Color color;
        engraving::Interval interval;
    };

    struct PitchStruct {
        int pitch = 0;
        int tpc2 = 0;
        engraving::AccidentalType accidType = engraving::AccidentalType::NONE;
        engraving::AccidentalBracket accidBracket = engraving::AccidentalBracket::NONE;
        engraving::AccidentalRole accidRole = engraving::AccidentalRole::AUTO;
    };

    /**
     * Methods for checking which element to create depending on some attribute values of the MuseScore or MEI element
     */
    static engraving::ElementType elementTypeForDir(const libmei::Element& meiElement);
    static engraving::ElementType elementTypeForDirWithExt(const libmei::Element& meiElement);
    static engraving::ElementType elementTypeFor(const libmei::RepeatMark& meiRepeatMark);
    static bool isDirWithExt(const libmei::Dir& meiDir);
    static bool isMordent(const engraving::Ornament* ornament);
    static bool isTrill(const engraving::Ornament* ornament);
    static bool isTurn(const engraving::Ornament* ornament);

    /**
     * Methods for converting from and to MEI
     */

    static engraving::AccidentalType accidFromMEI(const libmei::data_ACCIDENTAL_WRITTEN meiAccid, bool& warning);
    static libmei::data_ACCIDENTAL_WRITTEN accidToMEI(const engraving::AccidentalType accid);

    static engraving::AccidentalVal accidGesFromMEI(const libmei::data_ACCIDENTAL_GESTURAL meiAccid, bool& warning);
    static libmei::data_ACCIDENTAL_GESTURAL accidGesToMEI(const engraving::AccidentalVal accid);

    static engraving::ArticulationAnchor anchorFromMEI(const libmei::data_STAFFREL meiPlace, bool& warning);
    static libmei::data_STAFFREL anchorToMEI(engraving::ArticulationAnchor anchor);

    static void arpegFromMEI(engraving::Arpeggio* arpeggio, const libmei::Arpeg& meiArpeg, bool& warning);
    static libmei::Arpeg arpegToMEI(const engraving::Arpeggio* arpeggio);

    static void articFromMEI(engraving::Articulation* articulation, const libmei::Artic& meiArtic, bool& warning);
    static libmei::Artic articToMEI(const engraving::Articulation* articulation);

    static engraving::BarLineType barlineFromMEI(const libmei::data_BARRENDITION meiBarline, bool& warning);
    static libmei::data_BARRENDITION barlineToMEI(engraving::BarLineType barline);

    static engraving::BeamMode beamFromMEI(const std::string& typeAtt, const std::string& prefix, bool& warning);
    static std::string beamToMEI(engraving::BeamMode beamMode, const std::string& prefix);

    static engraving::BeamMode breaksecFromMEI(int breaksec, bool& warning);
    static int breaksecToMEI(engraving::BeamMode beamMode);

    static void breathFromMEI(engraving::Breath* breath, const libmei::Breath& meiBreath, bool& warning);
    static libmei::Breath breathToMEI(const engraving::Breath* breath);

    static void caesuraFromMEI(engraving::Breath* breath, const libmei::Caesura& meiCeasura, bool& warning);
    static libmei::Caesura caesuraToMEI(const engraving::Breath* breath);

    static engraving::ClefType clefFromMEI(const libmei::Clef& meiClef, bool& warning);
    static libmei::Clef clefToMEI(engraving::ClefType clef);

    static engraving::ClefType clefFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning);

    static void colorFromMEI(engraving::EngravingItem* item, const libmei::Element& meiElement);
    static void colorToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement);

    static void colorlineFromMEI(engraving::SLine* line, const libmei::Element& meiElement);
    static void colorlineToMEI(const engraving::SLine* line, libmei::Element& meiElement);

    static void dirFromMEI(engraving::TextBase* textBase, const muse::StringList& meiLines, const libmei::Dir& meiDir, bool& warning);
    static void dirFromMEI(engraving::TextLineBase* textLineBase, const muse::StringList& meiLines, const libmei::Dir& meiDir,
                           bool& warning);
    static libmei::Dir dirToMEI(const engraving::TextBase* textBase, muse::StringList& meiLines);
    static libmei::Dir dirToMEI(const engraving::TextLineBase* textLineBase, muse::StringList& meiLines);

    static engraving::DirectionV curvedirFromMEI(const libmei::curvature_CURVEDIR meiCurvedir, bool& warning);
    static libmei::curvature_CURVEDIR curvedirToMEI(engraving::DirectionV curvedir);

    static engraving::DurationType durFromMEI(const libmei::data_DURATION meiDuration, bool& warning);
    static libmei::data_DURATION durToMEI(const engraving::DurationType duration);

    static void dynamFromMEI(engraving::Dynamic* dynamic, const muse::StringList& meiLines, const libmei::Dynam& meiDynam, bool& warning);
    static libmei::Dynam dynamToMEI(const engraving::Dynamic* dynamic, muse::StringList& meiLines);

    static void endingFromMEI(engraving::Volta* volta, const libmei::Ending& meiEnding, bool& warning);
    static libmei::Ending endingToMEI(const engraving::Volta* volta);

    static void fFromMEI(engraving::FiguredBassItem* figuredBassItem, const muse::StringList& meiLines, const libmei::F& meiF,
                         bool& warning);
    static libmei::F fToMEI(const engraving::FiguredBassItem* figuredBassItem, muse::StringList& meiLines);

    static void fbFromMEI(engraving::FiguredBass* figuredBass, const libmei::Harm& meiHarm, const libmei::Fb& meiFb, bool& warning);
    static std::pair<libmei::Harm, libmei::Fb> fbToMEI(const engraving::FiguredBass* figuredBass);

    static void fermataFromMEI(engraving::Fermata* fermata, const libmei::Fermata& meiFermata, bool& warning);
    static libmei::Fermata fermataToMEI(const engraving::Fermata* fermata);

    static void fingFromMEI(engraving::Fingering* fing, const muse::StringList& meiLines, const libmei::Fing& meiFing, bool& warning);
    static libmei::Fing fingToMEI(const engraving::Fingering* fing, muse::StringList& meiLines);

    static void glissFromMEI(engraving::Glissando* gliss, const libmei::Gliss& meiGliss, bool& warning);
    static libmei::Gliss glissToMEI(const engraving::Glissando* gliss);

    static std::pair<bool, engraving::NoteType> gracegrpFromMEI(const libmei::graceGrpLog_ATTACH meiAttach,
                                                                const libmei::data_GRACE meiGrace, bool& warning);
    static std::pair<libmei::graceGrpLog_ATTACH, libmei::data_GRACE> gracegrpToMEI(bool isAfter, engraving::NoteType noteType);

    static void hairpinFromMEI(engraving::Hairpin* haipin, const libmei::Hairpin& meiHairpin, bool& warning);
    static libmei::Hairpin hairpinToMEI(const engraving::Hairpin* hairpin);

    static void harmFromMEI(engraving::Harmony* harmony, const muse::StringList& meiLines, const libmei::Harm& meiHarm, bool& warning);
    static libmei::Harm harmToMEI(const engraving::Harmony* harmony, muse::StringList& meiLines);

    static void harpPedalFromMEI(engraving::HarpPedalDiagram* harpPedalDiagram, const libmei::HarpPedal& meiHarpPedal, bool& warning);
    static libmei::HarpPedal harpPedalToMEI(const engraving::HarpPedalDiagram* harpPedalDiagram);

    static engraving::PedalPosition harpPedalPositionFromMEI(const libmei::data_HARPPEDALPOSITION& pedalPosition);
    static libmei::data_HARPPEDALPOSITION harpPedalPositionToMEI(const engraving::PedalPosition& pedalPosition);

    static void layerIdentFromMEI(engraving::EngravingItem* item, const libmei::Element& meiElement);
    static void layerIdentToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement);

    static void lvFromMEI(engraving::LaissezVib* lv, const libmei::Lv& meiLv, bool& warning);

    static void jumpFromMEI(engraving::Jump* jump, const libmei::RepeatMark& meiRepeatMark, bool& warning);
    static libmei::RepeatMark jumpToMEI(const engraving::Jump* jump, muse::String& text);

    static engraving::Key keyFromMEI(const libmei::data_KEYSIGNATURE& meiKeysig, bool& warning);
    static libmei::data_KEYSIGNATURE keyToMEI(const engraving::Key key);

    static engraving::LineType lineFromMEI(const libmei::data_LINEFORM meiLine, bool& warning);
    static libmei::data_LINEFORM lineToMEI(engraving::LineType line);

    static void markerFromMEI(engraving::Marker* marker, const libmei::RepeatMark& meiRepeatMark, bool& warning);
    static libmei::RepeatMark markerToMEI(const engraving::Marker* marker, muse::String& text);

    static MeasureStruct measureFromMEI(const libmei::Measure& meiMeasure, bool& warning);
    static libmei::Measure measureToMEI(const engraving::Measure* measure, int& measureN, bool& isLastIrregular);

    static std::pair<engraving::Fraction, engraving::TimeSigType> meterFromMEI(const libmei::ScoreDef& meiScoreDef, bool& warning);
    static std::pair<engraving::Fraction, engraving::TimeSigType> meterFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning);
    static libmei::StaffDef meterToMEI(const engraving::Fraction& fraction, engraving::TimeSigType tsType);

    static OrnamStruct mordentFromMEI(engraving::Ornament* ornament, const libmei::Mordent& meiMordent, bool& warning);
    static libmei::Mordent mordentToMEI(const engraving::Ornament* ornament);

    static void octaveFromMEI(engraving::Ottava* ottava, const libmei::Octave& meiOctave, bool& warning);
    static libmei::Octave octaveToMEI(const engraving::Ottava* ottava);

    static OrnamStruct ornamFromMEI(engraving::Ornament* ornament, const libmei::Element& meiElement, bool& warning);
    static libmei::Ornam ornamToMEI(const engraving::Ornament* ornament);
    static void ornamToMEI(const engraving::Ornament* ornament, libmei::Element& meiElement);

    static void ornamintervaleFromMEI(engraving::Ornament* ornament, const std::string& meiType);
    static muse::String ornamintervalToMEI(const engraving::Ornament* ornament);

    static void pedalFromMEI(engraving::Pedal* pedal, const libmei::Pedal& meiPedal, bool& warning);
    static libmei::Pedal pedalToMEI(const engraving::Pedal* pedal);

    static PitchStruct pitchFromMEI(const libmei::Note& meiNote, const libmei::Accid& meiAccid, const engraving::Interval& interval,
                                    bool& warning);
    static std::pair<libmei::Note, libmei::Accid> pitchToMEI(const engraving::Note* note, const engraving::Accidental* accid,
                                                             const engraving::Interval& interval);

    static engraving::PlacementV placeFromMEI(const libmei::data_STAFFREL meiPlace, bool& warning);
    static libmei::data_STAFFREL placeToMEI(engraving::PlacementV place);

    static engraving::DirectionV directionFromMEI(const libmei::data_STAFFREL meiPlace);
    static libmei::data_STAFFREL directionToMEI(engraving::DirectionV direction);

    static void slurFromMEI(engraving::SlurTie* slur, const libmei::Slur& meiSlur, bool& warning);
    static libmei::Slur slurToMEI(const engraving::SlurTie* slur);

    static engraving::SlurStyleType slurstyleFromMEI(const libmei::data_LINEFORM meiLine, bool& warning);
    static libmei::data_LINEFORM slurstyleToMEI(engraving::SlurStyleType slurstyle);

    static StaffStruct staffFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning);
    static libmei::StaffDef staffToMEI(const engraving::Staff* staff);

    static BracketStruct staffGrpFromMEI(const libmei::StaffGrp& meiStaffGrp);
    static libmei::StaffGrp staffGrpToMEI(const engraving::BracketType, int barLineSpan);

    static void staffIdentToMEI(const engraving::EngravingItem* item, libmei::Element& meiElement);

    static std::pair<engraving::DirectionV, bool> stemFromMEI(const libmei::AttStems& meiStemsAtt, bool& warning);
    static std::pair<libmei::data_STEMDIRECTION, double> stemToMEI(const engraving::DirectionV direction, bool noStem);

    static engraving::TremoloType stemModFromMEI(const libmei::data_STEMMODIFIER meiStemMod);
    static libmei::data_STEMMODIFIER stemModToMEI(const engraving::TremoloSingleChord* tremolo);

    static void sylFromMEI(engraving::Lyrics* lyrics, const libmei::Syl& meiSyl, ElisionType elision, bool& warning);
    static libmei::Syl sylToMEI(const engraving::Lyrics* lyrics, ElisionType elision);

    static engraving::BracketType symbolFromMEI(const libmei::staffGroupingSym_SYMBOL meiGrpSym);
    static libmei::staffGroupingSym_SYMBOL symbolToMEI(const engraving::BracketType bracket);

    static void tempoFromMEI(engraving::TempoText* tempo, const muse::StringList& meiLines, const libmei::Tempo& meiTempo, bool& warning);
    static libmei::Tempo tempoToMEI(const engraving::TempoText* tempoText, muse::StringList& meiLines);

    static engraving::TextStyleType textFromMEI(const libmei::Rend& meiRend, bool& warning);
    static std::tuple<libmei::Rend, TextCell, muse::String> textToMEI(const engraving::Text* text);

    using textWithSmufl = std::list<std::pair<bool, muse::String> >;

    static void textFromMEI(muse::String& text, const textWithSmufl& textBlocks);
    static void textToMEI(textWithSmufl& textBlocks, const muse::String& text);

    static void tieFromMEI(engraving::SlurTie* tie, const libmei::Tie& meiTie, bool& warning);
    static libmei::Tie tieToMEI(const engraving::SlurTie* tie);

    static OrnamStruct trillFromMEI(engraving::Ornament* ornament, const libmei::Trill& meiTrill, bool& warning);
    static libmei::Trill trillToMEI(const engraving::Ornament* ornament);

    static void tupletFromMEI(engraving::Tuplet* tuplet, const libmei::Tuplet& meiTuplet, bool& warning);
    static libmei::Tuplet tupletToMEI(const engraving::Tuplet* tuplet);

    static OrnamStruct turnFromMEI(engraving::Ornament* ornament, const libmei::Turn& meiTurn, bool& warning);
    static libmei::Turn turnToMEI(const engraving::Ornament* ornament);

    /**
     * Helper methods
     */
    static bool hasTypeValue(const std::string& typeStr, const std::string& value);
    static bool getTypeValueWithPrefix(const std::string& typeStr, const std::string& prefix, std::string& value);
    static std::list<std::string> getTypeValuesWithPrefix(const std::string& typeStr, const std::string& prefix);
    static double tstampFromFraction(const engraving::Fraction& fraction, const engraving::Fraction& timesig);
    static engraving::Fraction tstampToFraction(double tstamp, const engraving::Fraction& timesig);

    static muse::StringList logs;

private:
    static inline std::map<engraving::MarkerType, std::string> s_markerTypes = {
        { engraving::MarkerType::VARSEGNO, "segno-variation" },
        { engraving::MarkerType::VARCODA, "varied-coda" },
        { engraving::MarkerType::TOCODA, "to-coda" },
        { engraving::MarkerType::TOCODASYM, "to-coda-symbol" },
        { engraving::MarkerType::DA_CODA, "da-coda" },
        { engraving::MarkerType::DA_DBLCODA, "da-double-coda" }
    };

    static inline std::map<engraving::JumpType, std::string> s_jumpTypes = {
        { engraving::JumpType::DC_AL_FINE, "dc-al-fine" },
        { engraving::JumpType::DC_AL_CODA, "dc-al-coda" },
        { engraving::JumpType::DS_AL_CODA, "ds-al-coda" },
        { engraving::JumpType::DS_AL_FINE, "ds-al-fine" },
        { engraving::JumpType::DC_AL_DBLCODA, "dc-al-double-coda" },
        { engraving::JumpType::DS_AL_DBLCODA, "ds-al-double-coda" },
        { engraving::JumpType::DSS, "dal-segno-segno" },
        { engraving::JumpType::DSS_AL_CODA, "dss-al-coda" },
        { engraving::JumpType::DSS_AL_DBLCODA, "dss-al-double-coda" },
        { engraving::JumpType::DSS_AL_FINE, "dss-al-fine" }
    };

    /** Since MuseScore uses composed glyph - see EngravingFont::loadComposedGlyphs */
    static inline std::map<engraving::SymId, std::string> s_mordentGlyphs = {
        { engraving::SymId::ornamentPrallMordent, "ornamentPrecompTrillWithMordent" },
        { engraving::SymId::ornamentUpPrall, "ornamentPrecompSlideTrillDAnglebert" },
        { engraving::SymId::ornamentUpMordent, "ornamentPrecompSlideTrillBach" },
        { engraving::SymId::ornamentPrallDown, "ornamentPrecompTrillLowerSuffix" },
        { engraving::SymId::ornamentDownMordent, "ornamentPrecompTurnTrillBach" },
        { engraving::SymId::ornamentPrallUp, "ornamentPrecompTrillSuffixDandrieu" },
        { engraving::SymId::ornamentLinePrall, "ornamentPrecompAppoggTrill" },
        { engraving::SymId::ornamentPrecompSlide, "ornamentSchleifer" }
    };
};

/**
 * A class with a static instance for registering MEI xml:ids.
 * When objects are created from the MeiImporter the mapping is registered there.
 * It is re-used in the MeiExporter to preserved xml:ids for imported objects.
 */

class UIDRegister
{
public:

    static UIDRegister* instance();

    void reg(const engraving::EngravingItem* item, const std::string& meiUID) { m_uids[reinterpret_cast<uintptr_t>(item)] = meiUID; }
    bool hasUid(const engraving::EngravingItem* item) const { return m_uids.count(reinterpret_cast<uintptr_t>(item)) > 0; }
    std::string uid(const engraving::EngravingItem* item) const { return m_uids.at(reinterpret_cast<uintptr_t>(item)); }
    void clear() { m_uids.clear(); }

private:
    std::unordered_map<uintptr_t, std::string> m_uids;
};
} // namespace

#endif // MU_IMPORTEXPORT_MEICONVERTER_H
