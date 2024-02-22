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

#ifndef __IMPORTMXMLPASS1_H__
#define __IMPORTMXMLPASS1_H__

#include "global/serialization/xmlstreamreader.h"
#include "global/containers.h"
#include "global/types/flags.h"
#include "draw/types/geometry.h"

#include "importxmlfirstpass.h"
#include "musicxml.h" // for the creditwords and MusicXmlPartGroupList definitions
#include "musicxmlsupport.h"

#include "engraving/engravingerrors.h"

namespace mu::engraving {
class Score;

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

struct PageFormat {
    SizeF size;                         // automatically initialized (to invalid)
    double printableWidth = 5;          // _width - left margin - right margin
    double evenLeftMargin = 0.2;        // values in inch
    double oddLeftMargin = 0.2;
    double evenTopMargin = 0.2;
    double evenBottomMargin = 0.2;
    double oddTopMargin = 0.2;
    double oddBottomMargin = 0.2;
    bool twosided = false;
};

typedef std::map<String, Part*> PartMap;
typedef std::map<int, MusicXmlPartGroup*> MusicXmlPartGroupMap;

//---------------------------------------------------------
//   MxmlOctaveShiftDesc
//---------------------------------------------------------

struct MxmlOctaveShiftDesc {
    enum class Type : char {
        UP, DOWN, STOP, NONE
    };
    Type tp;
    short size;
    Fraction time;
    short num;
    MxmlOctaveShiftDesc()
        : tp(Type::NONE), size(0), num(-1) {}
    MxmlOctaveShiftDesc(Type _tp, short _size, Fraction _tm)
        : tp(_tp), size(_size), time(_tm), num(-1) {}
};

//---------------------------------------------------------
//   MxmlStartStop (also used in pass 2)
//---------------------------------------------------------

enum class MxmlStartStop : char {
    NONE, START, STOP
};

enum class MxmlTupletFlag : char {
    NONE = 0,
    STOP_PREVIOUS = 1,
    START_NEW = 2,
    ADD_CHORD = 4,
    STOP_CURRENT = 8
};

typedef Flags<MxmlTupletFlag> MxmlTupletFlags;

struct MxmlTupletState {
    void addDurationToTuplet(const Fraction duration, const Fraction timeMod);
    MxmlTupletFlags determineTupletAction(const Fraction noteDuration, const Fraction timeMod, const MxmlStartStop tupletStartStop,
                                          const TDuration normalType, Fraction& missingPreviousDuration, Fraction& missingCurrentDuration);
    bool inTuplet = false;
    bool implicit = false;
    int actualNotes = 1;
    int normalNotes = 1;
    Fraction duration { 0, 1 };
    int tupletType = 0;   // smallest note type in the tuplet // TODO_NOW rename ?
    int tupletCount = 0;   // number of smallest notes in the tuplet // TODO_NOW rename ?
};

using MxmlTupletStates = std::map<String, MxmlTupletState>;

//---------------------------------------------------------
//   declarations
//---------------------------------------------------------

void determineTupletFractionAndFullDuration(const Fraction duration, Fraction& fraction, Fraction& fullDuration);
Fraction missingTupletDuration(const Fraction duration);

//---------------------------------------------------------
//   MusicXMLParserPass1
//---------------------------------------------------------

class MxmlLogger;

class MusicXMLParserPass1
{
public:
    MusicXMLParserPass1(Score* score, MxmlLogger* logger);
    void initPartState(const String& partId);
    Err parse(const ByteArray& data);
    Err parse();
    String errors() const { return m_errors; }
    void scorePartwise();
    void identification();
    void credit(CreditWordsList& credits);
    void defaults();
    void pageLayout(PageFormat& pf, const double conversion);
    void partList(MusicXmlPartGroupList& partGroupList);
    void partGroup(const int scoreParts, MusicXmlPartGroupList& partGroupList, MusicXmlPartGroupMap& partGroups);
    void scorePart();
    void scoreInstrument(const String& partId);
    void setStyle(const String& type, const double val);
    void midiInstrument(const String& partId);
    void part();
    void measure(const String& partId, const Fraction cTime, Fraction& mdur, VoiceOverlapDetector& vod, const int measureNr);
    void print(const int measureNr);
    void attributes(const String& partId, const Fraction cTime);
    void clef(const String& partId);
    void time(const Fraction cTime);
    void transpose(const String& partId, const Fraction& tick);
    void divisions();
    void direction(const String& partId, const Fraction& cTime);
    void directionType(const Fraction cTime, std::vector<MxmlOctaveShiftDesc>& starts, std::vector<MxmlOctaveShiftDesc>& stops);
    void handleOctaveShift(const Fraction& cTime, const String& type, short size, MxmlOctaveShiftDesc& desc);
    void notations(MxmlStartStop& tupletStartStop);
    void note(const String& partId, const Fraction& cTime, Fraction& missingPrev, Fraction& dura, Fraction& missingCurr,
              VoiceOverlapDetector& vod, MxmlTupletStates& tupletStates);
    void notePrintSpacingNo(Fraction& dura);
    Fraction calcTicks(const int& intTicks, const int& _divisions, const XmlStreamReader* xmlReader);
    Fraction calcTicks(const int& intTicks) { return calcTicks(intTicks, m_divs, &m_e); }
    void duration(Fraction& dura, XmlStreamReader& e);
    void duration(Fraction& dura) { duration(dura, m_e); }
    void forward(Fraction& dura);
    void backup(Fraction& dura);
    void timeModification(Fraction& timeMod);
    void pitch(int& step, float& alter, int& oct);
    void rest();
    void skipLogCurrElem();
    bool determineMeasureLength(std::vector<Fraction>& ml) const;
    VoiceList getVoiceList(const String& id) const;
    bool determineStaffMoveVoice(const String& id, const int mxStaff, const int& mxVoice, int& msMove, int& msTrack, int& msVoice) const;
    int voiceToInt(const String& voice);
    track_idx_t trackForPart(const String& id) const;
    bool hasPart(const String& id) const;
    Part* getPart(const String& id) const { return mu::value(m_partMap, id); }
    MusicXmlPart getMusicXmlPart(const String& id) const { return mu::value(m_parts, id); }
    MusicXMLInstruments getInstruments(const String& id) const { return mu::value(m_instruments, id); }
    void setDrumsetDefault(const String& id, const String& instrId, const NoteHeadGroup hg, const int line, const DirectionV sd);
    MusicXmlInstrList getInstrList(const String& id) const;
    MusicXmlIntervalList getIntervals(const String& id) const;
    Fraction getMeasureStart(const size_t i) const;
    int octaveShift(const String& id, const staff_idx_t staff, const Fraction& f) const;
    const CreditWordsList& credits() const { return m_credits; }
    bool hasBeamingInfo() const { return m_hasBeamingInfo; }
    bool isVocalStaff(const String& id) const { return m_parts.at(id).isVocalStaff(); }
    static VBox* createAndAddVBoxForCreditWords(Score* score, const int miny = 0, const int maxy = 75);
    int maxDiff() const { return m_maxDiff; }
    void insertAdjustedDuration(Fraction key, Fraction value) { m_adjustedDurations.insert({ key, value }); }
    std::map<Fraction, Fraction>& adjustedDurations() { return m_adjustedDurations; }
    void insertSeenDenominator(int val) { m_seenDenominators.emplace(val); }

private:
    // functions
    void addError(const String& error);        // Add an error to be shown in the GUI

