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

#include "meiimporter.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/utils.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/fingering.h"
#include "thirdparty/libmei/lyrics.h"
#include "thirdparty/libmei/shared.h"
#include "thirdparty/libmei/midi.h"

#include "thirdparty/pugixml.hpp"

using namespace muse;
using namespace mu;
using namespace mu::iex::mei;
using namespace mu::engraving;

#define SCOREDEF_IDX -1

#define MEI_BASIC_VERSION "5.1+basic"

#define MEI_FB_HARM "fb-harm"

/**
 * Read the Score from the file.
 * Return false on error.
 */

bool MeiImporter::read(const muse::io::path_t& path)
{
    m_uids = UIDRegister::instance();
    m_uids->clear();
    m_hasMuseScoreIds = false;

    m_lastMeasure = nullptr;
    m_tremoloId.clear();
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
        Convert::logs.push_back(String("The MEI file does not seem to be a MEI Basic version '%1' file").arg(String(MEI_BASIC_VERSION)));
    }

    bool success = true;

    success = success && this->readMeiHead(root);

    pugi::xml_attribute xmlId = root.attribute("xml:id");
    bool hasRootXmlId = false;
    if (xmlId && !String(xmlId.value()).empty()) {
        hasRootXmlId = true;
        String xmlIdStr = String(xmlId.value());
        if (xmlIdStr.startsWith(u"mscore-")) {
            // Keep a global flag since we are going to read them only if mei@xml:id is given with mscore EID
            m_hasMuseScoreIds = true;
            String valStr = xmlIdStr.remove(u"mscore-").replace('.', '/').replace('-', '+');
            // The  mei@xml:id store the score EID
            EID eid = EID::fromStdString(valStr.toStdString());
            if (eid.isValid()) {
                m_score->setEID(eid);
            }
        } else {
            // Keep it as a seed
            m_score->setMetaTag(u"xml:id", xmlIdStr);
        }
    }

    success = success && this->readScore(root);

    if (hasRootXmlId) {
        // Do not keep a xml:id map when having a xml:id seed or MscoreIds
        m_uids->clear();
    }

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

ChordRest* MeiImporter::addChordRest(pugi::xml_node node, Measure* measure, int track, const libmei::Element& meiElement, Fraction& ticks,
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
    // For grace notes we use a dummy segment, otherwise a ChordRest segment with the appropriate ticks value
    if (m_readingGraceNotes) {
        segment = m_score->dummy()->segment();
    } else {
        segment = measure->getSegment(SegmentType::ChordRest, ticks + measure->tick());
    }

    ChordRest* chordRest = nullptr;
    if (isRest) {
        chordRest = Factory::createRest(segment);
    } else {
        chordRest = Factory::createChord(segment);
    }

    // Do not use single note xml:id / EID for the ChordRest
    if (!dynamic_cast<const libmei::Note*>(&meiElement)) {
        this->readXmlId(chordRest, meiElement.m_xmlId);
    }

    if (m_startIdChordRests.count(meiElement.m_xmlId)) {
        m_startIdChordRests[meiElement.m_xmlId] = chordRest;
    }
    if (m_endIdChordRests.count(meiElement.m_xmlId)) {
        m_endIdChordRests[meiElement.m_xmlId] = chordRest;
    }
    if (m_plistValueChordRests.count(meiElement.m_xmlId)) {
        m_plistValueChordRests[meiElement.m_xmlId] = chordRest;
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
        ticks += chordTicks;
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
 * Add grace notes to a ChordRest When a grace group was previously read and added to MeiImporter::m_graceNotes
 * Ignore (delete) the grace notes if the ChordRest is a Rest.
 * Look at m_graceNoteType for setting the acciaccatura note type (when grace notes precede only)
 */

bool MeiImporter::addGraceNotesToChord(ChordRest* chordRest, bool isAfter)
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
    } else if (meiElement.m_name == "dir") {
        ElementType elementType = Convert::elementTypeForDir(meiElement);
        switch (elementType) {
        case (ElementType::PLAYTECH_ANNOTATION): item = Factory::createPlayTechAnnotation(
                chordRest->segment(), PlayingTechniqueType::Natural, TextStyleType::STAFF);
            break;
        case (ElementType::STAFF_TEXT): item = Factory::createStaffText(chordRest->segment());
            break;
        default:
            item = Factory::createExpression(chordRest->segment());
        }
    } else if (meiElement.m_name == "dynam") {
        item = Factory::createDynamic(chordRest->segment());
    } else if (meiElement.m_name == "fermata") {
        item = Factory::createFermata(chordRest->segment());
    } else if (meiElement.m_name == "harm") {
        const libmei::AttLabelled* labeledAtt = dynamic_cast<const libmei::AttLabelled*>(&meiElement);
        if (labeledAtt && (labeledAtt->GetLabel() == MEI_FB_HARM)) {
            item = Factory::createFiguredBass(chordRest->segment());
        } else {
            item = Factory::createHarmony(chordRest->segment());
        }
    } else if (meiElement.m_name == "harpPedal") {
        item = Factory::createHarpPedalDiagram(chordRest->segment());
    } else if (meiElement.m_name == "reh") {
        item = Factory::createRehearsalMark(chordRest->segment());
    } else if (meiElement.m_name == "tempo") {
        item = Factory::createTempoText(chordRest->segment());
    } else {
        return nullptr;
    }
    this->readXmlId(item, meiElement.m_xmlId);

    item->setTrack(chordRest->track());
    segment->add(item);

    return item;
}

/**
 * Create a spanner (MEI control event with @startid end @endid).
 * Create the Spanner according to the MEI element name (e.g., "slur", "tie").
 * Add the corresponding pugixml::node to the m_openSpannerMap to be closed later
 * Return nullptr if the lookup fails.
 */

Spanner* MeiImporter::addSpanner(const libmei::Element& meiElement, Measure* measure, pugi::xml_node node)
{
    ChordRest* chordRest = this->findStart(meiElement, measure);
    if (!chordRest) {
        return nullptr;
    }

    Spanner* item = nullptr;

    if (meiElement.m_name == "dir") {
        ElementType elementType = Convert::elementTypeForDirWithExt(meiElement);
        switch (elementType) {
        case (ElementType::HAIRPIN): item = Factory::createHairpin(
                chordRest->segment());
            break;
        default:
            item = Factory::createTextLine(chordRest->segment());
        }
    } else if (meiElement.m_name == "hairpin") {
        item = Factory::createHairpin(chordRest->segment());
    } else if (meiElement.m_name == "octave") {
        item = Factory::createOttava(chordRest->segment());
    } else if (meiElement.m_name == "pedal") {
        item = Factory::createPedal(chordRest->segment());
    } else if (meiElement.m_name == "slur") {
        item = Factory::createSlur(chordRest->segment());
    } else if (meiElement.m_name == "trill") {
        item = Factory::createTrill(chordRest->segment());
    } else {
        return nullptr;
    }
    this->readXmlId(item, meiElement.m_xmlId);

    item->setTick(chordRest->tick());
    item->setStartElement(chordRest);
    item->setTrack(chordRest->track());
    item->setTrack2(chordRest->track());

    m_score->addElement(item);

    // Add it to the map for setting spanner end in MeiImporter::addSpannerEnds
    m_openSpannerMap[item] = node;

    return item;
}

/**
 * Create a articulation (MEI control event with @startid).
 * Create the EngravingItem according to the MEI element name (e.g., "mordent", "turn")
 * If measure is null, then chord is expected and no lookup for the chordRest will be performed (e.g., for "artic")
 * The articulation object is added to the EngravingItem (and not to the segment as for annotations)
 * Return nullptr if the lookup fails.
 */

