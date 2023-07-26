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

#include "meiimporter.h"

#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"

#include "meiconverter.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/shared.h"

#include "thirdparty/pugixml.hpp"

using namespace mu;
using namespace mu::iex::mei;
using namespace mu::engraving;

#define SCOREDEF_IDX -1

#define MEI_BASIC_VERSION "5.0.0-dev+basic"

/**
 * Read the Score from the file.
 * Return false on error.
 */

bool MeiImporter::read(const io::path_t& path)
{
    m_lastMeasure = nullptr;
    m_tuplet = nullptr;
    m_beamBeginMode = BeamMode::AUTO;
    m_graceBeamBeginMode = BeamMode::AUTO;

    m_readingGraceNotes = GraceNone;
    m_lastChord = nullptr;

    m_readingEnding = false;
    m_endingStart = nullptr;
    m_endingEnd = nullptr;

    Convert::logs.clear();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(
        path.toStdString().c_str(), (pugi::parse_comments | pugi::parse_default) & ~pugi::parse_eol);

    if (!result) {
        LOGD() << "Cannot open file <" << qPrintable(path.toString()) << ">";
        return false;
    }

    pugi::xml_node root = doc.first_child();

    pugi::xml_attribute meiVersion = root.attribute("meiversion");
    if (!meiVersion || String(meiVersion.value()) != String(MEI_BASIC_VERSION)) {
        Convert::logs.push_back(String("The MEI file does not seem to be a MEI basic version '%1' file").arg(String(MEI_BASIC_VERSION)));
    }

    bool success = true;

    success = success && this->readMeiHead(root);

    success = success && this->readScore(root);

    return success;
}

//---------------------------------------------------------
// helper methods
//---------------------------------------------------------

/**
 * Add a message together with a printout of the node to the log list.
 */

void MeiImporter::addLog(const std::string& msg, pugi::xml_node node)
{
    String nodeStr = u"&lt;";
    nodeStr += String(node.name());
    for (auto attribute : node.attributes()) {
        nodeStr += String(" %1='%2'").arg(String(attribute.name()), String(attribute.value()));
    }
    nodeStr += u" /&gt;";
    Convert::logs.push_back(String("Could not convert the %1 from %2").arg(String::fromStdString(msg), nodeStr));
}

/**
 * Return true if the node name matches the name parameter.
 */

bool MeiImporter::isNode(pugi::xml_node node, const String& name)
{
    if (!node) {
        return false;
    }

    String nodeName = String(node.name());
    return nodeName == name;
}

/**
 * Return the MuseScore staff index for the corresponding MEI staff `@n`
 * Actually useful only when reading files not written by MuseScore and that can have not continuous staff numbers
 */

int MeiImporter::getStaffIndex(int staffN)
{
    if (m_staffNs.find(staffN) == m_staffNs.end()) {
        return 0;
    }
    return static_cast<int>(std::distance(m_staffNs.begin(), m_staffNs.find(staffN)));
}

/**
 * Return the voice index (0 to 3) for the corresponding MEI layer `@n`
 * Actually useful only when reading files not written by MuseScore
 */

int MeiImporter::getVoiceIndex(int staffIdx, int layerN)
{
    if (!m_staffLayerNs.count(staffIdx)) {
        return 0;
    }
    const std::set<int>& layerNs = m_staffLayerNs.at(staffIdx);
    if (layerNs.find(layerN) == layerNs.end()) {
        return 0;
    }
    return static_cast<int>(std::distance(layerNs.begin(), layerNs.find(layerN)));
}

/**
 * Add a new ChordRest object to a measure and called when reading a <chord>, <note> or <rest>, including grace notes.
 * When reading grace notes, add the object to the m_graceNotes list, which will eventually be added to the appropriate Chord.
 * When reading tuplet, increase the ticks value to the corrected ratio, and not at all when reading grace notes.
 */

ChordRest* MeiImporter::addChordRest(pugi::xml_node node, Measure* measure, int track, const libmei::Element& meiElement, int& ticks,
                                     bool isRest)
{
    IF_ASSERT_FAILED(measure) {
        return nullptr;
    }

    bool warning = false;

    const libmei::AttAugmentDots* augmentDotsAtt = dynamic_cast<const libmei::AttAugmentDots*>(&meiElement);
    const libmei::AttDurationLog* durationLogAtt = dynamic_cast<const libmei::AttDurationLog*>(&meiElement);
    const libmei::AttStaffIdent* staffIdentAtt = dynamic_cast<const libmei::AttStaffIdent*>(&meiElement);
    const libmei::AttTyped* typedAtt = dynamic_cast<const libmei::AttTyped*>(&meiElement);

    IF_ASSERT_FAILED(augmentDotsAtt && durationLogAtt && staffIdentAtt && typedAtt) {
        return nullptr;
    }

    // No assert because att.beam.secondary is not available on MEI `<space>`
    const libmei::AttBeamSecondary* beamSecondaryAtt = dynamic_cast<const libmei::AttBeamSecondary*>(&meiElement);

    TDuration duration;
    duration.setType(Convert::durFromMEI(durationLogAtt->GetDur(), warning));
    if (warning) {
        this->addLog("duration", node);
    }

    if (augmentDotsAtt->HasDots()) {
        duration.setDots(augmentDotsAtt->GetDots());
    }

    Segment* segment = nullptr;
    // For grace notes we use a dummy segment, otherwise a ChordRest segement with the appropriate ticks value
    if (m_readingGraceNotes) {
        segment = m_score->dummy()->segment();
    } else {
        segment = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(ticks) + measure->tick());
    }

    ChordRest* chordRest = nullptr;
    if (isRest) {
        chordRest = Factory::createRest(segment);
    } else {
        chordRest = Factory::createChord(segment);
    }

    if (m_startIdChordRests.count(meiElement.m_xmlId)) {
        m_startIdChordRests[meiElement.m_xmlId] = chordRest;
    }
    if (m_endIdChordRests.count(meiElement.m_xmlId)) {
        m_endIdChordRests[meiElement.m_xmlId] = chordRest;
    }

    // Handle cross-staff notation
    if (staffIdentAtt->HasStaff()) {
        int staffN = staffIdentAtt->GetStaff().front();
        int staffMoveIdx = this->getStaffIndex(staffN);
        int staffMove = (staffMoveIdx - static_cast<int>(track2staff(track)));
        // Limit staff move to one staff - also, boundaries should be OK since we use getStaffIndex and track2staff previously
        if (staffMove == -1 || staffMove == 1) {
            chordRest->setStaffMove(staffMove);
        }
    }

    chordRest->setTrack(track);
    chordRest->setTicks(duration.fraction());
    chordRest->setDurationType(TDuration(duration));

    if (m_readingGraceNotes) {
        // non critical assert
        assert(chordRest->isChord());
        m_graceNotes.push_front(toChord(chordRest));
    } else {
        if (m_graceNotes.size() > 0) {
            // pre grace groups have to be added to the chord - if it is a rest they will be ignored (deleted)
            this->addGraceNotesToChord(chordRest);
        }
        segment->add(chordRest);
        // Keep a pointer to the last chord read for adding post grace groups
        if (chordRest->isChord()) {
            m_lastChord = static_cast<Chord*>(chordRest);
        }
    }

    BeamMode beamMode = Convert::beamFromMEI(typedAtt->GetType(), BEAM_ELEMENT_TYPE, warning);
    if (beamMode != BeamMode::AUTO) {
        chordRest->setBeamMode(beamMode);
    }
    // Set BeamMode::BEGIN16 or BeamMode::BEGIN32 (we have a distinct flag for grace notes for handling nested beams)
    else {
        if (!m_readingGraceNotes) {
            chordRest->setBeamMode(m_beamBeginMode);
            m_beamBeginMode = BeamMode::AUTO;
        } else {
            chordRest->setBeamMode(m_graceBeamBeginMode);
            m_graceBeamBeginMode = BeamMode::AUTO;
        }
    }

    Fraction chordTicks = chordRest->ticks();
    // For tuplet add the ChordRest to it and adjust the tick advance to the ratio
    if (m_tuplet) {
        m_tuplet->add(chordRest);
        chordRest->setTuplet(m_tuplet);
        // For tuplets the tick advance is adjusted to the ratio
        chordTicks /= m_tuplet->ratio();
    }
    // For grace notes, no tick advance
    if (!m_readingGraceNotes) {
        ticks += chordTicks.ticks();
        // Check if we have an AttBeamSecondary and read it for the next ChordRest
        if (beamSecondaryAtt) {
            m_beamBeginMode = Convert::breaksecFromMEI(beamSecondaryAtt->GetBreaksec(), warning);
        }
    } else {
        // Check if we have an AttBeamSecondary and read it for the next ChordRest
        if (beamSecondaryAtt) {
            m_graceBeamBeginMode = Convert::breaksecFromMEI(beamSecondaryAtt->GetBreaksec(), warning);
        }
    }

    return chordRest;
}