    // generic pass 1 data
    XmlStreamReader m_e;
    int m_divs = 0;                              // Current MusicXML divisions value
    std::map<String, MusicXmlPart> m_parts;     // Parts data, mapped on part id
    std::set<int> m_systemStartMeasureNrs;       // Measure numbers of measures starting a page
    std::set<int> m_pageStartMeasureNrs;         // Measure numbers of measures starting a page
    std::vector<Fraction> m_measureLength;           // Length of each measure
    std::vector<Fraction> m_measureStart;            // Start time of each measure
    CreditWordsList m_credits;                   // All credits collected
    PartMap m_partMap;                           // TODO merge into MusicXmlPart ??
    std::map<String, MusicXMLInstruments> m_instruments;   // instruments for each part, mapped on part id
    Score* m_score = nullptr;                    // MuseScore score
    MxmlLogger* m_logger = nullptr;              // Error logger
    String m_errors;                             // Errors to present to the user
    bool m_hasBeamingInfo = false;               // Whether the score supports or contains beaming info

    // part specific data (TODO: move to part-specific class)
    Fraction m_timeSigDura;                      // Measure duration according to last timesig read
    std::map<int, MxmlOctaveShiftDesc> m_octaveShifts;   // Pending octave-shifts
    Size m_pageSize;                             // Page width read from defaults

    const int m_maxDiff = 5;                   // Duration rounding tick threshold;
    std::map<Fraction, Fraction> m_adjustedDurations;  // Rounded durations
    std::set<int> m_seenDenominators;          // Denominators seen. Used for rounding errors.
};
} // namespace Ms
#endif