EngravingItem* MeiImporter::addToChordRest(const libmei::Element& meiElement, Measure* measure, Chord* chord)
{
    ChordRest* chordRest = (!measure) ? chord : this->findStart(meiElement, measure);
    if (!chordRest) {
        return nullptr;
    }

    EngravingItem* item = nullptr;

    static const std::vector<std::string> s_ornaments = { "mordent", "ornam", "trill", "turn" };

    if (std::find(s_ornaments.begin(), s_ornaments.end(), meiElement.m_name) != s_ornaments.end()) {
        item = Factory::createOrnament(chordRest);
    } else if (meiElement.m_name == "arpeg") {
        if (chordRest->isChord()) {
            item = Factory::createArpeggio(toChord(chordRest));
        }
    } else if (meiElement.m_name == "artic") {
        item = Factory::createArticulation(chordRest);
    } else {
        return nullptr;
    }
    this->readXmlId(item, meiElement.m_xmlId);

    item->setTrack(chordRest->track());
    chordRest->add(item);

    return item;
}

/**
 * Basic helper that removes the '#' character from a dataURI reference to \@xml:id.
 */

std::string MeiImporter::xmlIdFrom(std::string dataURI)
{
    // Basic check for the @stardid validity
    if (dataURI.size() < 1 || dataURI.at(0) != '#') {
        Convert::logs.push_back(String("Could not convert the data.URI '%1'").arg(String::fromStdString(dataURI)));
        return "";
    }
    return dataURI.erase(0, 1);
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
        std::string startId = this->xmlIdFrom(startIdAtt->GetStartid());
        // The startid corresponding ChordRest should have been added to the m_startIdChordRests previously
        if (!m_startIdChordRests.count(startId) || !m_startIdChordRests.at(startId)) {
            Convert::logs.push_back(String("Could not find element for @startid '%1'").arg(String::fromStdString(
                                                                                               startIdAtt->GetStartid())));
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
 * Do a lookup in the m_endIdChordRests map with the @endid value to retrieve the ChordRest to which the annotation points to.
 * If there is not @endid but a @tstamp2 (MEI not written by MuseScore), try to find the corresponding ChordRest
 */

ChordRest* MeiImporter::findEnd(pugi::xml_node controlNode, const ChordRest* startChordRest)
{
    libmei::InstStartEndId startEndIdAtt;
    startEndIdAtt.ReadStartEndId(controlNode);

    ChordRest* chordRest = nullptr;
    if (startEndIdAtt.HasEndid()) {
        std::string endId = this->xmlIdFrom(startEndIdAtt.GetEndid());
        // The @endid corresponding ChordRest should have been added to the m_endIdChordRests previously
        if (!m_endIdChordRests.count(endId) || !m_endIdChordRests.at(endId)) {
            Convert::logs.push_back(String("Could not find element for @endid '%1'").arg(String::fromStdString(startEndIdAtt.GetEndid())));
            return nullptr;
        }
        chordRest = m_endIdChordRests.at(endId);
    } else {
        // No @endid, try a lookup based on the @tstamp2. This is only for files not written via MuseScore
        libmei::InstTimestamp2Log timestamp2LogAtt;
        timestamp2LogAtt.ReadTimestamp2Log(controlNode);
        libmei::InstStaffIdent staffIdentAtt;
        staffIdentAtt.ReadStaffIdent(controlNode);
        libmei::InstLayerIdent layerIdentAtt;
        layerIdentAtt.ReadLayerIdent(controlNode);

        // We need at least a @tstamp2 and a startChordRest with its Measure
        if (!timestamp2LogAtt.HasTstamp2() || !startChordRest || !startChordRest->measure()) {
            return nullptr;
        }

        libmei::data_MEASUREBEAT tstamp2Value = timestamp2LogAtt.GetTstamp2();

        // Find the end Measure
        Measure* measure = startChordRest->measure();
        for (int i = tstamp2Value.first; i > 0; --i) {
            if (!measure->next() || !measure->next()->isMeasure()) {
                return nullptr;
            }
            measure = toMeasure(measure->next());
        }

        Fraction tstampFraction = Convert::tstampToFraction(tstamp2Value.second, measure->timesig());
        // Use the startChordRest staffIdx unless given in @staff
        staff_idx_t staffIdx = (staffIdentAtt.HasStaff() && staffIdentAtt.GetStaff().size() > 0) ? this->getStaffIndex(
            staffIdentAtt.GetStaff().at(0)) : startChordRest->staffIdx();
        // Use the startChordRest voice unless given in @layer
        track_idx_t layer
            = (layerIdentAtt.HasLayer()) ? this->getVoiceIndex(static_cast<int>(staffIdx),
                                                               layerIdentAtt.GetLayer()) : startChordRest->voice();

        chordRest = measure->findChordRest(measure->tick() + tstampFraction, staffIdx * VOICES + layer);
        if (!chordRest) {
            Convert::logs.push_back(String("Could not find element corresponding to @tstamp2 '%1m+%2'").arg(tstamp2Value.first).arg(
                                        tstamp2Value.second));
            return nullptr;
        }
    }

    return chordRest;
}

/**
 * Look for the Note for an MEI element with @startid.
 * Do a lookup in the m_startIdNotes map with the @startid value to retrieve the Note to which the element points to.
 */

Note* MeiImporter::findStartNote(const libmei::Element& meiElement)
{
    const libmei::AttStartId* startIdAtt = dynamic_cast<const libmei::AttStartId*>(&meiElement);
    IF_ASSERT_FAILED(startIdAtt && startIdAtt->HasStartid()) {
        return nullptr;
    }

    std::string startId = this->xmlIdFrom(startIdAtt->GetStartid());
    // The startid corresponding Note should have been added to the m_startIdNotes previously
    if (!m_startIdNotes.count(startId) || !m_startIdNotes.at(startId)) {
        Convert::logs.push_back(String("Could not find note for @startid '%1'").arg(String::fromStdString(startIdAtt->GetStartid())));
        return nullptr;
    }
    return m_startIdNotes.at(startId);
}

/**
 * Look for the Note for an MEI element with @endid.
 * Do a lookup in the m_endIdNotes map with the @endid value to retrieve the Note to which the element points to.
 */

Note* MeiImporter::findEndNote(pugi::xml_node controlNode)
{
    libmei::InstStartEndId startEndIdAtt;
    startEndIdAtt.ReadStartEndId(controlNode);

    // This should not happen because an element without an @endid will not have been added to the map
    if (!startEndIdAtt.HasEndid()) {
        return nullptr;
    }

    std::string endId = this->xmlIdFrom(startEndIdAtt.GetEndid());
    // The endid corresponding Note should have been added to the m_endIdNotes previously
    if (!m_endIdNotes.count(endId) || !m_endIdNotes.at(endId)) {
        Convert::logs.push_back(String("Could not find note for @endid '%1'").arg(String::fromStdString(startEndIdAtt.GetEndid())));
        return nullptr;
    }
    return m_endIdNotes.at(endId);
}

/**
 * Look for the ChordRest for an MEI element with @plist.
 * Do a lookup in the m_plistValueChordRests map with the @plist values to retrieve all the ChordRests to which the plist refers to.
 */

const std::list<ChordRest*> MeiImporter::findPlistChordRests(pugi::xml_node controlNode)
{
    libmei::InstPlist plistAtt;
    plistAtt.ReadPlist(controlNode);

    // This should not happen because an element without a @plist will not have been added to the map
    if (!plistAtt.HasPlist()) {
        return {};
    }

    std::list<ChordRest*> plistChordRests;
    for (auto& id : plistAtt.GetPlist()) {
        std::string plistValue = this->xmlIdFrom(id);
        // The plist corresponding Note should have been added to the m_plistValueChordRests previously
        if (!m_plistValueChordRests.count(plistValue) || !m_plistValueChordRests.at(plistValue)) {
            Convert::logs.push_back(String("Could not find note for @plist value '%1'").arg(String::fromStdString(id)));
            continue;
        }
        plistChordRests.push_back(m_plistValueChordRests.at(plistValue));
    }
    return plistChordRests;
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

/**
 * Check if we have an Lyrics to extend for the given track and Lyrics no.
 */

bool MeiImporter::hasLyricsToExtend(track_idx_t track, int no)
{
    return (m_lyricExtenders.count(track) > 0) && (m_lyricExtenders.at(track).count(no) > 0);
}

/**
 * Get the Lyrics / ChordRest endpoint pair.
 * Existence of the pair need to be checked first with MeiImporter::hasLyricsToExtend.
 */

const std::pair<Lyrics*, ChordRest*>& MeiImporter::getLyricsToExtend(track_idx_t track, int no)
{
    return m_lyricExtenders.at(track).at(no);
}

/**
 * Add a ChordRest as the tentative end point for the extended Lyrics.
 * Apply it to all the Lyrics no for the ChordRest track.
 */

void MeiImporter::addChordtoLyricsToExtend(ChordRest* chordRest)
{
    track_idx_t track = chordRest->track();
    if (m_lyricExtenders.count(track) > 0) {
        auto map = &m_lyricExtenders.at(track);
        for (auto& item : *map) {
            item.second.second = chordRest;
        }
    }
}

/**
 * Extend a Lyrics by setting the appropriate ticks.
 * Calculate it based on the tick of the ChordRest marking the end of the extension.
 */

void MeiImporter::extendLyrics(const std::pair<Lyrics*, ChordRest*>& lyricsToExtend)
{
    if (lyricsToExtend.second) {
        Fraction ticks = lyricsToExtend.second->tick() - lyricsToExtend.first->chordRest()->tick();
        lyricsToExtend.first->setTicks(ticks);
    }
}

/**
 * Extend all lyrics remaining.
 * Called at the end of the import for extended lyrics closed with a following verse.
 */

void MeiImporter::extendLyrics()
{
    for (auto& itemTrack : m_lyricExtenders) {
        for (auto& itemNo : itemTrack.second) {
            this->extendLyrics(itemNo.second);
        }
    }
}

/**
 * Creates the accidentals (above and below) for an ornament.
 * Currently does not work if the corresponding OrnamentInterval value is not set in the Ornament.
 */

void MeiImporter::setOrnamentAccid(engraving::Ornament* ornament, const Convert::OrnamStruct& ornamSt)
{
    if (ornament->hasIntervalAbove() && (ornamSt.accidTypeAbove != AccidentalType::NONE)) {
        Accidental* accidental = Factory::createAccidental(ornament);
        accidental->setAccidentalType(ornamSt.accidTypeAbove);
        accidental->setParent(ornament);
        ornament->setAccidentalAbove(accidental);
    }
    if (ornament->hasIntervalBelow() && (ornamSt.accidTypeBelow != AccidentalType::NONE)) {
        Accidental* accidental = Factory::createAccidental(ornament);
        accidental->setAccidentalType(ornamSt.accidTypeBelow);
        accidental->setParent(ornament);
        ornament->setAccidentalBelow(accidental);
    }
}

void MeiImporter::readXmlId(engraving::EngravingItem* item, const std::string& meiUID)
{
    String xmlIdStr = String::fromStdString(meiUID);
    // We have a file that has MuseScore EIDs and one on this element
    if (m_hasMuseScoreIds && xmlIdStr.startsWith(u"mscore-")) {
        String valStr = xmlIdStr.remove(u"mscore-").replace('.', '/').replace('-', '+');
        EID eid = EID::fromStdString(valStr.toStdString());
        if (!eid.isValid()) {
            Convert::logs.push_back(String("A valid MuseScore ID could not be extracted from '%1'").arg(xmlIdStr));
        } else {
            item->setEID(eid);
        }
    } else {
        m_uids->reg(item, meiUID);
    }
}

//---------------------------------------------------------
// parsing methods
//---------------------------------------------------------

/**
 * Read the <meiHead> and stores it as a custom MuseScore metatag
 * Also try to fill in some metadata
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

    pugi::xml_node firstTitleNode = root.select_node("//meiHead/fileDesc/titleStmt/title").node();
    if (firstTitleNode) {
        // assume the first title to be the main title
        m_score->setMetaTag(u"workTitle", String(firstTitleNode.text().as_string()));
    }
    pugi::xpath_node_set workTitleNodes = root.select_nodes("//meiHead/fileDesc/titleStmt/title[@type]");
    for (pugi::xpath_node workTitleNode : workTitleNodes) {
        const String type = String(workTitleNode.node().attribute("type").as_string());
        if (type == u"main") {
            if (m_score->metaTag(u"workTitle").isEmpty()) {
                m_score->setMetaTag(u"workTitle", String(workTitleNode.node().text().as_string()));
            }
        } else if (type == u"subordinate") {
            m_score->setMetaTag(u"subtitle", String(workTitleNode.node().text().as_string()));
        } else {
            const String metaTag = type + u"Title";
            m_score->setMetaTag(metaTag, String(workTitleNode.node().text().as_string()));
        }
    }

    // check for dedicated elements (only for import)
    pugi::xml_node composer = root.select_node("//meiHead/fileDesc/titleStmt/composer").node();
    if (composer) {
        m_score->setMetaTag(u"composer", String(composer.text().as_string()));
    }
    pugi::xml_node lyricist = root.select_node("//meiHead/fileDesc/titleStmt/lyricist").node();
    if (lyricist) {
        m_score->setMetaTag(u"lyricist", String(lyricist.text().as_string()));
    }
    pugi::xml_node arranger = root.select_node("//meiHead/fileDesc/titleStmt/arranger").node();
    if (arranger) {
        m_score->setMetaTag(u"arranger", String(arranger.text().as_string()));
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

    pugi::xml_node copyrightNode = root.select_node("//meiHead/fileDesc/pubStmt/availability").node();
    if (copyrightNode) {
        m_score->setMetaTag(u"copyright", String(copyrightNode.text().as_string()));
    }

    return true;
}

/**
 * Read the MEI score.
 * Previously builds a map of IDs being referred to (e.g., through `@startid` or `@endid`)
 * Also builds a map for staff@n and layer@n when reading MEI files not produced with MuseScore.
 * Reads the initial scoreDef before reading the section elements.
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
    this->extendLyrics();

    return success;
}

/**
 * Read a scoreDef (initial or intermediate).
 * For the initial scoreDef, also tries to build the part structure from the scoreDef relying on staffGrp@label and staffDef@label.
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
        size_t line = 0;
        this->readLines(rendNode, lines, line);

        TextStyleType textStyle = Convert::textFromMEI(meiRend, warning);
        if (warning) {
            this->addLog("text label", rendNode);
        }

        if (!vBox) {
            vBox = Factory::createTitleVBox(m_score->dummy()->system());
        }

        Text* text = Factory::createText(vBox, textStyle);
        text->setPlainText(lines.join(u"\n"));
        vBox->add(text);
    }

    if (vBox) {
        m_score->measures()->append(vBox);
    }

    return true;
}

/**
 * Read the lines of a textual mixed content.
 */

bool MeiImporter::readLines(pugi::xml_node parentNode, StringList& lines, size_t& line)
{
    for (pugi::xml_node child : parentNode.children()) {
        if (child.type() == pugi::node_pcdata) {
            // This is the first time we are adding text to the current line - push it
            if (lines.size() <= line) {
                lines.push_back(String(child.text().as_string()));
            }
            // If not, then concatenate it to the current line
            else {
                lines.at(line) += String(child.text().as_string());
            }
        } else if (this->isNode(child, u"lb")) {
            line += 1;
        } else {
            // Try to recursively read the text content of child nodes
            // For files not written by MuseScore with additional /rend elements
            this->addLog("unsupported child element, reading text only", child);
            readLines(child, lines, line);
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
            bool isSmufl = (meiRend.HasGlyphAuth() && meiRend.GetGlyphAuth() == SMUFL_AUTH);
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

        // try to import MEI from other applications
        pugi::xml_node meterSigNode = staffDefXpathNode.node().select_node(".//meterSig").node();
        if (meterSigNode) {
            meiStaffDef.SetMeterCount(meiStaffDef.AttMeterSigDefaultLog::StrToMetercountPair(meterSigNode.attribute("count").value()));
            meiStaffDef.SetMeterUnit(meterSigNode.attribute("unit").as_int());
            meiStaffDef.SetMeterSym(meiStaffDef.AttMeterSigDefaultLog::StrToMetersign(meterSigNode.attribute("sym").value()));
        }

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

    libmei::Section meiSection;
    meiSection.Read(parentNode);

    if (meiSection.HasRestart() && meiSection.GetRestart() == libmei::BOOLEAN_true) {
        MeasureBase* lastMeasureBase = !m_score->measures()->empty() ? m_score->measures()->last() : nullptr;
        if (lastMeasureBase) {
            m_score->insertBox(ElementType::HBOX, lastMeasureBase);
        }
    }

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
        this->readXmlId(volta, meiEnding.m_xmlId);
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
 * Try to manage measure offset looking at MEI measure@n (num-like values).
 * Adjust various flags (end barline, repeat counts).
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
    this->readXmlId(measure, meiMeasure.m_xmlId);
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

    Fraction measureTicks(0, 1);
    success = success & this->readStaves(measureNode, measure, measureTicks);
    // Make sure we correct empty content because this would crash MuseScore
    if (measureTicks.isZero()) {
        measureTicks = m_currentTimeSig;
        LOGD() << "MeiImporter::readMeasure empty content in " << meiMeasure.GetN();
    }
    measure->setTicks(measureTicks);

    success = success & this->readControlEvents(measureNode, measure);

    measure->setRepeatStart(measureSt.repeatStart);
    if (measureSt.repeatEnd) {
        measure->setRepeatEnd(true);
    } else if (measureSt.endBarLineType != BarLineType::NORMAL) {
        this->addEndBarLineToMeasure(measure, measureSt.endBarLineType);
    }

    if (measureSt.repeatCount) {
        measure->setRepeatCount(measureSt.repeatCount);
    }

    m_score->measures()->append(measure);
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
        this->addLayoutBreakToMeasure(m_lastMeasure, LayoutBreakType::PAGE);
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
        this->addLayoutBreakToMeasure(m_lastMeasure, LayoutBreakType::LINE);
    }

    return true;
}

/**
 * Read all staves of a measure and their content.
 * Lookup (and clear) the time signature and key signature maps to add them if necessary.
 */

bool MeiImporter::readStaves(pugi::xml_node parentNode, Measure* measure, Fraction& measureTicks)
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
            ksEvent.setConcertKey(m_keySigs.at(keySigIdx));
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

bool MeiImporter::readLayers(pugi::xml_node parentNode, Measure* measure, int staffN, Fraction& measureTicks)
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
        // We cannot have more than 4 voices in MuseScore
        if (i >= VOICES) {
            Convert::logs.push_back(String("More than %1 layers in a staff in not supported. Their content will not be imported.").arg(
                                        VOICES));
            break;
        }
        libmei::Layer meiLayer;
        meiLayer.Read(xpathNode.node());

        m_lastChord = nullptr;
        Fraction ticks;
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

bool MeiImporter::readElements(pugi::xml_node parentNode, Measure* measure, int track, Fraction& ticks)
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
        } else if (elementName == "bTrem") {
            success = success && this->readBTrem(xpathNode.node(), measure, track, ticks);
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
        } else if (elementName == "mRpt") {
            success = success && this->readMRpt(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "rest" && !m_readingGraceNotes) {
            success = success && this->readRest(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "space" && !m_readingGraceNotes) {
            success = success && this->readSpace(xpathNode.node(), measure, track, ticks);
        } else if (elementName == "tuplet" && !m_readingGraceNotes) {
            success = success && this->readTuplet(xpathNode.node(), measure, track, ticks);
        } else {
            success = success && this->readElements(xpathNode.node(), measure, track, ticks);
        }
    }
    return success;
}

/**
 * Loop through the content of the MEI <note> or <chord> and read the artics
 */

bool MeiImporter::readArtics(pugi::xml_node parentNode, Chord* chord)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set elements = parentNode.select_nodes("./artic");
    for (pugi::xpath_node xpathNode : elements) {
        success = success && this->readArtic(xpathNode.node(), chord);
    }

    return success;
}

/**
 * Read an artic.
 */

bool MeiImporter::readArtic(pugi::xml_node articNode, Chord* chord)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    bool warning = false;
    libmei::Artic meiArtic;
    meiArtic.Read(articNode);

    Articulation* articulation = static_cast<Articulation*>(this->addToChordRest(meiArtic, nullptr, chord));
    if (!articulation) {
        // Warning message given in MeiImporter::addSpanner
        return true;
    }

    Convert::articFromMEI(articulation, meiArtic, warning);

    return true;
}

/**
 * Read a beam.
 * Set MuseScore flags based on custom `@type` values.
 */

bool MeiImporter::readBeam(pugi::xml_node beamNode, Measure* measure, int track, Fraction& ticks)
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
 * Read a bTrem.
 * Set MuseScore TremoloType.
 */

bool MeiImporter::readBTrem(pugi::xml_node bTremNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    libmei::BTrem meiBTrem;
    meiBTrem.Read(bTremNode);
    m_tremoloId = meiBTrem.m_xmlId;

    success = readElements(bTremNode, measure, track, ticks);

    m_tremoloId.clear();

    return success;
}

/**
 * Read a chord and its content (note elements).
 */

bool MeiImporter::readChord(pugi::xml_node chordNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Chord meiChord;
    meiChord.Read(chordNode);

    Fraction chordTicks = ticks;

    // Support for @grace without <graceGrp>
    this->readGracedAtt(meiChord);
    Chord* chord = static_cast<Chord*>(addChordRest(chordNode, measure, track, meiChord, ticks, false));
    this->readStemsAtt(chord, meiChord);
    this->readArtics(chordNode, chord);

    if (!m_tremoloId.empty()) {
        TremoloType ttype = Convert::stemModFromMEI(meiChord.GetStemMod());
        if (isTremoloTwoChord(ttype)) {
            NOT_SUPPORTED;
        } else {
            TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(chord);
            this->readXmlId(tremolo, m_tremoloId);
            tremolo->setTremoloType(ttype);
            chord->add(tremolo);
        }
    }

    pugi::xpath_node_set notes = chordNode.select_nodes(".//note");
    for (pugi::xpath_node xpathNode : notes) {
        this->readNote(xpathNode.node(), measure, track, chordTicks, chord);
    }

    return true;
}

/*
 * Read a clef.
 */

bool MeiImporter::readClef(pugi::xml_node clefNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning = false;
    libmei::Clef meiClef;
    meiClef.Read(clefNode);

    Segment* segment = measure->getSegment(SegmentType::Clef, ticks + measure->tick());
    Clef* clef = Factory::createClef(segment);
    Convert::colorFromMEI(clef, meiClef);
    this->readXmlId(clef, meiClef.m_xmlId);
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

bool MeiImporter::readGraceGrp(pugi::xml_node graceGrpNode, Measure* measure, int track, Fraction& ticks)
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

bool MeiImporter::readMRest(pugi::xml_node mRestNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::MRest meiMRest;
    meiMRest.Read(mRestNode);

    TDuration duration;

    Segment* segment = measure->getSegment(SegmentType::ChordRest, ticks + measure->tick());
    Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
    Convert::colorFromMEI(rest, meiMRest);
    this->readXmlId(rest, meiMRest.m_xmlId);
    rest->setTicks(m_currentTimeSig);
    rest->setDurationType(DurationType::V_MEASURE);
    rest->setTrack(track);
    segment->add(rest);

    // Also add mRest as @startid / @endid
    if (m_startIdChordRests.count(meiMRest.m_xmlId)) {
        m_startIdChordRests[meiMRest.m_xmlId] = rest;
    }
    if (m_endIdChordRests.count(meiMRest.m_xmlId)) {
        m_endIdChordRests[meiMRest.m_xmlId] = rest;
    }

    // The duration is the duration according to the timesig
    ticks += rest->ticks();

    return true;
}

/**
 * Read a mRpt.
 */

bool MeiImporter::readMRpt(pugi::xml_node mRptNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::MRpt meiMRpt;
    meiMRpt.Read(mRptNode);

    if (meiMRpt.GetExpand() == libmei::BOOLEAN_true) {
        LOGD() << "MeiImporter::readMRpt cannot expand measure repeats";
    }

    Segment* segment = measure->getSegment(SegmentType::ChordRest, ticks + measure->tick());
    MeasureRepeat* measureRepeat = Factory::createMeasureRepeat(segment);
    Convert::colorFromMEI(measureRepeat, meiMRpt);
    this->readXmlId(measureRepeat, meiMRpt.m_xmlId);
    measureRepeat->setTrack(track);
    measureRepeat->setTicks(measure->ticks());
    measureRepeat->setNumMeasures(1);
    measure->setMeasureRepeatCount(1, track2staff(track));
    segment->add(measureRepeat);

    return true;
}

/**
 * Read a note.
 */

bool MeiImporter::readNote(pugi::xml_node noteNode, Measure* measure, int track, Fraction& ticks, Chord* chord)
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
        // Support for non MEI Basic accid and accid.ges encoded in <note> - this is not academic...
        meiAccid.Read(noteNode);
        // Remove the xml:id read from the note in that case
        meiAccid.m_xmlId = "";
    }

    Staff* staff = m_score->staff(track2staff(track));
    Interval interval = staff->part()->instrument()->transpose();

    Convert::PitchStruct pitchSt = Convert::pitchFromMEI(meiNote, meiAccid, interval, warning);
    if (warning) {
        this->addLog("pitch", noteNode);
        this->addLog("accidental", accidNode);
    }

    if (!chord) {
        chord = static_cast<Chord*>(addChordRest(noteNode, measure, track, meiNote, ticks, false));
        this->readStemsAtt(chord, meiNote);
        this->readArtics(noteNode, chord);
        this->readVerses(noteNode, chord);
        if (!m_tremoloId.empty()) {
            TremoloType ttype = Convert::stemModFromMEI(meiNote.GetStemMod());
            if (isTremoloTwoChord(ttype)) {
                NOT_SUPPORTED;
            } else {
                TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(chord);
                this->readXmlId(tremolo, m_tremoloId);
                tremolo->setTremoloType(ttype);
                chord->add(tremolo);
            }
        }
    }

    Note* note = Factory::createNote(chord);
    Convert::colorFromMEI(note, meiNote);
    this->readXmlId(note, meiNote.m_xmlId);

    // If there is a reference to the note in the MEI, add it the maps (e.g., for ties)
    if (m_startIdChordRests.count(meiNote.m_xmlId)) {
        m_startIdNotes[meiNote.m_xmlId] = note;
    }
    if (m_endIdChordRests.count(meiNote.m_xmlId)) {
        m_endIdNotes[meiNote.m_xmlId] = note;
    }

    int tpc1 = mu::engraving::transposeTpc(pitchSt.tpc2, interval, true);
    note->setPitch(pitchSt.pitch, tpc1, pitchSt.tpc2);

    if (meiNote.HasVel()) {
        note->setUserVelocity(meiNote.GetVel());
    }

    Accidental* accid = Factory::createAccidental(note);
    Convert::colorFromMEI(accid, meiAccid);
    this->readXmlId(accid, meiAccid.m_xmlId);
    accid->setAccidentalType(pitchSt.accidType);
    accid->setBracket(pitchSt.accidBracket);
    accid->setRole(pitchSt.accidRole);
    note->add(accid);

    note->setTrack(track);
    chord->add(note);

    return true;
}