/**
 * Add grace notes to a ChordRest When a grace group was previously read and added to MeiImpoter::m_graceNotes
 * Ignore (delete) the grace notes if the ChordRest is a Rest.
 * Look at m_graceNoteType for setting the acciaccatura note type (when grace notes preceed only)
 */

bool MeiImporter::addGraceNotesToChord(engraving::ChordRest* chordRest, bool isAfter)
{
    IF_ASSERT_FAILED(chordRest) {
        return false;
    }

    if (!chordRest->isChord()) {
        this->clearGraceNotes();
        return false;
    } else {
        if (isAfter) {
            NoteType noteType = NoteType::GRACE8_AFTER;
            // For after grace notes, the order of insertion is flipped
            m_graceNotes.reverse();
            for (auto graceChord : m_graceNotes) {
                switch (graceChord->durationType().type()) {
                case (DurationType::V_EIGHTH): noteType = NoteType::GRACE8_AFTER;
                    break;
                case (DurationType::V_16TH): noteType = NoteType::GRACE16_AFTER;
                    break;
                case (DurationType::V_32ND): noteType = NoteType::GRACE32_AFTER;
                    break;
                default:
                    LOGD() << "MeiImporter::addGraceNotesToChord unsupported grace duration type " <<
                        int(graceChord->durationType().type());
                    break;
                }
                graceChord->setNoteType(noteType);
                chordRest->add(graceChord);
            }
        } else {
            for (auto graceChord : m_graceNotes) {
                NoteType noteType = NoteType::APPOGGIATURA;
                if (m_graceNoteType == NoteType::ACCIACCATURA) {
                    noteType = NoteType::ACCIACCATURA;
                } else {
                    switch (graceChord->durationType().type()) {
                    case (DurationType::V_QUARTER): noteType = NoteType::GRACE4;
                        break;
                    case (DurationType::V_EIGHTH): noteType = NoteType::APPOGGIATURA;
                        break;
                    case (DurationType::V_16TH): noteType = NoteType::GRACE16;
                        break;
                    case (DurationType::V_32ND): noteType = NoteType::GRACE32;
                        break;
                    default:
                        LOGD("MeiImporter::addGraceNotesToChord unsupported grace duration type %d",
                             int(graceChord->durationType().type()));
                        break;
                    }
                }
                graceChord->setNoteType(noteType);
                chordRest->add(graceChord);
            }
        }
        m_graceNotes.clear();
        return true;
    }
}

/**
 * Create a annotation (MEI control event with @startid).
 * Create the EngravingItem according to the MEI element name (e.g., "dynam", "fermata")
 * Return nullptr if the lookup fails.
 */

EngravingItem* MeiImporter::addAnnotation(const libmei::Element& meiElement, Measure* measure)
{
    const ChordRest* chordRest = this->findStart(meiElement, measure);
    if (!chordRest) {
        return nullptr;
    }

    Segment* segment = chordRest->segment();
    EngravingItem* item = nullptr;

    if (meiElement.m_name == "breath" || meiElement.m_name == "caesura") {
        // For Breath we need to add a specific segment and add the breath to it (and not to the ChordRest one)
        segment = measure->getSegment(SegmentType::Breath, segment->tick() + chordRest->actualTicks());
        item = Factory::createBreath(segment);
    } else if (meiElement.m_name == "dynam") {
        item = Factory::createDynamic(chordRest->segment());
    } else if (meiElement.m_name == "fermata") {
        item = Factory::createFermata(chordRest->segment());
    } else if (meiElement.m_name == "harm") {
        item = Factory::createHarmony(chordRest->segment());
    } else if (meiElement.m_name == "tempo") {
        item = Factory::createTempoText(chordRest->segment());
    }

    item->setTrack(chordRest->track());
    segment->add(item);

    return item;
}

/**
 * Look for the ChordRest for an MEI element with @startid.
 * Do a lookup in the m_startIdChordRests map with the @startid value to retrieve the ChordRest to which the annotation points to.
 * If there is not @startid but a @tstamp (MEI not written by MuseScore), try to find the corresponding ChordRest
 */

ChordRest* MeiImporter::findStart(const libmei::Element& meiElement, Measure* measure)
{
    const libmei::AttStartId* startIdAtt = dynamic_cast<const libmei::AttStartId*>(&meiElement);
    IF_ASSERT_FAILED(measure && startIdAtt) {
        return nullptr;
    }

    ChordRest* chordRest = nullptr;
    if (startIdAtt->HasStartid()) {
        std::string startId = startIdAtt->GetStartid();
        // Basic check for the @stardid validity
        if (startId.size() < 1 || startId.at(0) != '#') {
            Convert::logs.push_back(String("Could not find element for @startid '%1'").arg(String::fromStdString(startIdAtt->GetStartid())));
            return nullptr;
        }
        startId.erase(0, 1);

        // The startid corresponding ChordRest should have been added to the m_startIdChordRests previously
        if (!m_startIdChordRests.count(startId) || !m_startIdChordRests.at(startId)) {
            Convert::logs.push_back(String("Could not find element for @startid '%1'").arg(String::fromStdString(startIdAtt->GetStartid())));
            return nullptr;
        }
        chordRest = m_startIdChordRests.at(startId);
    } else {
        // No @startid, try a lookup based on the @tstamp. This is only for files not written via MuseScore
        const libmei::AttTimestampLog* timestampLogAtt = dynamic_cast<const libmei::AttTimestampLog*>(&meiElement);
        const libmei::AttStaffIdent* staffIdentAtt = dynamic_cast<const libmei::AttStaffIdent*>(&meiElement);
        const libmei::AttLayerIdent* layerIdentAtt = dynamic_cast<const libmei::AttLayerIdent*>(&meiElement);

        IF_ASSERT_FAILED(timestampLogAtt && staffIdentAtt && layerIdentAtt) {
            return nullptr;
        }

        // If no @tstamp (invalid), put it on 1.0;
        double tstampValue = timestampLogAtt->HasTstamp() ? timestampLogAtt->GetTstamp() : 1.0;
        Fraction tstampFraction = Convert::tstampToFraction(tstampValue, measure->timesig());
        int staffIdx = (staffIdentAtt->HasStaff() && staffIdentAtt->GetStaff().size() > 0) ? this->getStaffIndex(
            staffIdentAtt->GetStaff().at(0)) : 0;
        int layer = (layerIdentAtt->HasLayer()) ? this->getVoiceIndex(staffIdx, layerIdentAtt->GetLayer()) : 0;

        chordRest = measure->findChordRest(measure->tick() + tstampFraction, staffIdx * VOICES + layer);
        if (!chordRest) {
            Convert::logs.push_back(String("Could not find element corresponding to @tstamp '%1'").arg(timestampLogAtt->GetTstamp()));
            return nullptr;
        }
    }

    return chordRest;
}

/**
 * Look for the ChordRest for an MEI element with @endid.
 * Do a lookup in the m_startIdChordRests map with the @endid value to retrieve the ChordRest to which the annotation points to.
 * If there is not @endid but a @tstamp2 (MEI not written by MuseScore), try to find the corresponding ChordRest
 */

