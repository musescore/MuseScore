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

#ifndef __IMPORTMXMLPASS1_H__
#define __IMPORTMXMLPASS1_H__

#include "global/serialization/xmlstreamreader.h"
#include "global/containers.h"
#include "draw/types/geometry.h"

#include "importxmlfirstpass.h"
#include "musicxmlsupport.h"
#include "musicxmltypes.h"
#include "musicxmltupletstate.h"

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

enum class MusicXMLExporterSoftware : char {
    SIBELIUS,
    DOLET6,
    DOLET8,
    FINALE,
    NOTEFLIGHT,
    OTHER
};

//---------------------------------------------------------
//   declarations
//---------------------------------------------------------

bool isLikelyCreditText(const String& text, const bool caseInsensitive);
bool isLikelySubtitleText(const String& text, const bool caseInsensitive);

//---------------------------------------------------------
//   MusicXMLParserPass1
//---------------------------------------------------------

class MxmlLogger;

class MusicXMLParserPass1
{
public:
    MusicXMLParserPass1(Score* score, MxmlLogger* logger);
    void initPartState(const String& partId);
    Err parse(const muse::ByteArray& data);
    Err parse();
    String errors() const { return m_errors; }
    void scorePartwise();
    void identification();
    void credit(CreditWordsList& credits);
    void defaults();
    void pageLayout(PageFormat& pf, const double conversion);
    void partList(MusicXmlPartGroupList& partGroupList);
    void partGroup(const int scoreParts, MusicXmlPartGroupList& partGroupList, MusicXmlPartGroupMap& partGroups, String& curPartGroupName);
    void scorePart(const String& curPartGroupName);
    void scoreInstrument(const String& partId, const String& curPartGroupName);
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
    Fraction calcTicks(const int& intTicks, const int& _divisions, const muse::XmlStreamReader* xmlReader);
    Fraction calcTicks(const int& intTicks) { return calcTicks(intTicks, m_divs, &m_e); }
    void duration(Fraction& dura, muse::XmlStreamReader& e);
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
    Part* getPart(const String& id) const { return muse::value(m_partMap, id); }
    MusicXmlPart getMusicXmlPart(const String& id) const { return muse::value(m_parts, id); }
    MusicXMLInstruments getInstruments(const String& id) const { return muse::value(m_instruments, id); }
    void setDrumsetDefault(const String& id, const String& instrId, const NoteHeadGroup hg, const int line, const DirectionV sd);
    MusicXmlInstrList getInstrList(const String& id) const;
    MusicXmlIntervalList getIntervals(const String& id) const;
    Fraction getMeasureStart(const size_t i) const;
    int octaveShift(const String& id, const staff_idx_t staff, const Fraction& f) const;
    const CreditWordsList& credits() const { return m_credits; }
    bool hasBeamingInfo() const { return m_hasBeamingInfo; }
    bool isVocalStaff(const String& partId) const { return m_parts.at(partId).isVocalStaff(); }
    bool isPercussionStaff(const String& partId) const { return m_parts.at(partId).isPercussionStaff(); }
    static VBox* createAndAddVBoxForCreditWords(Score* score);
    void createDefaultHeader(Score* const score);
    void createMeasuresAndVboxes(Score* const score, const std::vector<Fraction>& ml, const std::vector<Fraction>& ms,
                                 const std::set<int>& systemStartMeasureNrs, const std::set<int>& pageStartMeasureNrs,
                                 const CreditWordsList& crWords, const muse::Size& pageSize);
    void setHasInferredHeaderText(bool b) { m_hasInferredHeaderText = b; }
    bool hasInferredHeaderText() const { return m_hasInferredHeaderText; }
    int maxDiff() const { return m_maxDiff; }
    void insertAdjustedDuration(Fraction key, Fraction value) { m_adjustedDurations.insert({ key, value }); }
    std::map<Fraction, Fraction>& adjustedDurations() { return m_adjustedDurations; }
    void insertSeenDenominator(int val) { m_seenDenominators.emplace(val); }
    MusicXMLExporterSoftware exporterSoftware() const { return m_exporterSoftware; }
    bool sibOrDolet() const;
    bool dolet() const;

private:
    // functions
    void addError(const String& error);        // Add an error to be shown in the GUI
    void setExporterSoftware(String& exporter);

    // generic pass 1 data
    muse::XmlStreamReader m_e;
    MusicXMLExporterSoftware m_exporterSoftware = MusicXMLExporterSoftware::OTHER;   // Software which exported the file
    int m_divs = 0;                              // Current MusicXML divisions value
    std::map<String, MusicXmlPart> m_parts;      // Parts data, mapped on part id
    std::set<int> m_systemStartMeasureNrs;       // Measure numbers of measures starting a page
    std::set<int> m_pageStartMeasureNrs;         // Measure numbers of measures starting a page
    std::vector<Fraction> m_measureLength;       // Length of each measure
    std::vector<Fraction> m_measureStart;        // Start time of each measure
    CreditWordsList m_credits;                   // All credits collected
    PartMap m_partMap;                           // TODO merge into MusicXmlPart ??
    std::map<String, MusicXMLInstruments> m_instruments;   // instruments for each part, mapped on part id
    Score* m_score = nullptr;                    // MuseScore score
    MxmlLogger* m_logger = nullptr;              // Error logger
    String m_errors;                             // Errors to present to the user
    bool m_hasBeamingInfo = false;               // Whether the score supports or contains beaming info
    bool m_hasInferredHeaderText = false;

    // part specific data (TODO: move to part-specific class)
    Fraction m_timeSigDura;                      // Measure duration according to last timesig read
    std::map<int, MxmlOctaveShiftDesc> m_octaveShifts;   // Pending octave-shifts
    muse::Size m_pageSize;                             // Page width read from defaults

    const int m_maxDiff = 5;                   // Duration rounding tick threshold;
    std::map<Fraction, Fraction> m_adjustedDurations;  // Rounded durations
    std::set<int> m_seenDenominators;          // Denominators seen. Used for rounding errors.
};
} // namespace Ms
#endif
