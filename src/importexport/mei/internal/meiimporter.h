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

#pragma once

#include "engraving/types/types.h"

#include "modularity/ioc.h"
#include "imeiconfiguration.h"
#include "io/ifilesystem.h"
#include "io/path.h"

#include "meiconverter.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/element.h"
#include "thirdparty/libmei/shared.h"

#include "pugixml.hpp"

namespace mu::engraving {
class Chord;
class ChordRest;
class EngravingItem;
class Lyrics;
class Measure;
class Note;
class Part;
class Score;
class Spanner;
class Tuplet;
class VBox;
enum class NoteType : unsigned char;
enum class TimeSigType : unsigned char;
struct ClefTypeList;
}

namespace mu::iex::mei {
class UIDRegister;

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
    INJECT(muse::io::IFileSystem, fileSystem)

public:
    MeiImporter(engraving::Score* s) { m_score = s; }
    bool read(const muse::io::path_t& path);

private:
    /**
     * Methods for parsing the MEI tree and converting data
     */
    bool readMeiHead(pugi::xml_node root);
    bool readScore(pugi::xml_node root);
    bool readScoreDef(pugi::xml_node scoreDefNode, bool isInitial);
    bool readPgHead(pugi::xml_node pgHeadNode);
    bool readLines(pugi::xml_node parentNode, muse::StringList& lines, size_t& line);
    bool readLinesWithSmufl(pugi::xml_node parentNode, muse::StringList& lines);
    bool readStaffDefs(pugi::xml_node parentNode);
    bool readStaffGrps(pugi::xml_node parentNode, int& staffSpan, int column, size_t& idx);
    bool readInstrDef(pugi::xml_node instrDefNode, engraving::Part* part);
    bool readSectionElements(pugi::xml_node parentNode);
    bool readEnding(pugi::xml_node endingNode);
    bool readMeasure(pugi::xml_node measureNode);
    bool readPb(pugi::xml_node pbNode);
    bool readSb(pugi::xml_node sbNode);
    bool readStaves(pugi::xml_node parentNode, engraving::Measure* measure, engraving::Fraction& measureTicks);
    bool readLayers(pugi::xml_node parentNode, engraving::Measure* measure, int staffIdx, engraving::Fraction& measureTicks);