ChordRest* MeiImporter::findEnd(pugi::xml_node controlNode, Measure* startMeasure)
{
    libmei::InstStartEndId startEndIdAtt;
    startEndIdAtt.ReadStartEndId(controlNode);

    ChordRest* chordRest = nullptr;
    if (startEndIdAtt.HasEndid()) {
        std::string endId = startEndIdAtt.GetEndid();
        // Basic check for the @stardid validity
        if (endId.size() < 1 || endId.at(0) != '#') {
            Convert::logs.push_back(String("Could not find element for @endid '%1'").arg(String::fromStdString(startEndIdAtt.GetEndid())));
            return nullptr;
        }
        endId.erase(0, 1);

        // The startid corresponding ChordRest should have been added to the m_startIdChordRests previously
        if (!m_endIdChordRests.count(endId) || !m_endIdChordRests.at(endId)) {
            Convert::logs.push_back(String("Could not find element for @endid '%1'").arg(String::fromStdString(startEndIdAtt.GetEndid())));
            return nullptr;
        }
        chordRest = m_endIdChordRests.at(endId);
    } else {
        /*
        // No @end, try a lookup based on the @tstamp2. This is only for files not written via MuseScore
        const libmei::AttTimestamp2Log* timestamp2LogAtt = dynamic_cast<const libmei::AttTimestamp2Log*>(&meiElement);
        const libmei::AttStaffIdent* staffIdentAtt = dynamic_cast<const libmei::AttStaffIdent*>(&meiElement);
        const libmei::AttLayerIdent* layerIdentAtt = dynamic_cast<const libmei::AttLayerIdent*>(&meiElement);

        IF_ASSERT_FAILED(timestamp2LogAtt && staffIdentAtt && layerIdentAtt) {
            return nullptr;
        }

        // If no @tstamp (invalid), put it on 1.0;
        double tstampValue = timestamp2LogAtt->HasTstamp2() ? timestamp2LogAtt->GetTstamp2() : 1.0;
        Fraction tstampFraction = Convert::tstampToFraction(tstampValue, measure->timesig());
        int staffIdx = (staffIdentAtt->HasStaff() && staffIdentAtt->GetStaff().size() > 0) ? this->getStaffIndex(
                                                                                                                 staffIdentAtt->GetStaff().at(0)) : 0;
        int layer = (layerIdentAtt->HasLayer()) ? this->getVoiceIndex(staffIdx, layerIdentAtt->GetLayer()) : 0;

        chordRest = measure->findChordRest(measure->tick() + tstampFraction, staffIdx * VOICES + layer);
        if (!chordRest) {
            Convert::logs.push_back(String("Could not find element corresponding to @tstamp '%1'").arg(timestampLogAtt->GetTstamp()));
            return nullptr;
        }
        */
    }

    return chordRest;
}

/**
 * Clear grace notes that cannot be properly attached to a Chord.
 * Actually delete  the grace notes (Chord) because these have no parent.
 */

void MeiImporter::clearGraceNotes()
{
    for (auto graceChord : m_graceNotes) {
        delete graceChord;
    }
    m_graceNotes.clear();
    m_readingGraceNotes = GraceNone;
}

//---------------------------------------------------------
// parsing methods
//---------------------------------------------------------

/**
 * Read the <meiHead> and stores it as a custom MuseScore metatag
 * Also
 */

bool MeiImporter::readMeiHead(pugi::xml_node root)
{
    pugi::xml_node headNode = root.select_node("//meiHead").node();
    // Store the MEI header in a custom meta tag

    // Critical error
    if (!headNode) {
        return false;
    }

    pugi::xml_document docHeader;
    docHeader.append_copy(headNode);
    unsigned int output_flags = pugi::format_default | pugi::format_no_declaration | pugi::format_raw;

    std::stringstream strStream;
    docHeader.save(strStream, "", output_flags);
    m_score->setMetaTag(u"meiHead", String::fromStdString(strStream.str()));

    pugi::xml_node workTitleNode = root.select_node("//meiHead/fileDesc/titleStmt/title").node();
    if (workTitleNode) {
        m_score->setMetaTag(u"workTitle", String(workTitleNode.text().as_string()));
    }

    StringList persNames;
    // the creator types commonly found in MusicXML
    persNames << u"arranger" << u"composer" << u"lyricist" << u"translator";
    for (String tagName : persNames) {
        String xpath = String("//meiHead/fileDesc/titleStmt/respStmt/persName[@role='%1']").arg(tagName);
        pugi::xml_node persNameNode = root.select_node(xpath.toStdString().c_str()).node();
        if (persNameNode) {
            m_score->setMetaTag(tagName, String(persNameNode.text().as_string()));
        }
    }

    pugi::xml_node copyrightNode = root.select_node("//meiHead/fileDesc/pubStmt/availability/distributor").node();
    if (copyrightNode) {
        m_score->setMetaTag(u"copyright", String(copyrightNode.text().as_string()));
    }

    return true;
}

/**
 * Read the MEI score.
 * Previously builds a map of IDs being referred to (e.g., through `@startid` or `@endid`)
 * Also builds a map for staff@n and layer@n when reading MEI files not produced with MuseScore.
 * Reads the intitial scoreDef before reading the section elements.
 */

bool MeiImporter::readScore(pugi::xml_node root)
{
    pugi::xml_node scoreNode = root.select_node("./music//score").node();

    if (!scoreNode) {
        return false;
    }

    pugi::xml_node scoreDefNode = scoreNode.select_node("./scoreDef").node();

    bool success = true;

    success = success && this->buildIdMap(scoreNode);

    success = success && this->buildStaffLayerMap(scoreNode);

    success = success && this->readScoreDef(scoreDefNode, true);

    m_ticks = Fraction(0, 1);
    m_lastMeasureN = 0;
    m_lastMeasure = nullptr;

    pugi::xpath_node_set sections = scoreNode.select_nodes("./section");
    for (pugi::xpath_node xpathNode : sections) {
        success = success && this->readSectionElements(xpathNode.node());
    }

    this->addSpannerEnds();

    return success;
}

/**
 * Read a scoreDef (initial or intermediate).
 * For the intial scoreDef, also tries to build the part structure from the scoreDef relying on staffGrp@label and staffDef@label.
 * Sets the time signature and key signature to the global m_timeSigs and m_keySigs maps.
 * Uses the SCOREDEF_IDX index position for global (scoreDef) time signature and key signatures.
 * Since the map are ordered, these will have priority over the ones read in MeiImporter::readStaffDef.
 */

bool MeiImporter::readScoreDef(pugi::xml_node scoreDefNode, bool isInitial)
{
    bool warning = false;
    libmei::ScoreDef meiScoreDef;

    // Critical error
    if (!scoreDefNode) {
        return false;
    }

    bool success = true;

    if (isInitial) {
        success = readPgHead(scoreDefNode.child("pgHead"));
    }

    meiScoreDef.Read(scoreDefNode);

    if (meiScoreDef.HasMeterSym() || meiScoreDef.HasMeterCount()) {
        m_timeSigs[SCOREDEF_IDX] = Convert::meterFromMEI(meiScoreDef, warning);
        if (warning) {
            this->addLog("meter signature", scoreDefNode);
        }
    }
    if (meiScoreDef.HasKeysig()) {
        m_keySigs[SCOREDEF_IDX] = Convert::keyFromMEI(meiScoreDef.GetKeysig(), warning);
        if (warning) {
            this->addLog("key signature", scoreDefNode);
        }
    }

    if (isInitial) {
        success = success && this->buildScoreParts(scoreDefNode);
    }

    success = success && this->readStaffDefs(scoreDefNode);

    // If we have a time signature, pick the first one, which will be SCOREDEF_IDX one (if given)
    // Otherwise, it picks the first staffDef one and we expect it to be the same for all staves
    if (!m_timeSigs.empty()) {
        m_currentTimeSig = m_timeSigs.begin()->second.first;
    }

    return success;
}

/**
 * Read the pgHead into a MuseScore initial vBox.
 */

bool MeiImporter::readPgHead(pugi::xml_node pgHeadNode)
{
    if (!pgHeadNode) {
        return this->buildTextFrame();
    }

    bool warning;

    VBox* vBox = nullptr;

    // first level MEI /rend are positioning <rend> elements, which are ignored since positioning is based on
    // second level MEI /rend @label based on text styles ("title", "subtitle", "composer", etc).
    pugi::xpath_node_set rends = pgHeadNode.select_nodes("./rend/rend");

    for (pugi::xpath_node rendXpathNode : rends) {
        pugi::xml_node rendNode = rendXpathNode.node();
        if (!rendNode) {
            continue;
        }
        libmei::Rend meiRend;
        meiRend.Read(rendNode);

        // Read the string content (first level), including <lb>
        StringList lines;
        this->readLines(rendNode, lines);

        TextStyleType textStyle = Convert::textFromMEI(meiRend, warning);
        if (warning) {
            this->addLog("text label", rendNode);
        }

        if (!vBox) {
            vBox = Factory::createVBox(m_score->dummy()->system());
        }

        Text* text = Factory::createText(vBox, textStyle);
        text->setPlainText(lines.join(u"\n"));
        vBox->add(text);
    }

    if (vBox) {
        m_score->measures()->add(vBox);
    }

    return true;
}

