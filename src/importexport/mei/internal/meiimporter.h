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

#ifndef MU_IMPORTEXPORT_MEIIMPORTER_H
#define MU_IMPORTEXPORT_MEIIMPORTER_H

#include "engraving/types/fraction.h"

#include "modularity/ioc.h"
#include "imeiconfiguration.h"
#include "io/ifilesystem.h"
#include "io/path.h"

#include "thirdparty/pugixml.hpp"

namespace mu::engraving {
class Fraction;
class Measure;
class Score;
}

namespace mu::iex::mei {
enum GraceReading {
    GraceNone = 0,
    GraceAsGrp,
    GraceAsNote
};

/**
 * Class of importing Music Encoding Initiative (MEI) files
 */

class MeiImporter
{
    INJECT_STATIC(mu::iex::mei::IMeiConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)

public:
    MeiImporter(engraving::Score* s) { m_score = s; }
    bool read(const io::path_t& path);
    void convert();

private:
    /**
     * Methods for parsing the MEI tree and converting data
     */
    bool readMeiHead(pugi::xml_node root);
    bool readScore(pugi::xml_node root);
    bool readScoreDef(pugi::xml_node scoreDefNode, bool isInitial);
    bool readPgHead(pugi::xml_node pgHeadNode);
    bool readLines(pugi::xml_node parentNode, StringList& lines);
    bool readLinesWithSmufl(pugi::xml_node parentNode, StringList& lines);
    bool readStaffDefs(pugi::xml_node parentNode);
    bool readStaffGrps(pugi::xml_node parentNode, int& staffSpan, int column, size_t& idx);
    bool readSectionElements(pugi::xml_node parentNode);
    bool readEnding(pugi::xml_node endingNode);
    bool readMeasure(pugi::xml_node measureNode);
    bool readPb(pugi::xml_node pbNode);
    bool readSb(pugi::xml_node sbNode);
    bool readStaves(pugi::xml_node parentNode, engraving::Measure* measure, int& measureTicks);
    bool readLayers(pugi::xml_node parentNode, engraving::Measure* measure, int staffIdx, int& measureTicks);

    /**
     * Methods for parsing MEI elements within a <layer>
     */
    bool readElements(pugi::xml_node parentNode, engraving::Measure* measure, int track, int& ticks);
    bool readBeam(pugi::xml_node beamNode, engraving::Measure* measure, int track, int& ticks);
    bool readClef(pugi::xml_node clefNode, engraving::Measure* measure, int track, int& ticks);
    bool readChord(pugi::xml_node chordNode, engraving::Measure* measure, int track, int& ticks);
    bool readGraceGrp(pugi::xml_node graceGrpNode, engraving::Measure* measure, int track, int& ticks);
    bool readMRest(pugi::xml_node mRestNode, engraving::Measure* measure, int track, int& ticks);
    bool readNote(pugi::xml_node noteNode, engraving::Measure* measure, int track, int& ticks, engraving::Chord* chord = nullptr);
    bool readRest(pugi::xml_node restNode, engraving::Measure* measure, int track, int& ticks);
    bool readSpace(pugi::xml_node spaceNode, engraving::Measure* measure, int track, int& ticks);
    bool readTuplet(pugi::xml_node tupletNode, engraving::Measure* measure, int track, int& ticks);

    /**
     * Methods for parsing MEI control events (within <measure>)
     */
    bool readControlEvents(pugi::xml_node parentNode, engraving::Measure* measure);
    bool readBreath(pugi::xml_node breathNode, engraving::Measure* measure);
    bool readCaesura(pugi::xml_node caesuraNode, engraving::Measure* measure);
    bool readDir(pugi::xml_node dirNote, engraving::Measure* measure);
    bool readDynam(pugi::xml_node dynamNode, engraving::Measure* measure);
    bool readFermata(pugi::xml_node fermataNode, engraving::Measure* measure);
    bool readHairpin(pugi::xml_node hairpinNode, engraving::Measure* measure);
    bool readHarm(pugi::xml_node harmNode, engraving::Measure* measure);
    bool readOctave(pugi::xml_node octaveNode, engraving::Measure* measure);
    bool readRepeatMark(pugi::xml_node repeatMarkNode, engraving::Measure* measure);
    bool readSlur(pugi::xml_node slurNode, engraving::Measure* measure);
    bool readTempo(pugi::xml_node tempoNode, engraving::Measure* measure);
    bool readTie(pugi::xml_node tieNode, engraving::Measure* measure);