/**
 * Read a rest.
 */

bool MeiImporter::readRest(pugi::xml_node restNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Rest meiRest;
    meiRest.Read(restNode);

    Rest* rest = static_cast<Rest*>(addChordRest(restNode, measure, track, meiRest, ticks, true));
    Convert::colorFromMEI(rest, meiRest);

    UNUSED(rest);

    return true;
}

/**
 * Read a space.
 */

bool MeiImporter::readSpace(pugi::xml_node spaceNode, Measure* measure, int track, Fraction& ticks)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Space meiSpace;
    meiSpace.Read(spaceNode);

    Rest* space = static_cast<Rest*>(addChordRest(spaceNode, measure, track, meiSpace, ticks, true));
    space->setVisible(false);

    return true;
}

/**
 * Read a syl
 */

bool MeiImporter::readSyl(pugi::xml_node sylNode, Lyrics* lyrics, Convert::textWithSmufl& textBlocks, ElisionType elision)
{
    IF_ASSERT_FAILED(lyrics) {
        return false;
    }

    bool warning;
    libmei::Syl meiSyl;
    meiSyl.Read(sylNode);

    Convert::sylFromMEI(lyrics, meiSyl, elision, warning);

    textBlocks.push_back(std::make_pair(false, String(sylNode.text().as_string())));

    return true;
}