/**
 * Read the lines of a textual mixed content.
 */

bool MeiImporter::readLines(pugi::xml_node parentNode, StringList& lines)
{
    for (pugi::xml_node child : parentNode.children()) {
        if (child.type() == pugi::node_pcdata) {
            lines.push_back(String(child.text().as_string()));
        } else if (!this->isNode(child, u"lb")) {
            this->addLog("skipping child element", child);
        }
    }
    return true;
}

/**
 * Read the lines of a textual mixed content.
 * For each line, read <rend> with @glyph.auth as XML symbols.
 * Read the text content line by line and convert each line into MuseScore xmlText (with <sym>)
 * The method does not parse the content of <rend> recursively.
 */

bool MeiImporter::readLinesWithSmufl(pugi::xml_node parentNode, StringList& lines)
{
    // A list of segmented text or smufl text blocks
    Convert::textWithSmufl lineBlocks;
    String line;

    for (pugi::xml_node child : parentNode.children()) {
        // Plain text, add it as is to the text block list
        if (child.type() == pugi::node_pcdata) {
            lineBlocks.push_back(std::make_pair(false, String(child.text().as_string())));
        }
        // A rend, check is the content is smufl
        else if (this->isNode(child, u"rend")) {
            libmei::Rend meiRend;
            meiRend.Read(child);
            bool isSmufl = (meiRend.HasGlyphAuth() && meiRend.GetGlyphAuth() == "smufl");
            // If smufl, add its text as smufl, otherwise as text
            lineBlocks.push_back(std::make_pair(isSmufl, String(child.first_child().text().as_string())));
        }
        // A line break, already convert the current text blocks
        else if (this->isNode(child, u"lb")) {
            Convert::textFromMEI(line, lineBlocks);
            lines.push_back(line);
            lineBlocks.clear();
        } else {
            this->addLog("skipping child element", child);
        }
    }

    // At the end, convert the remaining text blocks
    Convert::textFromMEI(line, lineBlocks);
    lines.push_back(line);

    return true;
}

/**
 * Read the staff definition.
 * Stores clef, time signature and key signature into the corresponding global maps.
 */

bool MeiImporter::readStaffDefs(pugi::xml_node parentNode)
{
    bool warning = false;

    pugi::xpath_node_set staffDefs = parentNode.select_nodes(".//staffDef");
    // No critical error here if not staffDefs is empty because we can all this from an empty <scoreDef> change

    for (pugi::xpath_node staffDefXpathNode : staffDefs) {
        libmei::StaffDef meiStaffDef;
        meiStaffDef.Read(staffDefXpathNode.node());

        // Mapped index from MEI staff@n to MuseScore index
        const int staffIdx = this->getStaffIndex(meiStaffDef.GetN());

        pugi::xml_node clefNode = staffDefXpathNode.node().select_node(".//clef").node();
        if (clefNode) {
            libmei::Clef meiClef;
            meiClef.Read(clefNode);
            m_clefs[staffIdx] = Convert::clefFromMEI(meiClef, warning);
            if (warning) {
                this->addLog("clef", clefNode);
            }
            //staff->setDefaultClefType(ClefTypeList(Convert::clefFromMEI(meiClef)));
        } else if (meiStaffDef.HasClefShape() && meiStaffDef.HasClefLine()) {
            m_clefs[staffIdx] = Convert::clefFromMEI(meiStaffDef, warning);
            if (warning) {
                this->addLog("clef", staffDefXpathNode.node());
            }
            //staff->setDefaultClefType(ClefTypeList(Convert::clefFromMEI(meiStaffDef)));
        }
        //m_clefs[meiStaffDef.GetN()] = staff->defaultClefType();

        if (meiStaffDef.HasMeterSym() || meiStaffDef.HasMeterCount()) {
            m_timeSigs[staffIdx] = Convert::meterFromMEI(meiStaffDef, warning);
            if (warning) {
                this->addLog("meter signature", staffDefXpathNode.node());
            }
        }
        if (meiStaffDef.HasKeysig()) {
            m_keySigs[staffIdx] = Convert::keyFromMEI(meiStaffDef.GetKeysig(), warning);
            if (warning) {
                this->addLog("key signature", staffDefXpathNode.node());
            }
        }
    }

    return true;
}

/**
 * Read the staffGrp.
 */

bool MeiImporter::readStaffGrps(pugi::xml_node parentNode, int& staffSpan, int column, size_t& idx)
{
    pugi::xpath_node_set children = parentNode.select_nodes("./*");

    bool success = true;

    // Loop through all staffDef and staffGrp
    for (pugi::xpath_node child : children) {
        if (isNode(child.node(), u"staffDef")) {
            staffSpan++;
            idx++;
        } else if (isNode(child.node(), u"staffGrp")) {
            Staff* staff = m_score->staff(idx);
            libmei::StaffGrp meiStaffGrp;
            meiStaffGrp.Read(child.node());
            Convert::BracketStruct bracketSt = Convert::bracketFromMEI(meiStaffGrp);
            staff->setBracketType(column, bracketSt.bracketType);

            int childStaffSpan = 0;
            // Recursive call
            success = success && this->readStaffGrps(child.node(), childStaffSpan, column + 1, idx);

            // Now we know the spanning of the group
            staff->setBracketSpan(column, childStaffSpan);
            // We can also set the barline spanning - staff by staff since this is what MuseScore seems to do by default
            if (bracketSt.barLineSpan > 0) {
                size_t staffIdxStart = idx - static_cast<size_t>(childStaffSpan);
                size_t staffIdxEnd = idx - 1;
                for (size_t staffIdx = staffIdxStart; staffIdx < staffIdxEnd; staffIdx++) {
                    Staff* currentStaff = m_score->staff(staffIdx);
                    if (currentStaff) {
                        currentStaff->setBarLineSpan(1);
                    }
                }
            }

            staffSpan += childStaffSpan;
        }
    }

    return success;
}

/**
 * Read section content, including through a recursive call for nested section elements.
 */

bool MeiImporter::readSectionElements(pugi::xml_node parentNode)
{
    bool success = true;

    pugi::xpath_node_set elements = parentNode.select_nodes("./*");
    for (pugi::xpath_node xpathNode : elements) {
        std::string elementName = std::string(xpathNode.node().name());
        if (elementName == "section") {
            success = success && this->readSectionElements(xpathNode.node());
        } else if (elementName == "ending") {
            success = success && this->readEnding(xpathNode.node());
        } else if (elementName == "measure") {
            success = success && this->readMeasure(xpathNode.node());
        } else if (elementName == "pb") {
            success = success && this->readPb(xpathNode.node());
        } else if (elementName == "sb") {
            success = success && this->readSb(xpathNode.node());
        } else if (elementName == "scoreDef") {
            success = success && this->readScoreDef(xpathNode.node(), false);
        }
    }

    // Post-processing adjustment for the last barLine.
    if (m_score->measures()->last() && m_score->measures()->last()->isMeasure()) {
        Measure* measure = static_cast<Measure*>(m_score->measures()->last());
        if (!measure->endBarLine()) {
            this->addEndBarLineToMeasure(measure, BarLineType::NORMAL);
        }
    }

    return success;
}

/**
 * Read ending and its content.
 * Spanning of the ending is set using m_endingStart and m_endingEnd measures
 */

bool MeiImporter::readEnding(pugi::xml_node endingNode)
{
    bool success = true;

    bool warning;
    libmei::Ending meiEnding;
    meiEnding.Read(endingNode);

    m_readingEnding = true;

    success = success && this->readSectionElements(endingNode);

    if (!m_endingStart || !m_endingEnd) {
        success = false;
    } else {
        Volta* volta = Factory::createVolta(m_score->dummy());
        Convert::endingFromMEI(volta, meiEnding, warning);
        volta->setTrack(0);
        volta->setTrack2(0);
        volta->setTick(m_endingStart->tick());
        volta->setTick2(m_endingEnd->tick() + m_endingEnd->ticks());
        m_score->addElement(volta);
    }

    m_readingEnding = false;
    m_endingStart = nullptr;
    m_endingEnd = nullptr;

    return success;
}