    /**
     * Methods for parsing specific MEI attribute classes within elements
     */
    bool readGracedAtt(libmei::AttGraced& gracedAtt);
    bool readStemsAtt(engraving::Chord* chord, libmei::AttStems& stemsAtt);

    /**
     * Methods for extracting content from the MEI
     */
    bool buildIdMap(pugi::xml_node scoreNode);
    bool buildStaffLayerMap(pugi::xml_node root);
    bool buildTextFrame();
    bool buildScoreParts(pugi::xml_node scoreDefNode);

    /**
     * Helper methods for content creation
     */
    void addEndBarLineToMeasure(engraving::Measure* measure, engraving::BarLineType barLineType);
    void addLayoutBreakToMeasure(engraving::Measure* measure, engraving::LayoutBreakType layoutBreakType);
    void addTextToTitleFrame(VBox*& vBox, const String& str, TextStyleType textStyleType);
    void addSpannerEnds();

    /**
     * Helper methods
     */
    int getStaffIndex(int staffN);
    int getVoiceIndex(int staffIdx, int layerN);
    void addLog(const std::string& msg, pugi::xml_node node);
    bool isNode(pugi::xml_node node, const String& name);
    engraving::ChordRest* addChordRest(pugi::xml_node node, Measure* measure, int track, const libmei::Element& meiElement, int& ticks,
                                       bool isRest);
    bool addGraceNotesToChord(engraving::ChordRest* chordRest, bool isAfter = false);
    engraving::EngravingItem* addAnnotation(const libmei::Element& meiElement, Measure* measure);
    engraving::Spanner* addSpanner(const libmei::Element& meiElement, Measure* measure, pugi::xml_node node);
    std::string xmlIdFrom(std::string dataURI);
    engraving::ChordRest* findStart(const libmei::Element& meiElement, engraving::Measure* measure);
    engraving::ChordRest* findEnd(pugi::xml_node controlNode, const engraving::ChordRest* startChordRest);
    engraving::Note* findStartNote(const libmei::Element& meiElement);
    engraving::Note* findEndNote(pugi::xml_node controlNode);
    void clearGraceNotes();

    /** The Score pointer */
    engraving::Score* m_score = nullptr;

    Fraction m_ticks;
    int m_lastMeasureN;
    engraving::Measure* m_lastMeasure;

    /** map of MuseScore indexes and clef, timesig and keysig */
    std::map<int, engraving::ClefTypeList> m_clefs;
    std::map<int, std::pair<engraving::Fraction, engraving::TimeSigType> > m_timeSigs;
    std::map<int, engraving::Key> m_keySigs;
    /** the current time signature */
    engraving::Fraction m_currentTimeSig = engraving::Fraction(4, 4);

    /* Set for mapping between MEI staff@n and MuseScore staff indexes */
    std::set<int> m_staffNs;
    /* A map of sets for mapping between MEI layer@ in MuseScore voice indexes */
    std::map<int, std::set<int> > m_staffLayerNs;
    /* A map for original staffN and MuseScore parts */
    std::map<int, engraving::Part*> m_staffParts;

    /* A map for startId and corresponding engraving::ChordRest */
    std::map<std::string, engraving::ChordRest*> m_startIdChordRests;
    /* A map for endId and corresponding engraving::ChordRest */
    std::map<std::string, engraving::ChordRest*> m_endIdChordRests;
    /* A map for startId and corresponding engraving::Note */
    std::map<std::string, engraving::Note*> m_startIdNotes;
    /* A map for endId and corresponding engraving::Note */
    std::map<std::string, engraving::Note*> m_endIdNotes;
    /* A map for open spanners that needs to be ended */
    std::map<engraving::Spanner*, pugi::xml_node> m_openSpannerMap;

    engraving::Tuplet* m_tuplet;
    engraving::BeamMode m_beamBeginMode;
    engraving::BeamMode m_graceBeamBeginMode;
    engraving::Chord* m_lastChord;

    std::list<engraving::Chord*> m_graceNotes;
    GraceReading m_readingGraceNotes;
    engraving::NoteType m_graceNoteType;

    bool m_readingEnding;
    engraving::Measure* m_endingStart;
    engraving::Measure* m_endingEnd;
};
} // namespace

#endif // MU_IMPORTEXPORT_MEIIMPORTER_H