/**
 * Read a tuplet.
 */

bool MeiImporter::readTuplet(pugi::xml_node tupletNode, Measure* measure, int track, Fraction& ticks)
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

    Fraction startTicks = ticks;
    libmei::Tuplet meiTuplet;
    meiTuplet.Read(tupletNode);

    m_tuplet = Factory::createTuplet(measure);
    this->readXmlId(m_tuplet, meiTuplet.m_xmlId);
    Convert::tupletFromMEI(m_tuplet, meiTuplet, warning);
    if (warning) {
        this->addLog("tuplet", tupletNode);
    }

    m_tuplet->setTrack(track);
    m_tuplet->setParent(measure);

    success = readElements(tupletNode, measure, track, ticks);

    Fraction tupletTicks = ticks - startTicks;

    TDuration d;
    d.setVal((tupletTicks / m_tuplet->ratio().denominator()).ticks());

    m_tuplet->setBaseLen(d);
    m_tuplet->setTick(startTicks + measure->tick());
    m_tuplet->setTicks(tupletTicks);

    m_tuplet = nullptr;

    return success;
}

/**
 * Loop through the content of the MEI <note> or <chord> and read the verses
 */

bool MeiImporter::readVerses(pugi::xml_node parentNode, Chord* chord)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    bool success = true;

    pugi::xpath_node_set elements = parentNode.select_nodes("./verse");
    for (pugi::xpath_node xpathNode : elements) {
        success = success && this->readVerse(xpathNode.node(), chord);
    }

    // The chord as a potential end point for the lyrics to extend (all lyrics no for that track)
    this->addChordtoLyricsToExtend(chord);

    return success;
}