/**
 * Read measure and its content.
 * Sets m_endingStart and m_endingEnd pointers as appropriate.
 * Try to manage measure offest looking at MEI measure@n (num-like values).
 * Ajust various flags (end barline, repeat counts).
 * Reads measure control events.
 */

bool MeiImporter::readMeasure(pugi::xml_node measureNode)
{
    bool success = true;

    bool warning;
    libmei::Measure meiMeasure;
    meiMeasure.Read(measureNode);
    Convert::MeasureStruct measureSt = Convert::measureFromMEI(meiMeasure, warning);

    Measure* measure = Factory::createMeasure(m_score->dummy()->system());
    measure->setTick(m_ticks);
    measure->setTimesig(m_currentTimeSig);

    if (m_readingEnding) {
        if (!m_endingStart) {
            m_endingStart = measure;
        }
        m_endingEnd = measure;
    }

    // Try to manage measure offsets
    if (measureSt.n) {
        m_lastMeasureN++;
        // The offset might be positive or negative
        // For example, two measure 9a and 9b, with the second with a -1 offset
        if (measureSt.n != m_lastMeasureN) {
            measure->setNoOffset(measureSt.n - m_lastMeasureN);
            m_lastMeasureN = measureSt.n;
        }
    }
    measure->setIrregular(measureSt.irregular);

    int measureTicks = 0;
    success = success & this->readStaves(measureNode, measure, measureTicks);
    // Make sure we correct empty content because this would crash MuseScore
    if (measureTicks == 0) {
        measureTicks = m_currentTimeSig.ticks();
        LOGD() << "MeiImporter::readMeasure empty content in " << meiMeasure.GetN();
    }
    measure->setTicks(Fraction::fromTicks(measureTicks));

    success = success & this->readControlEvents(measureNode, measure);

    measure->setRepeatStart(measureSt.repeatStart);
    if (measureSt.repeatEnd) {
        measure->setRepeatEnd(true);
    } else if (measureSt.endBarLineType != engraving::BarLineType::NORMAL) {
        this->addEndBarLineToMeasure(measure, measureSt.endBarLineType);
    }

    if (measureSt.repeatCount) {
        measure->setRepeatCount(measureSt.repeatCount);
    }

    m_score->measures()->add(measure);
    m_ticks += measure->ticks();

    m_lastMeasure = measure;

    return success;
}

/**
 * Read page begin.
 * Only pb with a BREAK_TYPE `@type` will be imported as user added breaks
 */

bool MeiImporter::readPb(pugi::xml_node pbNode)
{
    if (!configuration()->meiImportLayout()) {
        return true;
    }

    libmei::Pb meiPb;
    meiPb.Read(pbNode);

    if (meiPb.HasType() && Convert::hasTypeValue(meiPb.GetType(), BREAK_TYPE)) {
        this->addLayoutBreakToMeasure(m_lastMeasure, engraving::LayoutBreakType::PAGE);
    }

    return true;
}

/**
 * Read system begin.
 * Only sb with a BREAK_TYPE `@type` will be imported as user added breaks
 */

bool MeiImporter::readSb(pugi::xml_node pbNode)
{
    if (!configuration()->meiImportLayout()) {
        return true;
    }

    libmei::Sb meiSb;
    meiSb.Read(pbNode);

    if (meiSb.HasType() && Convert::hasTypeValue(meiSb.GetType(), BREAK_TYPE)) {
        this->addLayoutBreakToMeasure(m_lastMeasure, engraving::LayoutBreakType::LINE);
    }

    return true;
}

/**
 * Read all staves of a measure and their content.
 * Lookup (and clear) the time signature and key signature maps to add them if necessary.
 */

bool MeiImporter::readStaves(pugi::xml_node parentNode, Measure* measure, int& measureTicks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set staves = parentNode.select_nodes("./staff");
    // Critical error
    if (staves.empty()) {
        success = false;
    }

    for (pugi::xpath_node xpathNode : staves) {
        libmei::Staff meiStaff;
        meiStaff.Read(xpathNode.node());
        const int staffIdx = this->getStaffIndex(meiStaff.GetN());

        // Either the staffDef specific value (if any), the scoreDef one (if any) or nothing
        int timeSigIdx = m_timeSigs.count(staffIdx) ? staffIdx : m_timeSigs.count(SCOREDEF_IDX) ? SCOREDEF_IDX : MEI_UNSET;
        if (timeSigIdx != MEI_UNSET) {
            Segment* segment = measure->getSegment(SegmentType::TimeSig, Fraction::fromTicks(0) + measure->tick());
            TimeSig* timeSig = Factory::createTimeSig(segment);
            timeSig->setTrack(staffIdx * VOICES);
            timeSig->setSig(m_timeSigs.at(timeSigIdx).first, m_timeSigs.at(timeSigIdx).second);
            segment->add(timeSig);
            // Erase the staffDef specific values (add them only for this staff).
            m_timeSigs.erase(staffIdx);
        }

        int keySigIdx = m_keySigs.count(staffIdx) ? staffIdx : m_keySigs.count(SCOREDEF_IDX) ? SCOREDEF_IDX : MEI_UNSET;
        if (keySigIdx != MEI_UNSET) {
            Segment* segment = measure->getSegment(SegmentType::KeySig, Fraction::fromTicks(0) + measure->tick());
            KeySigEvent ksEvent;
            ksEvent.setKey(m_keySigs.at(keySigIdx));
            KeySig* keySig = Factory::createKeySig(segment);
            keySig->setTrack(staffIdx * VOICES);
            keySig->setKeySigEvent(ksEvent);
            segment->add(keySig);
            m_keySigs.erase(staffIdx);
        }

        if (m_clefs.count(staffIdx)) {
            Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction::fromTicks(0) + measure->tick());
            Clef* clef = Factory::createClef(segment);
            clef->setClefType(m_clefs.at(staffIdx));
            clef->setTrack(staffIdx * VOICES);
            segment->add(clef);
            m_clefs.erase(staffIdx);
        }

        success = success && this->readLayers(xpathNode.node(), measure, staffIdx, measureTicks);
    }

    // Erase the scoreDef values (add them only for this measure)
    m_timeSigs.erase(SCOREDEF_IDX);
    m_keySigs.erase(SCOREDEF_IDX);

    return success;
}

/**
 * Read the layer and its content.
 * Also read grace notes not within a graceGrp for MEI files not written by MuseScore.
 * Relies on the m_lastChord pointer for adding grace notes to the correct ChordRest
 */

bool MeiImporter::readLayers(pugi::xml_node parentNode, Measure* measure, int staffN, int& measureTicks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set layers = parentNode.select_nodes("./layer");
    // Critical error
    if (layers.empty()) {
        success = false;
    }

    size_t i = 0;
    for (pugi::xpath_node xpathNode : layers) {
        // We cannot have more than 4 voices in Musescore
        if (i >= VOICES) {
            Convert::logs.push_back(String("More than %1 layers in a staff in not supported. Their content will not be imported.").arg(
                                        VOICES));
            break;
        }
        libmei::Layer meiLayer;
        meiLayer.Read(xpathNode.node());

        m_lastChord = nullptr;
        int ticks = 0;
        int track = staffN * VOICES + static_cast<int>(i);
        success = success && this->readElements(xpathNode.node(), measure, track, ticks);
        measureTicks = std::max(measureTicks, ticks);
        i++;
        this->clearGraceNotes();
    }

    return success;
}

/**
 * Read the layer content, including through a recursive call for nested elements.
 */

