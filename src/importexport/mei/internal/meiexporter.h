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

#ifndef MU_IMPORTEXPORT_MEIEXPORTER_H
#define MU_IMPORTEXPORT_MEIEXPORTER_H

#include "engraving/types/fraction.h"

#include "modularity/ioc.h"
#include "imeiconfiguration.h"

#include "thirdparty/pugixml.hpp"

namespace mu::engraving {
class Fraction;
class Measure;
class Score;
class Staff;
}

namespace mu::iex::mei {
enum measureElementCounter {
    BREATH_M = 0,
    CAESURA_M,
    DYNAM_M,
    FERMATA_M,
    HARM_M,
    REPEATMARK_M,
    SLUR_M,
    TEMPO_M,
    TIE_M,
    UNSPECIFIED_M
};

enum layerElementCounter {
    ACCID_L = 0,
    BEAM_L,
    CHORD_L,
    GRACEGRP_L,
    NOTE_L,
    REST_L,
    TUPLET_L,
    UNSPECIFIED_L,
};

/**
 * Class of exporting Music Encoding Initiative (MEI) files
 */

class MeiExporter
{
    INJECT_STATIC(mu::iex::mei::IMeiConfiguration, configuration)

public:
    MeiExporter(engraving::Score* s) { m_score = s; }
    bool write(QIODevice& destinationDevice);

private:
    bool writeHeader();
    bool writeScore();
    bool writeScoreDef();
    bool writePgHead(const engraving::VBox* vBox);
    bool writeLines(pugi::xml_node node, const StringList& lines);
    bool writeLinesWithSMuFL(pugi::xml_node node, const StringList& lines);
    bool writeScoreDefChange();
    bool writeStaffGrpStart(const engraving::Staff* staff, std::vector<int>& ends, const engraving::Part* part);
    bool writeStaffGrpEnd(const engraving::Staff* staff, std::vector<int>& ends);
    bool writeStaffDef(const engraving::Staff* staff, const engraving::Measure* measure, const engraving::Part* part, bool isPart);
    bool writeLabel(pugi::xml_node node, const engraving::Part* part);
    bool writeEnding(const engraving::Measure* measure);
    bool writeEndingEnd(const engraving::Measure* measure);
    bool writeMeasure(const engraving::Measure* measure, int& measureN, bool& isFirst, bool& wasLastIrregular);
    bool writeStaff(const engraving::Staff* staff, const engraving::Measure* measure);
    bool writeLayer(engraving::track_idx_t track, const engraving::Staff* staff, const engraving::Measure* measure);

    /**
     * Methods for writing MEI elements within a <layer>
     */
    bool writeBeamAndTuplet(const engraving::ChordRest* chordRest, bool& closingBeam, bool& closingTuplet, bool& closingBeamInTuplet);
    bool writeBeamAndTupletEnd(bool closingBeam, bool closingTuplet, bool closingBeamInTuplet);
    bool writeBeam(const engraving::Beam* beam, const engraving::ChordRest* chordRest, bool& closing);
    bool writeClef(const engraving::Clef* clef);
    bool writeChord(const engraving::Chord* chord, const engraving::Staff* staff);
    bool writeGraceGrp(const engraving::Chord* chord, const engraving::Staff* staff, bool isAfter = false);
    bool writeNote(const engraving::Note* note, const engraving::Chord* chord, const engraving::Staff* staff, bool isChord);
    bool writeRest(const engraving::Rest* rest, const engraving::Staff* staff);
    bool writeTuplet(const engraving::Tuplet* tuplet, const engraving::EngravingItem* item, bool& closing);

    /**
     * Methods for writing MEI control events (within <measure>)
     */
    bool writeBreath(const engraving::Breath* breath, const std::string& startid);
    bool writeDynam(const engraving::Dynamic* dynamic, const std::string& startid);
    bool writeFermata(const engraving::Fermata* fermata, const std::string& startid);
    bool writeFermata(const engraving::Fermata* fermata, const libmei::xsdPositiveInteger_List& staffNs, double tstamp);
    bool writeHarm(const engraving::Harmony* harmony, const std::string& startid);
    bool writeRepeatMark(const engraving::Jump* jump, const engraving::Measure* measure);
    bool writeRepeatMark(const engraving::Marker* marker, const engraving::Measure* measure);
    bool writeSlur(const engraving::Slur* slur, const std::string& startid);
    bool writeTempo(const engraving::TempoText* tempoText, const std::string& startid);
    bool writeTie(const engraving::Tie* tie, const std::string& startid);