/**
 * Read a verse.
 */

bool MeiImporter::readVerse(pugi::xml_node verseNode, Chord* chord)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    libmei::Verse meiVerse;
    meiVerse.Read(verseNode);

    int no = 0;
    if (meiVerse.HasN()) {
        no = std::stoi(meiVerse.GetN()) - 1;
        // Make sure we have no verse number below 0;
        no = std::max(0, no);
    }

    // First check if we have a lyric extension that needs to be ended for that track / no
    if (this->hasLyricsToExtend(chord->track(), no)) {
        auto lyricsToExtend = this->getLyricsToExtend(chord->track(), no);
        this->extendLyrics(lyricsToExtend);
        // Remove it from the lyrics to extend
        m_lyricExtenders.at(chord->track()).erase(no);
    }

    Lyrics* lyrics = Factory::createLyrics(chord);
    this->readXmlId(lyrics, meiVerse.m_xmlId);
    Convert::colorFromMEI(lyrics, meiVerse);

    bool success = true;

    // If the verse has a syl with @con="u", add it to the lyrics to extend
    pugi::xpath_node extender = verseNode.select_node("./syl[@con='u']");
    if (extender) {
        m_lyricExtenders[chord->track()][no] = std::make_pair(lyrics, nullptr);
    }

    // @place
    if (meiVerse.HasPlace()) {
        lyrics->setPlacement(meiVerse.GetPlace()
                             == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        lyrics->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // Aggregate the syllable into line blocks
    Convert::textWithSmufl textBlocks;
    pugi::xpath_node_set elements = verseNode.select_nodes("./syl");

    // If we have more than one syl we assume to have elision
    ElisionType elision = (elements.size() > 1) ? ElisionFirst : ElisionNone;
    size_t sylCount = 0;

    for (pugi::xpath_node xpathNode : elements) {
        if (sylCount > 0) {
            textBlocks.push_back(std::make_pair(true, u"\uE551"));
        }

        success = success && this->readSyl(xpathNode.node(), lyrics, textBlocks, elision);
        sylCount++;
        elision = (sylCount == elements.size() - 1) ? ElisionLast : ElisionMiddle;
    }

    String syllable;
    Convert::textFromMEI(syllable, textBlocks);

    lyrics->setXmlText(syllable);
    lyrics->setNo(no);
    lyrics->initTextStyleType(lyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD, /*preserveDifferent*/ true);
    lyrics->setTrack(chord->track());
    chord->add(lyrics);

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
        if (elementName == "arpeg") {
            success = success && this->readArpeg(xpathNode.node(), measure);
        } else if (elementName == "breath") {
            success = success && this->readBreath(xpathNode.node(), measure);
        } else if (elementName == "caesura") {
            success = success && this->readCaesura(xpathNode.node(), measure);
        } else if (elementName == "dir") {
            success = success && this->readDir(xpathNode.node(), measure);
        } else if (elementName == "dynam") {
            success = success && this->readDynam(xpathNode.node(), measure);
        } else if (elementName == "fermata") {
            success = success && this->readFermata(xpathNode.node(), measure);
        } else if (elementName == "fing") {
            success = success && this->readFing(xpathNode.node(), measure);
        } else if (elementName == "hairpin") {
            success = success && this->readHairpin(xpathNode.node(), measure);
        } else if (elementName == "harm") {
            if (xpathNode.node().select_node("./fb")) {
                success = success && this->readFb(xpathNode.node(), measure);
            } else {
                success = success && this->readHarm(xpathNode.node(), measure);
            }
        } else if (elementName == "harpPedal") {
            success = success && this->readHarpPedal(xpathNode.node(), measure);
        } else if (elementName == "lv") {
            success = success && this->readLv(xpathNode.node(), measure);
        } else if (elementName == "mordent") {
            success = success && this->readMordent(xpathNode.node(), measure);
        } else if (elementName == "octave") {
            success = success && this->readOctave(xpathNode.node(), measure);
        } else if (elementName == "ornam") {
            success = success && this->readOrnam(xpathNode.node(), measure);
        } else if (elementName == "pedal") {
            success = success && this->readPedal(xpathNode.node(), measure);
        } else if (elementName == "reh") {
            success = success && this->readReh(xpathNode.node(), measure);
        } else if (elementName == "repeatMark") {
            success = success && this->readRepeatMark(xpathNode.node(), measure);
        } else if (elementName == "slur") {
            success = success && this->readSlur(xpathNode.node(), measure);
        } else if (elementName == "tempo") {
            success = success && this->readTempo(xpathNode.node(), measure);
        } else if (elementName == "tie") {
            success = success && this->readTie(xpathNode.node(), measure);
        } else if (elementName == "trill") {
            success = success && this->readTrill(xpathNode.node(), measure);
        } else if (elementName == "turn") {
            success = success && this->readTurn(xpathNode.node(), measure);
        }
    }
    return success;
}