bool MeiImporter::readElements(pugi::xml_node parentNode, Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set elements = parentNode.select_nodes("./*");
    for (pugi::xpath_node xpathNode : elements) {
        std::string elementName = std::string(xpathNode.node().name());
        if (elementName == "beam") {
            success = success && this->readBeam(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "chord") {
            success = success && this->readChord(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "clef" && !m_readingGraceNotes) {
            success = success && this->readClef(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "graceGrp" && !m_readingGraceNotes) {
            success = success && this->readGraceGrp(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "mRest" && !m_readingGraceNotes) {
            success = success && this->readMRest(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "note") {
            success = success && this->readNote(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "rest" && !m_readingGraceNotes) {
            success = success && this->readRest(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "tuplet" && !m_readingGraceNotes) {
            success = success && this->readTuplet(xpathNode.node(), measure, track, ticks);
        } else {
            success = success && this->readElements(xpathNode.node(), measure, track, ticks);
        }
    }
    return success;
}

/**
 * Read a beam.
 * Set MuseScore flags based on custom `@type` values.
 */

bool MeiImporter::readBeam(pugi::xml_node beamNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    libmei::Beam meiBeam;
    meiBeam.Read(beamNode);

    success = readElements(beamNode, measure, track, ticks);

    // Reset the beam BEGIN16 and BEGIN32 mode
    if (!m_readingGraceNotes) {
        m_beamBeginMode = BeamMode::AUTO;
    } else {
        m_graceBeamBeginMode = BeamMode::AUTO;
    }

    return success;
}

/**
 * Read a chord and its content (note elements).
 */

bool MeiImporter::readChord(pugi::xml_node chordNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Chord meiChord;
    meiChord.Read(chordNode);

    int chordTicks = ticks;

    // Support for @grace without <graceGrp>
    this->readGracedAtt(meiChord);
    Chord* chord = static_cast<Chord*>(addChordRest(chordNode, measure, track, meiChord, ticks, false));
    this->readStemsAtt(chord, meiChord);

    pugi::xpath_node_set notes = chordNode.select_nodes(".//note");
    for (pugi::xpath_node xpathNode : notes) {
        this->readNote(xpathNode.node(), measure, track, chordTicks, chord);
    }

    return true;
}

/*
 * Read a clef.
 */

bool MeiImporter::readClef(pugi::xml_node clefNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning = false;
    libmei::Clef meiClef;
    meiClef.Read(clefNode);

    Segment* segment = measure->getSegment(SegmentType::Clef, Fraction::fromTicks(ticks) + measure->tick());
    Clef* clef = Factory::createClef(segment);
    clef->setClefType(ClefTypeList(Convert::clefFromMEI(meiClef, warning)));
    if (warning) {
        this->addLog("clef", clefNode);
    }

    clef->setTrack(track);
    //clef->setGenerated(true);
    segment->add(clef);

    return true;
}

/**
 * Read a <graceGrp> and adjust the MeiImporter::m_readingGraceNotes flag.
 * For <graceGrp> with @attach="pre", add the grace notes read recursively (through readElements) to m_lastChord (if any)
 * Otherwise, only reset the flag and adding the grace notes will be performed in MeiImporter::addChordRest when the next <chord> / <note> is read
 */

bool MeiImporter::readGraceGrp(pugi::xml_node graceGrpNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::GraceGrp meiGraceGrp;
    meiGraceGrp.Read(graceGrpNode);

    bool warning = false;
    auto [isAfter, noteType] = Convert::gracegrpFromMEI(meiGraceGrp.GetAttach(), meiGraceGrp.GetGrace(), warning);
    m_graceNoteType = noteType;

    m_readingGraceNotes = GraceAsGrp;

    this->readElements(graceGrpNode, measure, track, ticks);

    if (isAfter) {
        if (m_lastChord) {
            this->addGraceNotesToChord(m_lastChord, true);
        } else {
            this->clearGraceNotes();
        }
    }

    m_readingGraceNotes = GraceNone;

    return true;
}

/**
 * Read a mRest.
 */

bool MeiImporter::readMRest(pugi::xml_node mRestNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::MRest meiMRest;
    meiMRest.Read(mRestNode);

    TDuration duration;

    Segment* segment = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(ticks) + measure->tick());
    Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
    rest->setTicks(m_currentTimeSig);
    rest->setDurationType(DurationType::V_MEASURE);
    rest->setTrack(track);
    segment->add(rest);

    // The duration is the duration according to the timesig
    ticks += rest->ticks().ticks();

    return true;
}

/**
 * Read a note.
 */

bool MeiImporter::readNote(pugi::xml_node noteNode, engraving::Measure* measure, int track, int& ticks, engraving::Chord* chord)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning = false;
    libmei::Note meiNote;
    meiNote.Read(noteNode);

    // Support for @grace without <graceGrp>
    if (!chord) {
        this->readGracedAtt(meiNote);
    }

    pugi::xml_node accidNode = noteNode.select_node(".//accid").node();
    libmei::Accid meiAccid;
    if (accidNode) {
        meiAccid.Read(accidNode);
    } else {
        // Support for non MEI-Basic accid and accid.ges encoded in <note> - this is not accademic...
        meiAccid.Read(noteNode);
    }

    Staff* staff = m_score->staff(track2staff(track));
    engraving::Interval interval = staff->part()->instrument()->transpose();

    Convert::PitchStruct pitchSt = Convert::pitchFromMEI(meiNote, meiAccid, interval, warning);
    if (warning) {
        this->addLog("pitch", noteNode);
        this->addLog("accidental", accidNode);
    }

    if (!chord) {
        chord = static_cast<Chord*>(addChordRest(noteNode, measure, track, meiNote, ticks, false));
        this->readStemsAtt(chord, meiNote);
    }

    Note* note = Factory::createNote(chord);

    int tpc1 = mu::engraving::transposeTpc(pitchSt.tpc2, interval, true);
    note->setPitch(pitchSt.pitch, tpc1, pitchSt.tpc2);

    Accidental* accid = Factory::createAccidental(note);
    accid->setAccidentalType(pitchSt.accidType);
    //accid->setBracket(AccidentalBracket::BRACKET); // Not supported in MEI-Basic
    accid->setRole(pitchSt.accidRole);
    note->add(accid);

    note->setTrack(track);
    chord->add(note);

    return true;
}

/**
 * Read a rest.
 */

bool MeiImporter::readRest(pugi::xml_node restNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Rest meiRest;
    meiRest.Read(restNode);

    Rest* rest = static_cast<Rest*>(addChordRest(restNode, measure, track, meiRest, ticks, true));

    UNUSED(rest);
    //ticks += rest->ticks().ticks();

    return true;
}

/**
 * Read a tuplet.
 */

bool MeiImporter::readTuplet(pugi::xml_node tupletNode, engraving::Measure* measure, int track, int& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    if (m_tuplet) {
        Convert::logs.push_back(u"Nested tuplets are not supported");
        return false;
    }

    bool warning = false;
    bool success = true;

    int startTicks = ticks;
    libmei::Tuplet meiTuplet;
    meiTuplet.Read(tupletNode);

    m_tuplet = Factory::createTuplet(measure);
    Convert::tupletFromMEI(m_tuplet, meiTuplet, warning);
    if (warning) {
        this->addLog("tuplet", tupletNode);
    }

    m_tuplet->setTrack(track);
    m_tuplet->setParent(measure);

    success = readElements(tupletNode, measure, track, ticks);

    Fraction tupletTicks = Fraction::fromTicks(ticks - startTicks);

    TDuration d;
    d.setVal((tupletTicks / m_tuplet->ratio().denominator()).ticks());

    m_tuplet->setBaseLen(d);
    m_tuplet->setTick(Fraction::fromTicks(startTicks) + measure->tick());
    m_tuplet->setTicks(tupletTicks);

    m_tuplet = nullptr;

    return success;
}

//---------------------------------------------------------
// read MEI control events
//---------------------------------------------------------

/**
 * Loop through the content of the MEI <measure> and read the control events
 */

bool MeiImporter::readControlEvents(pugi::xml_node parentNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set elements = parentNode.select_nodes("./*");
    for (pugi::xpath_node xpathNode : elements) {
        std::string elementName = std::string(xpathNode.node().name());
        if (elementName == "breath") {
            success = success && this->readBreath(xpathNode.node(), measure);
        } else if (elementName == "caesura") {
            success = success && this->readCaesura(xpathNode.node(), measure);
        } else if (elementName == "dynam") {
            success = success && this->readDynam(xpathNode.node(), measure);
        } else if (elementName == "fermata") {
            success = success && this->readFermata(xpathNode.node(), measure);
        } else if (elementName == "harm") {
            success = success && this->readHarm(xpathNode.node(), measure);
        } else if (elementName == "repeatMark") {
            success = success && this->readRepeatMark(xpathNode.node(), measure);
        } else if (elementName == "slur") {
            success = success && this->readSlur(xpathNode.node(), measure);
        } else if (elementName == "tempo") {
            success = success && this->readTempo(xpathNode.node(), measure);
        }
    }
    return success;
}

/**
 * Read a breath.
 */

bool MeiImporter::readBreath(pugi::xml_node breathNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Breath meiBreath;
    meiBreath.Read(breathNode);

    Breath* breath = static_cast<Breath*>(this->addAnnotation(meiBreath, measure));
    if (!breath) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    Convert::breathFromMEI(breath, meiBreath, warning);

    return true;
}

/**
 * Read a caesura (
 */

bool MeiImporter::readCaesura(pugi::xml_node caesuraNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Caesura meiCaesura;
    meiCaesura.Read(caesuraNode);

    Breath* breath = static_cast<Breath*>(this->addAnnotation(meiCaesura, measure));
    if (!breath) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    Convert::caesuraFromMEI(breath, meiCaesura, warning);

    return true;
}

/**
 * Read a dynam.
 */

bool MeiImporter::readDynam(pugi::xml_node dynamNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Dynam meiDynam;
    meiDynam.Read(dynamNode);

    Dynamic* dynamic = static_cast<Dynamic*>(this->addAnnotation(meiDynam, measure));
    if (!dynamic) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    StringList meiLines;
    this->readLines(dynamNode, meiLines);

    Convert::dynamFromMEI(dynamic, meiLines, meiDynam, warning);

    return true;
}

/**
 * Read a fermata.
 */

bool MeiImporter::readFermata(pugi::xml_node fermataNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Fermata meiFermata;
    meiFermata.Read(fermataNode);

    Fermata* fermata = nullptr;
    if (meiFermata.HasTstamp()) {
        Fraction fermataPos = Convert::tstampToFraction(meiFermata.GetTstamp(), measure->timesig());
        if (fermataPos == measure->ticks()) {
            Segment* segment = measure->getSegment(SegmentType::EndBarLine, measure->tick() + measure->ticks());
            fermata = Factory::createFermata(segment);
            const int staffIdx
                = (meiFermata.HasStaff() && meiFermata.GetStaff().size() > 0) ? this->getStaffIndex(meiFermata.GetStaff().at(0)) : 0;
            fermata->setTrack(staffIdx * VOICES);
            segment->add(fermata);
        }
    }

    if (!fermata) {
        fermata = static_cast<Fermata*>(this->addAnnotation(meiFermata, measure));
    }

    if (!fermata) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    Convert::fermataFromMEI(fermata, meiFermata, warning);

    return true;
}

/**
 * Read a harm.
 */

bool MeiImporter::readHarm(pugi::xml_node harmNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Harm meiHarm;
    meiHarm.Read(harmNode);

    Harmony* harmony = static_cast<Harmony*>(this->addAnnotation(meiHarm, measure));
    if (!harmony) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    StringList meiLines;
    this->readLines(harmNode, meiLines);

    Convert::harmFromMEI(harmony, meiLines, meiHarm, warning);

    return true;
}

/**
 * Read a repeatMark and create a Jump or a Marker as appropriate.
 * For Jump, default values for jumpTo, playUntil and continueAt are used.
 * Similarly, default value for label is used for Marker.
 */

bool MeiImporter::readRepeatMark(pugi::xml_node repeatMarkNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::RepeatMark meiRepeatMark;
    meiRepeatMark.Read(repeatMarkNode);

    EngravingItem* item = nullptr;
    if (Convert::elementTypeFor(meiRepeatMark) == ElementType::JUMP) {
        item = Factory::createJump(measure);
        Convert::jumpFromMEI(dynamic_cast<Jump*>(item), meiRepeatMark, warning);
    } else {
        item = Factory::createMarker(measure);
        Convert::markerFromMEI(dynamic_cast<Marker*>(item), meiRepeatMark, warning);
    }
    item->setTrack(0);
    measure->add(item);

    return true;
}

/**
 * Read a slur.
 */

bool MeiImporter::readSlur(pugi::xml_node slurNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Slur meiSlur;
    meiSlur.Read(slurNode);

    ChordRest* chordRest = this->findStart(meiSlur, measure);
    if (!chordRest) {
        return true;
    }

    Slur* slur = Factory::createSlur(chordRest->segment());

    m_score->addElement(slur);

    slur->setTick(chordRest->tick());
    slur->setStartElement(chordRest);
    slur->setTrack(chordRest->track());

    // Add it to the map for setting slur end in MeiImporter::addSpannerEnds
    m_openSpannerMap[slur] = slurNode;

    //Convert::tempoFromMEI(tempoText, meiLines, meiSlur, warning);

    return true;
}

/**
 * Read a tempo.
 */

bool MeiImporter::readTempo(pugi::xml_node tempoNode, engraving::Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Tempo meiTempo;
    meiTempo.Read(tempoNode);

    TempoText* tempoText = static_cast<TempoText*>(this->addAnnotation(meiTempo, measure));
    if (!tempoText) {
        // Warning message given in MeiExpoter::addAnnotation
        return true;
    }

    StringList meiLines;
    this->readLinesWithSmufl(tempoNode, meiLines);

    Convert::tempoFromMEI(tempoText, meiLines, meiTempo, warning);

    return true;
}

//---------------------------------------------------------
// read MEI attribute classes
//---------------------------------------------------------

/**
 * Check the presence of `@grace` on <chord> and <note> to see if we have grace encoded not as <graceGrp>
 * Used only when importing files not generated from MuseScore since these will use <graceGrp>.
 * Always set the grace notes not within a <graceGrp> as attached to a following <chord> / <note>.
 * Grace notes at the end of a measure and not in a <graceGrp> will be dropped.
 */

bool MeiImporter::readGracedAtt(libmei::AttGraced& gracedAtt)
{
    // if we are inside a graceGrp, ignore @grace on the chord/node
    if (m_readingGraceNotes == GraceAsGrp) {
        return true;
    }
    if (gracedAtt.HasGrace()) {
        m_readingGraceNotes = GraceAsNote;
        bool warning = false;
        auto [isAfter, noteType] = Convert::gracegrpFromMEI(libmei::graceGrpLog_ATTACH_post, gracedAtt.GetGrace(), warning);
        m_graceNoteType = noteType;
    } else {
        m_readingGraceNotes = GraceNone;
    }

    return true;
}

/**
 * Read the stem direction attributes on <chord> and <note>.
 * Attribute read are `@stem.dir` and `@stem.len` when `0.0` is converted as stem less.
 */

bool MeiImporter::readStemsAtt(engraving::Chord* chord, libmei::AttStems& stemsAtt)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    bool warning = false;

    auto [direction, noStem] = Convert::stemFromMEI(stemsAtt, warning);
    chord->setStemDirection(direction);
    chord->setNoStem(noStem);

    return true;
}

//---------------------------------------------------------
// content extraction methods
//---------------------------------------------------------

/**
 * Build a map of xml:id and corresponding EngravingItem elements.
 * After building the map, all values are `nullptr`.
 * They will be assigned a object when the element with the corresponding `xml:id` is read in MeiImporter::addChordRest
 */
bool MeiImporter::buildIdMap(pugi::xml_node scoreNode)
{
    pugi::xpath_node_set nodesWithStartid = scoreNode.select_nodes("//@startid");

    for (pugi::xpath_node elementXpathNode : nodesWithStartid) {
        std::string id = elementXpathNode.attribute().value();
        if (id.size() < 1 || id.at(0) != '#') {
            continue;
        }
        m_startIdChordRests[id.erase(0, 1)] = nullptr;
    }

    pugi::xpath_node_set nodesWithEndid = scoreNode.select_nodes("//@endid");

    for (pugi::xpath_node elementXpathNode : nodesWithEndid) {
        std::string id = elementXpathNode.attribute().value();
        if (id.size() < 1 || id.at(0) != '#') {
            continue;
        }
        m_endIdChordRests[id.erase(0, 1)] = nullptr;
    }

    return true;
}

/**
 * Create a mapping for <staff> `@n` and <layer> `@n`.
 * Usefull only when reading MEI files where the sequence of `@n` is not starting from 1 or not sequential.
 * Not really useful when reading MEI files generated from MuseScore since these will have sequential numbers starting with 1.
 */

bool MeiImporter::buildStaffLayerMap(pugi::xml_node node)
{
    pugi::xpath_node_set staves = node.select_nodes("//staff");
    // Critical error
    if (staves.empty()) {
        return false;
    }

    for (pugi::xpath_node staffXpathNode : staves) {
        pugi::xml_node staff = staffXpathNode.node();
        int staffN = staff.attribute("n") ? staff.attribute("n").as_int() : 1;
        m_staffNs.insert(staffN);
    }

    pugi::xpath_node_set layers = node.select_nodes("//layer");
    // Critical error
    if (layers.empty()) {
        return false;
    }

    for (pugi::xpath_node layerXpathNode : layers) {
        pugi::xml_node layer = layerXpathNode.node();
        int layerN = layer.attribute("n") ? layer.attribute("n").as_int() : 1;
        pugi::xml_node staff = layer.parent();
        if (!staff) {
            continue;
        }
        const int staffN = staff.attribute("n") ? staff.attribute("n").as_int() : 1;
        const int staffIdx = this->getStaffIndex(staffN);
        m_staffLayerNs[staffIdx].insert(layerN);
    }

    return true;
}

/**
 * Build a title frame from the metaTag values.
 */

bool MeiImporter::buildTextFrame()
{
    VBox* vBox = nullptr;

    if (!m_score->metaTag(u"workTitle").isEmpty()) {
        this->addTextToTitleFrame(vBox, m_score->metaTag(u"workTitle"), TextStyleType::TITLE);
    }

    StringList creators;
    if (!m_score->metaTag(u"composer").isEmpty()) {
        creators << m_score->metaTag(u"composer");
    }
    if (!m_score->metaTag(u"arranger").isEmpty()) {
        creators << m_score->metaTag(u"arranger");
    }
    if (!creators.empty()) {
        this->addTextToTitleFrame(vBox, creators.join(u"\n"), TextStyleType::COMPOSER);
    }

    StringList textCreators;
    if (!m_score->metaTag(u"lyricist").isEmpty()) {
        textCreators << m_score->metaTag(u"lyricist");
    }
    if (!m_score->metaTag(u"translator").isEmpty()) {
        textCreators << m_score->metaTag(u"translator");
    }
    if (!textCreators.empty()) {
        this->addTextToTitleFrame(vBox, textCreators.join(u"\n"), TextStyleType::POET);
    }

    if (vBox) {
        vBox->setTick(Fraction(0, 1));
        m_score->measures()->add(vBox);
    }

    return true;
}

/**
 * Build the score parts from the scoreDef staffGrp@label and staffDef@label.
 */

bool MeiImporter::buildScoreParts(pugi::xml_node scoreDefNode)
{
    bool warning = false;

    // First try to build the list of parts from the labels
    pugi::xpath_node_set labels = scoreDefNode.select_nodes(".//label");
    for (pugi::xpath_node labelXpathNode : labels) {
        pugi::xml_node labelNode = labelXpathNode.node();
        if (!labelNode) {
            continue;
        }

        Part* part = new Part(m_score);
        part->setLongName(String(labelNode.text().as_string()));

        pugi::xml_node labelAbbrNode = labelNode.select_node("./following-sibling::labelAbbr").node();
        if (labelAbbrNode) {
            part->setShortName(String(labelAbbrNode.text().as_string()));
        }

        m_score->appendPart(part);

        // If the label is a child of a staffGrp, the part contains all the child staffDefs
        if (isNode(labelNode.parent(), u"staffGrp")) {
            pugi::xpath_node_set staffDefs = labelNode.parent().select_nodes(".//staffDef");
            for (pugi::xpath_node staffDefXpathNode : staffDefs) {
                libmei::StaffDef meiStaffDef;
                meiStaffDef.Read(staffDefXpathNode.node());
                m_staffParts[meiStaffDef.GetN()] = part;
            }
        }
        // Otherwise the parent must be a staffDef and it is a part with a single staff
        else if (isNode(labelNode.parent(), u"staffDef")) {
            libmei::StaffDef meiStaffDef;
            meiStaffDef.Read(labelNode.parent());
            m_staffParts[meiStaffDef.GetN()] = part;
        } else {
            LOGD() << "MeiImporter::readStaffDefs unknown label part " << labelNode.text();
        }
    }

    pugi::xpath_node_set staffDefs = scoreDefNode.select_nodes(".//staffDef");
    // Critical error
    if (staffDefs.empty()) {
        return false;
    }

    for (pugi::xpath_node staffDefXpathNode : staffDefs) {
        libmei::StaffDef meiStaffDef;
        meiStaffDef.Read(staffDefXpathNode.node());
        Convert::StaffStruct staffSt = Convert::staffFromMEI(meiStaffDef, warning);
        if (warning) {
            this->addLog("staff definition", staffDefXpathNode.node());
        }

        Part* part = nullptr;
        // Try to get the corresponding part from the map
        if (m_staffParts.count(meiStaffDef.GetN()) > 0) {
            part = m_staffParts.at(meiStaffDef.GetN());
        }
        // If not found, create a new part and add it to the score
        else {
            part = new Part(m_score);
            m_score->appendPart(part);
        }
        Staff* staff = Factory::createStaff(part);
        // Mapped index from MEI staff@n to MuseScore index
        const int staffIdx = this->getStaffIndex(meiStaffDef.GetN());
        staff->setId(staffIdx);
        staff->setLines(Fraction(0, 1), staffSt.lines);
        part->instrument()->setTranspose(staffSt.interval);

        m_score->appendStaff(staff);
    }

    int staffSpan = 0;
    size_t idx = 0;
    // Start from the top (default) staffGrp
    pugi::xml_node staffGrpNode = scoreDefNode.select_node("./staffGrp").node();

    bool success = this->readStaffGrps(staffGrpNode, staffSpan, 0, idx);

    return success;
}

//---------------------------------------------------------
// content creation helper methods
//---------------------------------------------------------

/**
 * Add an end barline to a measure.
 */

void MeiImporter::addEndBarLineToMeasure(engraving::Measure* measure, engraving::BarLineType barLineType)
{
    IF_ASSERT_FAILED(measure) {
        return;
    }

    // Here we could check if this is the last measure and not add END because mscore sets it automatically
    Segment* segment = measure->getSegment(SegmentType::EndBarLine, measure->endTick());
    for (staff_idx_t staffIdx = 0; staffIdx < m_score->nstaves(); staffIdx++) {
        BarLine* barLine = Factory::createBarLine(segment);
        barLine->setTrack(staffIdx * VOICES);
        barLine->setBarLineType(barLineType);
        barLine->setSpanStaff(m_score->staff(staffIdx)->barLineSpan());
        segment->add(barLine);
    }
}

/**
 * Add a layout break to a measure.
 */

void MeiImporter::addLayoutBreakToMeasure(engraving::Measure* measure, engraving::LayoutBreakType layoutBreakType)
{
    if (!measure) {
        LOGD() << "MeiImporter::addLayoutBreakToMeasure no measure to add the break";
        return;
    }

    LayoutBreak* layoutBreak = Factory::createLayoutBreak(measure);
    layoutBreak->setLayoutBreakType(layoutBreakType);
    layoutBreak->setTrack(mu::nidx); // this are system elements
    measure->add(layoutBreak);
}

/**
 *  Add some text with a textStyleType to a vBox.
 */

void MeiImporter::addTextToTitleFrame(VBox*& vBox, const String& str, TextStyleType textStyleType)
{
    if (!str.isEmpty()) {
        if (vBox == nullptr) {
            vBox = Factory::createVBox(m_score->dummy()->system());
        }
        Text* text = Factory::createText(vBox, textStyleType);
        text->setPlainText(str);
        vBox->add(text);
    }
}

/**
 * Add the end element / tick / track for spanner ends.
 * This is happening at the end of the import because we need to make sure spanning ends have been read.
 */

void MeiImporter::addSpannerEnds()
{
    for (auto spanner : m_openSpannerMap) {
        ChordRest* chordRest = this->findEnd(spanner.second, nullptr);
        if (!chordRest) {
            continue;
        }
        spanner.first->setTick2(chordRest->tick());
        spanner.first->setEndElement(chordRest);
        spanner.first->setTrack2(chordRest->track());
    }
}