    /**
     * Methods for writing specific MEI attribute classes within elements
     */
    bool writeBeamTypeAtt(const engraving::ChordRest* chordRest, libmei::AttTyped& typeAtt);
    bool writeStaffIdenAtt(const engraving::ChordRest* chordRest, const engraving::Staff* staff, libmei::AttStaffIdent& staffIdentAtt);
    bool writeStemAtt(const engraving::Chord* chord, libmei::AttStems& stemsAtt);

    /**
     * Helper methods
     */
    bool isCurrentNode(const libmei::Element& element);
    std::list<const engraving::Volta*> findVoltasInMeasure(const engraving::Measure* measure);
    void fillControlEventMap(const std::string& xmlId, const engraving::ChordRest* chordRest);
    std::string findStartIdFor(const engraving::EngravingItem* item);
    void addToRepeatMarkList(const engraving::EngravingItem* repeatMark, pugi::xml_node node, const std::string& xmlId);
    void addJumpToRepeatMarks();
    bool addFermataToMap(engraving::track_idx_t track, const engraving::Segment* segment, const engraving::Measure* measure);
    std::pair<libmei::xsdPositiveInteger_List, double> findTstampFor(const engraving::EngravingItem* item);
    bool isNode(pugi::xml_node node, const String& name);
    pugi::xml_node getLastChordRest(pugi::xml_node node);
    void addEndidToControlEvents();

    /**
     * Methods for generating @xml:ids
     */
    void resetMeasureIDs();
    void resetLayerIDs();
    std::string getSectionXmlId();
    std::string getEndingXmlId();
    std::string getMeasureXmlId();
    std::string getMeasureXmlIdFor(measureElementCounter elementType);
    std::string getStaffXmlId();
    std::string getLayerXmlId();
    std::string getLayerXmlIdFor(layerElementCounter elementType, const engraving::Segment* segment = nullptr);

    /** A structure of holding a list of repeat marks to be post-process in addJumpToRepeatMarks (currently unused) */
    struct RepeatMark {
        const engraving::EngravingItem* m_repeatMark = nullptr;
        pugi::xml_node m_node;
        std::string m_xmlId;
        std::string m_jumptToLabel;
        std::string m_jumpToXmlId;
    };

    /** The Score pointer */
    engraving::Score* m_score = nullptr;

    /** MEI xml element */
    pugi::xml_node m_mei;
    /** Current xml element */
    pugi::xml_node m_currentNode;

    /** When writing layers, keep a pointer to the keySig segment (if any) to prepend a scoreDef if necessary */
    const engraving::Segment* m_keySig;
    /** Same for the timeSig segment */
    const engraving::Segment* m_timeSig;
    /** A flag indicating that we have mulitple sections in the file */
    bool m_hasSections;

    std::map<const engraving::EngravingItem*, std::string> m_startingControlEventMap;
    std::map<const engraving::EngravingItem*, std::string> m_endingControlEventMap;
    std::map<const engraving::EngravingItem*, std::pair<libmei::xsdPositiveInteger_List, double> > m_tstampControlEventMap;
    std::map<const engraving::EngravingItem*, pugi::xml_node> m_openControlEventMap;

    std::list<MeiExporter::RepeatMark> m_repeatMarks;

    /** Counters for generating xml:ids */
    int m_sectionCounter;
    int m_endingCounter;
    int m_measureCounter;
    int m_staffCounter;
    int m_layerCounter;
    /** Sub counters by elementType */
    std::vector<int> m_measureCounterFor;
    std::vector<int> m_layerCounterFor;

    /** map of abbreviations for control events within measure */
    inline static std::map<measureElementCounter, String> s_measureXmlIdMap = {
        { BREATH_M, u"bt" },
        { CAESURA_M, u"cs" },
        { DYNAM_M, u"dn" },
        { FERMATA_M, u"fm" },
        { HARM_M, u"hr" },
        { REPEATMARK_M, u"rp" },
        { SLUR_M, u"sl" },
        { TEMPO_M, u"tp" },
        { TIE_M, u"ti" },
        { UNSPECIFIED_M, u"z" }
    };

    /** map of abbreviations for element within layers */
    inline static std::map<layerElementCounter, String> s_layerXmlIdMap = {
        { ACCID_L, u"a" },
        { BEAM_L, u"b" },
        { CHORD_L, u"c" },
        { GRACEGRP_L, u"g" },
        { NOTE_L, u"n" },
        { REST_L, u"r" },
        { TUPLET_L, u"t" },
        { UNSPECIFIED_L, u"z" }
    };
};
} // namespace

#endif // MU_IMPORTEXPORT_MEIEXPORTER_H