/**
 * Read a arpeg.
 */

bool MeiImporter::readArpeg(pugi::xml_node arpegNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Arpeg meiArpeg;
    meiArpeg.Read(arpegNode);

    Arpeggio* arpeggio = static_cast<Arpeggio*>(this->addToChordRest(meiArpeg, measure));
    if (!arpeggio) {
        // Warning message given in MeiImporter::addToChordRest
        return true;
    }

    Convert::arpegFromMEI(arpeggio, meiArpeg, warning);

    if (meiArpeg.HasPlist()) {
        // Add the Arpeggio to the open arpeggio map, which will handle ties differently as appropriate
        m_openArpegMap[arpeggio] = arpegNode;
    }

    return true;
}

/**
 * Read a breath.
 */

bool MeiImporter::readBreath(pugi::xml_node breathNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Breath meiBreath;
    meiBreath.Read(breathNode);

    Breath* breath = static_cast<Breath*>(this->addAnnotation(meiBreath, measure));
    if (!breath) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    Convert::breathFromMEI(breath, meiBreath, warning);

    return true;
}

/**
 * Read a caesura (
 */

bool MeiImporter::readCaesura(pugi::xml_node caesuraNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Caesura meiCaesura;
    meiCaesura.Read(caesuraNode);

    Breath* breath = static_cast<Breath*>(this->addAnnotation(meiCaesura, measure));
    if (!breath) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    Convert::caesuraFromMEI(breath, meiCaesura, warning);

    return true;
}

/**
 * Read a dir.
 */

bool MeiImporter::readDir(pugi::xml_node dirNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Dir meiDir;
    meiDir.Read(dirNode);

    StringList meiLines;
    size_t meiLine = 0;
    this->readLines(dirNode, meiLines, meiLine);

    if (Convert::isDirWithExt(meiDir)) {
        TextLineBase* textLineBase = static_cast<TextLineBase*>(this->addSpanner(meiDir, measure, dirNode));
        if (!textLineBase) {
            // Warning message given in MeiImporter::addSpanner
            return true;
        }
        Convert::dirFromMEI(textLineBase, meiLines, meiDir, warning);
    } else {
        TextBase* textBase = static_cast<TextBase*>(this->addAnnotation(meiDir, measure));
        if (!textBase) {
            // Warning message given in MeiImporter::addAnnotation
            return true;
        }
        Convert::dirFromMEI(textBase, meiLines, meiDir, warning);
    }

    return true;
}

/**
 * Read a dynam.
 */

bool MeiImporter::readDynam(pugi::xml_node dynamNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Dynam meiDynam;
    meiDynam.Read(dynamNode);

    Dynamic* dynamic = static_cast<Dynamic*>(this->addAnnotation(meiDynam, measure));
    if (!dynamic) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    StringList meiLines;
    size_t meiLine = 0;
    this->readLines(dynamNode, meiLines, meiLine);

    Convert::dynamFromMEI(dynamic, meiLines, meiDynam, warning);

    return true;
}

/**
 * Read a f (FiguredBassItem)
 */

bool MeiImporter::readF(pugi::xml_node fNode, engraving::FiguredBass* figuredBass)
{
    IF_ASSERT_FAILED(figuredBass) {
        return false;
    }

    bool warning;
    libmei::F meiF;
    meiF.Read(fNode);

    const int line = static_cast<int>(figuredBass->itemsCount());
    FiguredBassItem* figuredBassItem = figuredBass->createItem(line);
    this->readXmlId(figuredBassItem, meiF.m_xmlId);
    figuredBassItem->setTrack(figuredBass->track());
    figuredBassItem->setParent(figuredBass);

    StringList meiLines;
    size_t meiLine = 0;
    this->readLines(fNode, meiLines, meiLine);

    Convert::fFromMEI(figuredBassItem, meiLines, meiF, warning);

    figuredBass->appendItem(figuredBassItem);

    return true;
}

/**
 * Read a fb (FiguredBass).
 */

bool MeiImporter::readFb(pugi::xml_node harmNode, Measure* measure)
{
    // Already checked in MeiImporter::readControlEvents
    pugi::xml_node fbNode = harmNode.select_node("./fb").node();

    IF_ASSERT_FAILED(fbNode && measure) {
        return false;
    }

    bool warning;
    libmei::Harm meiHarm;
    meiHarm.Read(harmNode);
    // Add a custom label for MeiImporter::addAnnotation to create a FiguredBass
    meiHarm.SetLabel(MEI_FB_HARM);
    libmei::Fb meiFb;
    meiFb.Read(fbNode);

    FiguredBass* figuredBass = static_cast<FiguredBass*>(this->addAnnotation(meiHarm, measure));
    if (!figuredBass) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }
    // Needs to be registered by hand because we pass meiHarm to MeiImporter::addAnnotation
    this->readXmlId(figuredBass, meiFb.m_xmlId);

    Convert::fbFromMEI(figuredBass, meiHarm, meiFb, warning);

    bool success = true;

    pugi::xpath_node_set fs = fbNode.select_nodes("./f");
    for (pugi::xpath_node xpathNode : fs) {
        success = success && this->readF(xpathNode.node(), figuredBass);
    }

    return success;
}