    /**
     * Methods for parsing MEI elements within a <layer>
     */
    bool readElements(pugi::xml_node parentNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readArtics(pugi::xml_node parentNode, engraving::Chord* chord);
    bool readArtic(pugi::xml_node articNode, engraving::Chord* chord);
    bool readBeam(pugi::xml_node beamNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readBTrem(pugi::xml_node bTremNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readClef(pugi::xml_node clefNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readChord(pugi::xml_node chordNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readGraceGrp(pugi::xml_node graceGrpNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readMRest(pugi::xml_node mRestNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readMRpt(pugi::xml_node mRptNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readNote(pugi::xml_node noteNode, engraving::Measure* measure, int track, engraving::Fraction& ticks,
                  engraving::Chord* chord = nullptr);
    bool readRest(pugi::xml_node restNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readSpace(pugi::xml_node spaceNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readSyl(pugi::xml_node sylNode, engraving::Lyrics* lyrics, Convert::textWithSmufl& textBlocks, ElisionType elision);
    bool readTuplet(pugi::xml_node tupletNode, engraving::Measure* measure, int track, engraving::Fraction& ticks);
    bool readVerses(pugi::xml_node parentNode, engraving::Chord* chord);
    bool readVerse(pugi::xml_node verseNode, engraving::Chord* chord);

    /**
     * Methods for parsing MEI control events (within <measure>)
     */
    bool readControlEvents(pugi::xml_node parentNode, engraving::Measure* measure);
    bool readArpeg(pugi::xml_node arpegNode, engraving::Measure* measure);
    bool readBreath(pugi::xml_node breathNode, engraving::Measure* measure);
    bool readCaesura(pugi::xml_node caesuraNode, engraving::Measure* measure);
    bool readDir(pugi::xml_node dirNote, engraving::Measure* measure);
    bool readDynam(pugi::xml_node dynamNode, engraving::Measure* measure);
    bool readF(pugi::xml_node fNode, engraving::FiguredBass* figuredBass);
    bool readFb(pugi::xml_node harmNode, engraving::Measure* measure);
    bool readFermata(pugi::xml_node fermataNode, engraving::Measure* measure);
    bool readFing(pugi::xml_node fingNode, engraving::Measure* measure);
    bool readGliss(pugi::xml_node glissNode, engraving::Measure* measure);
    bool readHairpin(pugi::xml_node hairpinNode, engraving::Measure* measure);
    bool readHarm(pugi::xml_node harmNode, engraving::Measure* measure);
    bool readHarpPedal(pugi::xml_node harpPedalNode, engraving::Measure* measure);
    bool readLv(pugi::xml_node lvNode, engraving::Measure* measure);
    bool readMordent(pugi::xml_node mordentNode, engraving::Measure* measure);
    bool readOctave(pugi::xml_node octaveNode, engraving::Measure* measure);
    bool readOrnam(pugi::xml_node ornamNode, engraving::Measure* measure);
    bool readPedal(pugi::xml_node pedalNode, engraving::Measure* measure);
    bool readReh(pugi::xml_node rehNode, engraving::Measure* measure);
    bool readRepeatMark(pugi::xml_node repeatMarkNode, engraving::Measure* measure);
    bool readSlur(pugi::xml_node slurNode, engraving::Measure* measure);
    bool readTempo(pugi::xml_node tempoNode, engraving::Measure* measure);
    bool readTie(pugi::xml_node tieNode, engraving::Measure* measure);
    bool readTrill(pugi::xml_node trillNode, engraving::Measure* measure);
    bool readTurn(pugi::xml_node turnNode, engraving::Measure* measure);

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
    void addTextToTitleFrame(engraving::VBox*& vBox, const muse::String& str, engraving::TextStyleType textStyleType);
    void addSpannerEnds();

    /**
     * Helper methods
     */
    int getStaffIndex(int staffN);
    int getVoiceIndex(int staffIdx, int layerN);
    void addLog(const std::string& msg, pugi::xml_node node);
    bool isNode(pugi::xml_node node, const muse::String& name);
    engraving::ChordRest* addChordRest(pugi::xml_node node, engraving::Measure* measure, int track, const libmei::Element& meiElement,
                                       engraving::Fraction& ticks, bool isRest);
    bool addGraceNotesToChord(engraving::ChordRest* chordRest, bool isAfter = false);
    engraving::EngravingItem* addAnnotation(const libmei::Element& meiElement, engraving::Measure* measure);
    engraving::Spanner* addSpanner(const libmei::Element& meiElement, engraving::Measure* measure, pugi::xml_node node);
    engraving::EngravingItem* addToChordRest(const libmei::Element& meiElement, engraving::Measure* measure,
                                             engraving::Chord* chord = nullptr);
    std::string xmlIdFrom(std::string dataURI);
    engraving::ChordRest* findStart(const libmei::Element& meiElement, engraving::Measure* measure);
    engraving::ChordRest* findEnd(pugi::xml_node controlNode, const engraving::ChordRest* startChordRest);
    engraving::Note* findStartNote(const libmei::Element& meiElement);
    engraving::Note* findEndNote(pugi::xml_node controlNode);
    const std::list<engraving::ChordRest*> findPlistChordRests(pugi::xml_node controlNode);
    void clearGraceNotes();
    bool hasLyricsToExtend(engraving::track_idx_t track, int no);
    const std::pair<engraving::Lyrics*, engraving::ChordRest*>& getLyricsToExtend(engraving::track_idx_t track, int no);
    void addChordtoLyricsToExtend(engraving::ChordRest* chordRest);
    void extendLyrics(const std::pair<engraving::Lyrics*, engraving::ChordRest*>& lyricsToExtend);
    void extendLyrics();
    void setOrnamentAccid(engraving::Ornament* ornament, const Convert::OrnamStruct& ornamSt);

    /** Read the xmlId and process it appropriately */
    void readXmlId(engraving::EngravingItem* item, const std::string& meiUID);

    /** The Score pointer */
    engraving::Score* m_score = nullptr;

    /** The uid register */
    UIDRegister* m_uids;

    /** A flag indicating the file has MuseScore EIDs as xml:ids */
    bool m_hasMuseScoreIds;

    engraving::Fraction m_ticks;
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
    /* A map for plist values and corresponding engraving::ChordRest */
    std::map<std::string, engraving::ChordRest*> m_plistValueChordRests;
    /* A map for open spanners that needs to be ended */
    std::map<engraving::Spanner*, pugi::xml_node> m_openSpannerMap;
    /* A map for open arpeg that needs to be spanned */
    std::map<engraving::Arpeggio*, pugi::xml_node> m_openArpegMap;

    /** A map of a map for lyrics with extender that needs to be extended */
    std::map<engraving::track_idx_t, std::map<int, std::pair<engraving::Lyrics*, engraving::ChordRest*> > > m_lyricExtenders;

    engraving::Tuplet* m_tuplet;
    engraving::BeamMode m_beamBeginMode;
    engraving::BeamMode m_graceBeamBeginMode;
    engraving::Chord* m_lastChord;
    std::string m_tremoloId;

    std::list<engraving::Chord*> m_graceNotes;
    GraceReading m_readingGraceNotes;
    engraving::NoteType m_graceNoteType;

    bool m_readingEnding;
    engraving::Measure* m_endingStart;
    engraving::Measure* m_endingEnd;
};
} // namespace