/**
 * Read a fermata.
 */

bool MeiImporter::readFermata(pugi::xml_node fermataNode, Measure* measure)
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
            this->readXmlId(fermata, meiFermata.m_xmlId);
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
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    Convert::fermataFromMEI(fermata, meiFermata, warning);

    return true;
}

/**
 * Read a fing.
 */

bool MeiImporter::readFing(pugi::xml_node fingNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Fing meiFing;
    meiFing.Read(fingNode);

    Note* note = this->findStartNote(meiFing);
    if (!note) {
        // Warning message given in MeiImporter::findStartNote
        return true;
    }

    Fingering* fing = Factory::createFingering(note);
    m_uids->reg(fing, meiFing.m_xmlId);

    StringList meiLines;
    size_t meiLine = 0;
    this->readLines(fingNode, meiLines, meiLine);

    Convert::fingFromMEI(fing, meiLines, meiFing, warning);

    note->add(fing);

    return true;
}

/**
 * Read a hairpin.
 */

bool MeiImporter::readHairpin(pugi::xml_node hairpinNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Hairpin meiHairpin;
    meiHairpin.Read(hairpinNode);

    Hairpin* hairpin = static_cast<Hairpin*>(this->addSpanner(meiHairpin, measure, hairpinNode));
    if (!hairpin) {
        // Warning message given in MeiImporter::addSpanner
        return true;
    }

    Convert::hairpinFromMEI(hairpin, meiHairpin, warning);

    return true;
}

/**
 * Read a harm.
 * <harm> with <fb> are read by MeiImporter::readFb
 */

bool MeiImporter::readHarm(pugi::xml_node harmNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Harm meiHarm;
    meiHarm.Read(harmNode);

    Harmony* harmony = static_cast<Harmony*>(this->addAnnotation(meiHarm, measure));
    if (!harmony) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    StringList meiLines;
    size_t meiLine = 0;
    this->readLines(harmNode, meiLines, meiLine);

    Convert::harmFromMEI(harmony, meiLines, meiHarm, warning);

    return true;
}

/**
 * Read a harpPedal.
 */

bool MeiImporter::readHarpPedal(pugi::xml_node harpPedalNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::HarpPedal meiHarpPedal;
    meiHarpPedal.Read(harpPedalNode);

    HarpPedalDiagram* harpPedalDiagram = static_cast<HarpPedalDiagram*>(this->addAnnotation(meiHarpPedal, measure));
    if (!harpPedalDiagram) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    Convert::harpPedalFromMEI(harpPedalDiagram, meiHarpPedal, warning);

    return true;
}

/**
 * Read a instrDef (instrument definition).
 */

bool MeiImporter::readInstrDef(pugi::xml_node instrDefNode, Part* part)
{
    IF_ASSERT_FAILED(part) {
        return false;
    }

    libmei::InstrDef meiInstrDef;
    meiInstrDef.Read(instrDefNode);

    part->setMidiProgram(meiInstrDef.GetMidiInstrnum());

    return true;
}

/**
 * Read a lv.
 */

bool MeiImporter::readLv(pugi::xml_node lvNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Lv meiLv;
    meiLv.Read(lvNode);

    Note* note = this->findStartNote(meiLv);
    if (!note) {
        return true;
    }

    LaissezVib* lv = Factory::createLaissezVib(note);
    lv->setParent(note);
    note->score()->undoAddElement(lv);
    m_uids->reg(lv, meiLv.m_xmlId);

    Convert::lvFromMEI(lv, meiLv, warning);

    return true;
}

/**
 * Read a mordent.
 */

bool MeiImporter::readMordent(pugi::xml_node mordentNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Mordent meiMordent;
    meiMordent.Read(mordentNode);

    Ornament* ornament = static_cast<Ornament*>(this->addToChordRest(meiMordent, measure));
    if (!ornament) {
        // Warning message given in MeiImporter::addToChordRest
        return true;
    }

    Convert::OrnamStruct ornamSt = Convert::mordentFromMEI(ornament, meiMordent, warning);
    this->setOrnamentAccid(ornament, ornamSt);

    return true;
}

/**
 * Read a octave.
 */

bool MeiImporter::readOctave(pugi::xml_node octaveNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Octave meiOctave;
    meiOctave.Read(octaveNode);

    Ottava* ottava = static_cast<Ottava*>(this->addSpanner(meiOctave, measure, octaveNode));
    if (!ottava) {
        // Warning message given in MeiImporter::addSpanner
        return true;
    }

    Convert::octaveFromMEI(ottava, meiOctave, warning);

    return true;
}

/**
 * Read a ornam.
 */

bool MeiImporter::readOrnam(pugi::xml_node ornamNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Ornam meiOrnam;
    meiOrnam.Read(ornamNode);

    Ornament* ornament = static_cast<Ornament*>(this->addToChordRest(meiOrnam, measure));
    if (!ornament) {
        // Warning message given in MeiImporter::addToChordRest
        return true;
    }

    Convert::OrnamStruct ornamSt = Convert::ornamFromMEI(ornament, meiOrnam, warning);
    this->setOrnamentAccid(ornament, ornamSt);

    return true;
}

/**
 * Read a pedal.
 */

bool MeiImporter::readPedal(pugi::xml_node pedalNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Pedal meiPedal;
    meiPedal.Read(pedalNode);

    Pedal* pedal = static_cast<Pedal*>(this->addSpanner(meiPedal, measure, pedalNode));
    if (!pedal) {
        // Warning message given in MeiImporter::addSpanner
        return true;
    }

    Convert::pedalFromMEI(pedal, meiPedal, warning);

    return true;
}

/**
 * Read a reh.
 */

bool MeiImporter::readReh(pugi::xml_node rehNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    libmei::Reh meiReh;
    meiReh.Read(rehNode);

    RehearsalMark* rehearsalMark = static_cast<RehearsalMark*>(this->addAnnotation(meiReh, measure));
    if (!rehearsalMark) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }
    Convert::colorFromMEI(rehearsalMark, meiReh);

    StringList meiLines;
    this->readLinesWithSmufl(rehNode, meiLines);

    // text
    rehearsalMark->setXmlText(meiLines.join(u"\n"));

    return true;
}

/**
 * Read a repeatMark and create a Jump or a Marker as appropriate.
 * For Jump, default values for jumpTo, playUntil and continueAt are used.
 * Similarly, default value for label is used for Marker.
 */

bool MeiImporter::readRepeatMark(pugi::xml_node repeatMarkNode, Measure* measure)
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
    this->readXmlId(item, meiRepeatMark.m_xmlId);
    item->setTrack(0);
    measure->add(item);

    return true;
}

/**
 * Read a slur.
 */

bool MeiImporter::readSlur(pugi::xml_node slurNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Slur meiSlur;
    meiSlur.Read(slurNode);

    Slur* slur = static_cast<Slur*>(this->addSpanner(meiSlur, measure, slurNode));
    if (!slur) {
        // Warning message given in MeiImporter::addSpanner
        return true;
    }

    Convert::slurFromMEI(slur, meiSlur, warning);

    return true;
}

/**
 * Read a tempo.
 */

bool MeiImporter::readTempo(pugi::xml_node tempoNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Tempo meiTempo;
    meiTempo.Read(tempoNode);

    TempoText* tempoText = static_cast<TempoText*>(this->addAnnotation(meiTempo, measure));
    if (!tempoText) {
        // Warning message given in MeiImporter::addAnnotation
        return true;
    }

    StringList meiLines;
    this->readLinesWithSmufl(tempoNode, meiLines);

    Convert::tempoFromMEI(tempoText, meiLines, meiTempo, warning);

    return true;
}

/**
 * Read a tie.
 */

bool MeiImporter::readTie(pugi::xml_node tieNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Tie meiTie;
    meiTie.Read(tieNode);

    // We do not use addSpanner here because Tie object are added directly to the start and end Note objects
    Note* startNote = this->findStartNote(meiTie);
    if (!startNote) {
        // Here we could detect if it's a tied chord (for files not exported from MuseScore)
        // We would need a dedicated list and tie each note once the second chord has been found.
        return true;
    }

    Tie* tie = new Tie(m_score->dummy());
    this->readXmlId(tie, meiTie.m_xmlId);
    startNote->setTieFor(tie);
    tie->setStartNote(startNote);
    tie->setTrack(startNote->track());

    // Still add the Tie to the open Spanner map, which will handle ties differently as appropriate
    m_openSpannerMap[tie] = tieNode;

    Convert::tieFromMEI(tie, meiTie, warning);

    return true;
}

/**
 * Read a trill.
 */

bool MeiImporter::readTrill(pugi::xml_node trillNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Trill meiTrill;
    meiTrill.Read(trillNode);

    Ornament* ornament = static_cast<Ornament*>(this->addToChordRest(meiTrill, measure));
    if (!ornament) {
        // Warning message given in MeiImporter::addToChordRest
        return true;
    }

    if (meiTrill.HasEndid()) {
        Trill* trill = static_cast<Trill*>(this->addSpanner(meiTrill, measure, trillNode));
        if (trill) {
            // move ornament to spanner
            ornament->parentItem()->remove(ornament);
            trill->setOrnament(ornament);
            // @color
            Convert::colorlineFromMEI(trill, meiTrill);
        }
    }

    Convert::OrnamStruct ornamSt = Convert::trillFromMEI(ornament, meiTrill, warning);
    this->setOrnamentAccid(ornament, ornamSt);

    return true;
}

/**
 * Read a mordent.
 */

bool MeiImporter::readTurn(pugi::xml_node turnNode, Measure* measure)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool warning;
    libmei::Turn meiTurn;
    meiTurn.Read(turnNode);

    Ornament* ornament = static_cast<Ornament*>(this->addToChordRest(meiTurn, measure));
    if (!ornament) {
        // Warning message given in MeiImporter::addToChordRest
        return true;
    }

    Convert::OrnamStruct ornamSt = Convert::turnFromMEI(ornament, meiTurn, warning);
    this->setOrnamentAccid(ornament, ornamSt);

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

bool MeiImporter::readStemsAtt(Chord* chord, libmei::AttStems& stemsAtt)
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

    pugi::xpath_node_set nodesWithPlist = scoreNode.select_nodes("//@plist");

    for (pugi::xpath_node elementXpathNode : nodesWithPlist) {
        std::string plist = elementXpathNode.attribute().value();
        std::istringstream iss(plist);
        std::string id;
        while (std::getline(iss, id, ' ')) {
            if (id.size() < 1 || id.at(0) != '#') {
                continue;
            }
            m_plistValueChordRests[id.erase(0, 1)] = nullptr;
        }
    }

    return true;
}

/**
 * Create a mapping for <staff> `@n` and <layer> `@n`.
 * Useful only when reading MEI files where the sequence of `@n` is not starting from 1 or not sequential.
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
        this->addTextToTitleFrame(vBox, textCreators.join(u"\n"), TextStyleType::LYRICIST);
    }

    if (vBox) {
        vBox->setTick(Fraction(0, 1));
        m_score->measures()->append(vBox);
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

        StringList lines;
        size_t line = 0;
        this->readLines(labelNode, lines, line);
        part->setLongName(lines.join(u"\n"));

        pugi::xml_node labelAbbrNode = labelNode.select_node("./following-sibling::labelAbbr").node();
        if (labelAbbrNode) {
            StringList abbrLines;
            size_t abbrLine = 0;
            this->readLines(labelAbbrNode, abbrLines, abbrLine);
            part->setShortName(abbrLines.join(u"\n"));
        }

        pugi::xml_node instrDefNode = labelNode.select_node("./following-sibling::instrDef").node();
        readInstrDef(instrDefNode, part);

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
        if (staffSt.color.isValid()) {
            staff->staffType(Fraction(0, 1))->setColor(staffSt.color);
        }
        staff->staffType(Fraction(0, 1))->setInvisible(staffSt.invisible);
        staff->staffType(Fraction(0, 1))->setUserMag(staffSt.scale / 100);
        part->instrument()->setTranspose(staffSt.interval);

        m_score->appendStaff(staff);
    }

    int staffSpan = 0;
    size_t idx = 0;
    // Start from the top (default) staffGrp
    pugi::xml_node staffGrpNode = scoreDefNode.select_node("./staffGrp").node();

    bool success = this->readStaffGrps(staffGrpNode, staffSpan, 0, idx);

    // Update parts instruments IDs for playback
    for (const Part* part : m_score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }

    return success;
}

//---------------------------------------------------------
// content creation helper methods
//---------------------------------------------------------

/**
 * Add an end barline to a measure.
 */

void MeiImporter::addEndBarLineToMeasure(Measure* measure, BarLineType barLineType)
{
    IF_ASSERT_FAILED(measure) {
        return;
    }

    // Here we could check if this is the last measure and not add END because MuseScore sets it automatically
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

void MeiImporter::addLayoutBreakToMeasure(Measure* measure, LayoutBreakType layoutBreakType)
{
    if (!measure) {
        LOGD() << "MeiImporter::addLayoutBreakToMeasure no measure to add the break";
        return;
    }

    LayoutBreak* layoutBreak = Factory::createLayoutBreak(measure);
    layoutBreak->setLayoutBreakType(layoutBreakType);
    layoutBreak->setTrack(0);
    measure->add(layoutBreak);
}

/**
 *  Add some text with a textStyleType to a vBox.
 */

void MeiImporter::addTextToTitleFrame(VBox*& vBox, const String& str, TextStyleType textStyleType)
{
    if (!str.isEmpty()) {
        if (vBox == nullptr) {
            vBox = Factory::createTitleVBox(m_score->dummy()->system());
        }
        Text* text = Factory::createText(vBox, textStyleType);
        text->setPlainText(str);
        vBox->add(text);
    }
}

/**
 * Add the end element / tick / track for spanner ends.
 * This is happening at the end of the import because we need to make sure spanning ends have been read.
 * Different handing for ties since these are attached to the notes
 * Additional handling for ottava to adjust end position tricker pitch offset calculation
 */

void MeiImporter::addSpannerEnds()
{
    for (auto spannerMapEntry : m_openSpannerMap) {
        // Ties are added directly to the notes
        if (spannerMapEntry.first->isTie()) {
            Note* endNote = this->findEndNote(spannerMapEntry.second);
            if (!endNote) {
                continue;
            }
            Tie* tie = toTie(spannerMapEntry.first);
            endNote->setTieBack(tie);
            tie->setEndNote(endNote);
            // All other Spanners
        } else if (spannerMapEntry.first->startCR()) {
            ChordRest* chordRest = this->findEnd(spannerMapEntry.second, spannerMapEntry.first->startCR());
            if (!chordRest) {
                continue;
            }
            spannerMapEntry.first->setTick2(chordRest->tick());
            spannerMapEntry.first->setEndElement(chordRest);
            spannerMapEntry.first->setTrack2(chordRest->track());
            if (spannerMapEntry.first->isOttava() || spannerMapEntry.first->isTrill()) {
                // Set the tick2 to include the duration of the ChordRest
                spannerMapEntry.first->setTick2(chordRest->tick() + chordRest->ticks());
                // Special handling of ottavas
                if (spannerMapEntry.first->isOttava()) {
                    Ottava* ottava = toOttava(spannerMapEntry.first);
                    // Make the staff fill the pitch offsets accordingly since we use Note::ppitch in export
                    ottava->staff()->updateOttava();
                }
            }
        }
    }
    for (auto arpegMapEntry : m_openArpegMap) {
        std::list plistChordRests = findPlistChordRests(arpegMapEntry.second);
        // Go through the list of chord rest and check if they are on a staff below
        for (auto chordRest : plistChordRests) {
            Arpeggio* arpeggio = arpegMapEntry.first;
            int span = static_cast<int>(chordRest->track() - arpeggio->track()) + 1;
            // Adjust the span if it is currently smaller
            if (arpeggio->span() < span) {
                arpeggio->setSpan(span);
            }
        }
    }
}
