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

#include "meiexporter.h"

#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/jump.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

#include "log.h"

#include "meiconverter.h"

#include "thirdparty/libmei/cmn.h"
#include "thirdparty/libmei/harmony.h"
#include "thirdparty/libmei/shared.h"

using namespace mu::iex::mei;
using namespace mu::engraving;

// Number of spaces for the XML indentation. Set to 0 for tabs
#define MEI_INDENT 3

/**
 * Write the Score to the destination file.
 * Return false on error.
 */

bool MeiExporter::write(QIODevice& destinationDevice)
{
    // Still using QTextStream since we have a QIODevice
    QTextStream out(&destinationDevice);

    m_hasSections = false;

    m_sectionCounter = 0;
    m_endingCounter = 0;
    m_measureCounter = 0;
    m_staffCounter = 0;
    m_layerCounter = 0;
    m_measureCounterFor.resize(UNSPECIFIED_M + 1);
    this->resetMeasureIDs();
    m_layerCounterFor.resize(UNSPECIFIED_L + 1);
    this->resetLayerIDs();

    try {
        pugi::xml_document meiDoc;

        pugi::xml_node decl = meiDoc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "UTF-8";

        // schema processing instruction
        std::string schema = "https://music-encoding.org/schema/dev/mei-basic.rng";
        decl = meiDoc.append_child(pugi::node_declaration);
        decl.set_name("xml-model");
        decl.append_attribute("href") = schema.c_str();
        decl.append_attribute("type") = "application/xml";
        decl.append_attribute("schematypens") = "http://relaxng.org/ns/structure/1.0";

        decl = meiDoc.append_child(pugi::node_declaration);
        decl.set_name("xml-model");
        decl.append_attribute("href") = schema.c_str();
        decl.append_attribute("type") = "application/xml";
        decl.append_attribute("schematypens") = "http://purl.oclc.org/dsdl/schematron";

        m_mei = meiDoc.append_child("mei");
        m_mei.append_attribute("xmlns") = "http://www.music-encoding.org/ns/mei";

        libmei::AttConverter converter;
        libmei::meiVersion_MEIVERSION meiVersion = libmei::meiVersion_MEIVERSION_5_0_0_devplusbasic;
        m_mei.append_attribute("meiversion") = (converter.MeiVersionMeiversionToStr(meiVersion)).c_str();

        this->writeHeader();

        this->writeScore();

        // Currently not used. To be enabled for unfolding MuseScore Jumps into `@jumpto` MEI attribute if it becomes available on MEI repeatMark
        // this->addJumpToRepeatMarks();

        unsigned int output_flags = pugi::format_default;

        // Tabulation of MEI_INDENT * spaces (tabs if 0)
        std::string indent = MEI_INDENT ? std::string(MEI_INDENT, ' ') : "\t";
        std::stringstream strStream;
        meiDoc.save(strStream, indent.c_str(), output_flags);
        out << String::fromStdString(strStream.str());
    }
    catch (char* str) {
        // Do something with the error message
        return false;
    }

    out.flush();
    return true;
}

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

/**
 * Write the MEI header.
 * Look for the meiHead custom meta tag (given when importing an MEI file into MuseScore).
 * Otherwise, generate the header using the common meta tags.
 */

bool MeiExporter::writeHeader()
{
    String headStr = m_score->metaTag(u"meiHead");
    if (headStr.size() > 0) {
        pugi::xml_document docHead;
        docHead.load_string(headStr.toStdString().c_str());
        m_mei.append_copy(docHead.first_child());
    } else {
        // create header
        pugi::xml_node meiHead = m_mei.append_child("meiHead");
        pugi::xml_node fileDesc = meiHead.append_child("fileDesc");
        pugi::xml_node titleStmt = fileDesc.append_child("titleStmt");
        pugi::xml_node title = titleStmt.append_child("title");
        if (!m_score->metaTag(u"workTitle").isEmpty()) {
            title.text().set(m_score->metaTag(u"workTitle").toStdString().c_str());
        }

        pugi::xml_node respStmt;
        StringList persNames;
        // the creator types commonly found in MusicXML
        persNames << u"arranger" << u"composer" << u"lyricist" << u"translator";
        for (String tagName : persNames) {
            String persName = m_score->metaTag(tagName);
            if (!persName.isEmpty()) {
                if (!respStmt) {
                    respStmt = titleStmt.append_child("respStmt");
                }
                pugi::xml_node persNameNode = respStmt.append_child("persName");
                persNameNode.text().set(persName.toStdString().c_str());
                persNameNode.append_attribute("role") = tagName.toStdString().c_str();
            }
        }

        pugi::xml_node pubStmt = fileDesc.append_child("pubStmt");
        pugi::xml_node date = pubStmt.append_child("date");

        // date
        String dateStr = DateTime::currentDateTime().toString();
        date.append_attribute("isodate") = dateStr.toStdString().c_str();

        if (!m_score->metaTag(u"copyright").isEmpty()) {
            pugi::xml_node availability = pubStmt.append_child("availability");
            pugi::xml_node distributor = availability.append_child("distributor");
            distributor.text().set(m_score->metaTag(u"copyright").toStdString().c_str());
        }
    }

    return true;
}

/**
 * Write the MEI score.
 * First write the initial scoreDef, and then loop through pages and systems.
 * Layout information (sb and pb) is written only when the corresponding output option is selected.
 */

bool MeiExporter::writeScore()
{
    pugi::xml_node music = m_mei.append_child("music");
    m_currentNode = music.append_child("body");
    m_currentNode = m_currentNode.append_child("mdiv");
    m_currentNode = m_currentNode.append_child("score");

    this->writeScoreDef();

    m_currentNode = m_currentNode.append_child();
    libmei::Section meiSection;
    meiSection.Write(m_currentNode, this->getSectionXmlId());

    int measureN = 0;
    bool isFirst = true;
    bool wasPreviousIrregular = true;

    bool exportLayout = configuration()->meiExportLayout();
    bool pageBreak = false;
    bool lineBreak = false;
    //bool sectionBreak = false; // not implemented, but we should had additional sections elements where m_hasSections is true

    for (const Page* page : m_score->pages()) {
        bool firstSystem = true;
        if (exportLayout) {
            libmei::Pb pb;
            if (pageBreak) {
                pb.SetType(BREAK_TYPE);
            }
            pb.Write(m_currentNode.append_child());
        }
        for (const System* system : page->systems()) {
            if (exportLayout && !firstSystem) {
                libmei::Sb sb;
                if (lineBreak) {
                    sb.SetType(BREAK_TYPE);
                }
                sb.Write(m_currentNode.append_child());
            }
            for (const MeasureBase* mBase : system->measures()) {
                if (mBase->isMeasure()) {
                    const Measure* measure = static_cast<const Measure*>(mBase);
                    this->writeEnding(measure);
                    this->writeMeasure(measure, measureN, isFirst, wasPreviousIrregular);
                    this->writeEndingEnd(measure);
                    firstSystem = false;
                }
                lineBreak = mBase->lineBreak();
                pageBreak = mBase->pageBreak();
                //sectionBreak = mBase->sectionBreak(); // see comment above
            }
        }
    }

    // non critical assert
    assert(this->isCurrentNode(libmei::Section()));
    m_currentNode = m_currentNode.parent();

    return true;
}

/**
 * Write the initial score definition.
 */

bool MeiExporter::writeScoreDef()
{
    m_currentNode = m_currentNode.append_child("scoreDef");

    // If we have a VBox on the first measure, assume it to be a title frame and use it for the pgHead
    MeasureBase* mBase = m_score->measures()->first();
    if (mBase->isVBox()) {
        this->writePgHead(toVBox(mBase));
    }

    m_currentNode = m_currentNode.append_child("staffGrp");

    // Number of staffGrp closing at each staff
    std::vector<int> staffGrpEnds(m_score->staves().size(), 0);

    const Measure* measure = nullptr;
    for (MeasureBase* mBase = m_score->measures()->first(); mBase != nullptr; mBase = mBase->next()) {
        if (!measure && mBase->isMeasure()) {
            // the first actuall measure we are going built the scoreDef from
            measure = static_cast<const Measure*>(mBase);
        }
        // Also check here if we have multiple sections in the score
        if (mBase->sectionBreak()) {
            m_hasSections = true;
            break;
        }
    }

    for (Part* part : m_score->parts()) {
        // For parts with more than one staff, write the label in the staffGrp
        Part* staffGrpPart = (part->nstaves() > 1) ? part : nullptr;
        // Otherwise write the label in the staffDef
        bool isStaffDefPart = (part->nstaves() == 1) ? true : false;
        for (Staff* staff : part->staves()) {
            this->writeStaffGrpStart(staff, staffGrpEnds, staffGrpPart);
            this->writeStaffDef(staff, measure, part, isStaffDefPart);
            this->writeStaffGrpEnd(staff, staffGrpEnds);
            // We pass the part only for the first staff of the staffGrp, so set it to null after
            staffGrpPart = nullptr;
        }
    }

    m_currentNode = m_currentNode.parent();
    m_currentNode = m_currentNode.parent();

    return true;
}

/**
 * Write the page header.
 * Uses the first MuseScore vBox.
 */

bool MeiExporter::writePgHead(const VBox* vBox)
{
    IF_ASSERT_FAILED(vBox) {
        return false;
    }

    m_currentNode = m_currentNode.append_child();

    libmei::PgHead pgHead;
    pgHead.Write(m_currentNode);

    std::list<std::pair<libmei::Rend, String> > cells[CellCount];

    // For each text in the frame create an MEI Rend
    // Convert::textToMEI set the size of the Rend looking at the TextStyleType
    // It also places them (cell) accordingly
    for (const EngravingItem* element : vBox->el()) {
        if (element->isText()) {
            const Text* text = toText(element);
            auto [meiRend, cell, rendText] = Convert::textToMEI(text);
            cells[cell].push_back(std::make_pair(meiRend, rendText));
        }
    }

    // Each cell is now a list of pairs of Rend and the corresponding text content
    // The text content is plain text but can be mutliple lines separated with a "\n"
    for (int cell = TopLeft; cell < CellCount; cell++) {
        if (cells[cell].empty()) {
            continue;
        }
        // if the cell is not empty, create a cell Rend an place it accordingly
        pugi::xml_node cellNode = m_currentNode.append_child();
        libmei::Rend meiRendCell;
        if (cell < 3) {
            meiRendCell.SetValign(libmei::VERTICALALIGNMENT_top);
        } else if (cell < 6) {
            meiRendCell.SetValign(libmei::VERTICALALIGNMENT_middle);
        } else {
            meiRendCell.SetValign(libmei::VERTICALALIGNMENT_bottom);
        }
        if ((cell % 3) == 0) {
            meiRendCell.SetHalign(libmei::HORIZONTALALIGNMENT_left);
        } else if ((cell % 3) == 1) {
            meiRendCell.SetHalign(libmei::HORIZONTALALIGNMENT_center);
        } else {
            meiRendCell.SetHalign(libmei::HORIZONTALALIGNMENT_right);
        }
        meiRendCell.Write(cellNode);
        // In the cell, write each Rend
        bool isFirst = true;
        for (auto& pair : cells[cell]) {
            // If we have more than one Rend in the cell-list, add an <lb/>
            if (!isFirst) {
                cellNode.append_child("lb");
            }
            pugi::xml_node rendNode = cellNode.append_child();
            pair.first.Write(rendNode);
            // Each Rend (as plain text) can itself be multi-line, split it with <lb/>
            StringList lines = pair.second.split(u"\n");
            this->writeLines(rendNode, lines);
            isFirst = false;
        }
    }

    m_currentNode = m_currentNode.parent();

    return true;
}

/**
 * Write a list of string as lines separated with line breaks
 */

bool MeiExporter::writeLines(pugi::xml_node node, const StringList& lines)
{
    if (lines.size() > 0) {
        node.text().set(lines[0].toStdString().c_str());
    }
    for (size_t index = 1; index < lines.size(); index++) {
        node.append_child("lb");
        pugi::xml_node textNode = node.append_child(pugi::node_pcdata);
        textNode.text() = lines[index].toStdString().c_str();
    }
    return true;
}

/**
 * Write a list of string as lines separated with line breaks.
 * For each line group the SMuFL symbols into <rend> with a @glyph.num
 * Convert line by line MuseScore plain text (without <sym>) into text segmented text blocks
 */

bool MeiExporter::writeLinesWithSMuFL(pugi::xml_node node, const StringList& lines)
{
    bool isFirst = true;
    for (size_t index = 0; index < lines.size(); index++) {
        if (!isFirst) {
            node.append_child("lb");
        }

        Convert::textWithSmufl lineBlocks;
        Convert::textToMEI(lineBlocks, lines.at(index));

        bool isFirstBlock = true;
        for (auto& block : lineBlocks) {
            if (block.first) {
                pugi::xml_node rendText = node.append_child();
                libmei::Rend meiRend;
                meiRend.SetGlyphAuth("smufl");
                meiRend.Write(rendText);
                rendText.text() = block.second.toStdString().c_str();
            } else {
                if (isFirst && isFirstBlock) {
                    node.text().set(block.second.toStdString().c_str());
                } else {
                    pugi::xml_node textNode = node.append_child(pugi::node_pcdata);
                    textNode.text() = block.second.toStdString().c_str();
                }
            }
            isFirstBlock = false;
        }
        isFirst = false;
    }
    return true;
}

/**
 * Write a score definition change.
 * The scoreDef is preprended to the m_currentNode that currently holds the measure.
 * The changes are encoded in the scoreDef element if that is possible.
 * Otherwise, staffGrp with staffDef are encoded (e.g., when changing a key signature with transposing instruments)
 */

bool MeiExporter::writeScoreDefChange()
{
    if (!m_timeSig && !m_keySig) {
        return true;
    }

    // First check that the timesig change is the same at all staves
    const TimeSig* scoreDefTimeSig = nullptr;
    if (m_timeSig) {
        for (size_t staff = 0; staff < m_score->nstaves(); staff++) {
            const TimeSig* current = dynamic_cast<const TimeSig*>(m_timeSig->element(staff2track(staff)));
            if (scoreDefTimeSig) {
                // Compare the current and the previous one, stop if they are different
                if (!current || (*current) != (*scoreDefTimeSig)) {
                    scoreDefTimeSig = nullptr;
                    break;
                }
            }
            scoreDefTimeSig = current;
            // The first staff had none, that will be different than any other one in the segment anyway
            if (!scoreDefTimeSig) {
                break;
            }
        }
    }

    // Same for keysig changes (likely to be different with transposing instruments)
    const KeySig* scoreDefKeySig = nullptr;
    if (m_keySig) {
        for (size_t staff = 0; staff < m_score->nstaves(); staff++) {
            const KeySig* current = dynamic_cast<const KeySig*>(m_keySig->element(staff2track(staff)));
            if (scoreDefKeySig) {
                if (!current || (current->key() != scoreDefKeySig->key())) {
                    scoreDefKeySig = nullptr;
                    break;
                }
            }
            scoreDefKeySig = current;
            // The first staff had none, same as for time sig
            if (!scoreDefKeySig) {
                break;
            }
        }
    }

    // m_currentNode is the last measure and we need to prepend the scoreDef
    pugi::xml_node scoreDefNode = m_currentNode.parent().insert_child_before("scoreDef", m_currentNode);
    libmei::ScoreDef meiScoreDef;

    // Single timesig change
    if (scoreDefTimeSig) {
        libmei::StaffDef timeSigDef = Convert::meterToMEI(scoreDefTimeSig->sig(), scoreDefTimeSig->timeSigType());
        meiScoreDef.SetMeterSym(timeSigDef.GetMeterSym());
        meiScoreDef.SetMeterUnit(timeSigDef.GetMeterUnit());
        meiScoreDef.SetMeterCount(timeSigDef.GetMeterCount());
    }
    // Single keysig change
    if (scoreDefKeySig) {
        //libmei::StaffDef keySigDef = Convert::keyToMEI(scoreDefKeySig->sig());
        meiScoreDef.SetKeysig(Convert::keyToMEI(scoreDefKeySig->key()));
    }
    // Otherwise, add staffGrp/staffDef
    if ((!scoreDefTimeSig && m_timeSig) || (!scoreDefKeySig && m_keySig)) {
        pugi::xml_node staffGrpNode = scoreDefNode.append_child("staffGrp");
        for (size_t staff = 0; staff < m_score->nstaves(); staff++) {
            pugi::xml_node staffDefNode = staffGrpNode.append_child();
            libmei::StaffDef meiStaffDef;
            if (!scoreDefTimeSig && m_timeSig) {
                const TimeSig* timeSig = dynamic_cast<const TimeSig*>(m_timeSig->element(staff2track(staff)));
                if (timeSig) {
                    meiStaffDef = Convert::meterToMEI(timeSig->sig(), timeSig->timeSigType());
                }
            }
            if (!scoreDefKeySig && m_keySig) {
                const KeySig* keySig = dynamic_cast<const KeySig*>(m_keySig->element(staff2track(staff)));
                if (keySig) {
                    meiStaffDef.SetKeysig(Convert::keyToMEI(keySig->key()));
                }
            }
            meiStaffDef.SetN(static_cast<int>(staff + 1));
            meiStaffDef.Write(staffDefNode);
        }
    }

    meiScoreDef.Write(scoreDefNode);

    return true;
}

/**
 * Write a staffGrp (opening).
 * Increments in ends the ending position of the staffGrp to be closed by MeiExporter::writeStaffGrpEnd.
 */

bool MeiExporter::writeStaffGrpStart(const Staff* staff, std::vector<int>& ends, const Part* staffGrpPart)
{
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    for (size_t j = 0; j < staff->bracketLevels() + 1; j++) {
        if (staff->bracketType(j) != BracketType::NO_BRACKET) {
            libmei::StaffGrp meiStaffGrp = Convert::bracketToMEI(staff->bracketType(j), staff->barLineSpan());
            // mark at which staff we will need to close the staffGrp
            int end = static_cast<int>(staff->idx() + staff->bracketSpan(j)) - 1;
            // Something is wrong, maybe a staff was delete in the MuseScore file?
            if (end >= static_cast<int>(ends.size())) {
                continue;
            }
            ends.at(end)++;
            //
            m_currentNode = m_currentNode.append_child();
            meiStaffGrp.Write(m_currentNode);
            // If we have a part and reached the latest level, write the label and labelAbbr
            if (staffGrpPart && j == staff->bracketLevels()) {
                this->writeLabel(m_currentNode, staffGrpPart);
            }
        }
    }
    return true;
}

/**
 * Write a staffGrp (closing).
 * Looks in ends how many staffGrp levels need to be closed for the corresponding staffIdx.
 */

bool MeiExporter::writeStaffGrpEnd(const Staff* staff, std::vector<int>& ends)
{
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    size_t idx = staff->idx();
    for (int i = 0; i < ends.at(idx); i++) {
        m_currentNode = m_currentNode.parent();
    }

    return true;
}

/**
 * Write the initial staff definitions.
 */

bool MeiExporter::writeStaffDef(const Staff* staff, const Measure* measure, const Part* part, bool isPart)
{
    IF_ASSERT_FAILED(staff && measure && part) {
        return false;
    }

    libmei::StaffDef meiStaffDef = Convert::staffToMEI(staff);
    pugi::xml_node staffDefNode = m_currentNode.append_child();

    if (isPart) {
        this->writeLabel(staffDefNode, part);
    }

    if (measure) {
        Fraction tick = measure->tick();

        track_idx_t startTrack = staff2track(staff->idx());
        track_idx_t endTrack = startTrack + VOICES;

        // clef
        Segment* clefSeg = measure->findSegment(SegmentType::HeaderClef, tick);
        if (clefSeg) {
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                Clef* clef = static_cast<Clef*>(clefSeg->element(track));
                if (clef) {
                    libmei::Clef meiClef = Convert::clefToMEI(clef->clefType());
                    pugi::xml_node clefNode = staffDefNode.append_child();
                    meiClef.Write(clefNode);
                    break;
                }
            }
        }
        // time signature
        Segment* timeSigSeg = measure->findSegment(SegmentType::TimeSig, tick);
        if (timeSigSeg) {
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                TimeSig* timeSig = static_cast<TimeSig*>(timeSigSeg->element(track));
                if (timeSig) {
                    libmei::StaffDef timeSigDef = Convert::meterToMEI(timeSig->sig(), timeSig->timeSigType());
                    meiStaffDef.SetMeterSym(timeSigDef.GetMeterSym());
                    meiStaffDef.SetMeterUnit(timeSigDef.GetMeterUnit());
                    meiStaffDef.SetMeterCount(timeSigDef.GetMeterCount());
                    break;
                }
            }
        }
        // key signature
        Segment* keySigSeg = measure->findSegment(SegmentType::KeySig, tick);
        if (keySigSeg) {
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                KeySig* keySig = static_cast<KeySig*>(keySigSeg->element(track));
                // For the initial staffDef we do not write @key.sig="0"
                if (keySig && keySig->key() != Key::C) {
                    meiStaffDef.SetKeysig(Convert::keyToMEI(keySig->key()));
                    break;
                }
            }
        }
    }

    meiStaffDef.Write(staffDefNode);

    return true;
}

/**
 * Write label and label abbreviations.
 */

bool MeiExporter::writeLabel(pugi::xml_node node, const Part* part)
{
    IF_ASSERT_FAILED(part) {
        return false;
    }

    const Instrument* instrument = part->instrument();
    if (instrument && instrument->longNames().size() > 0) {
        libmei::Label meiLabel;
        pugi::xml_node labelNode = node.append_child();
        meiLabel.Write(labelNode);
        labelNode.text().set(instrument->nameAsPlainText().toStdString().c_str());
    }
    if (instrument && instrument->shortNames().size() > 0) {
        libmei::LabelAbbr meiLabelAbbr;
        pugi::xml_node labelAbbrNode = node.append_child();
        meiLabelAbbr.Write(labelAbbrNode);
        labelAbbrNode.text().set(instrument->abbreviatureAsPlainText().toStdString().c_str());
    }

    return true;
}

/**
 * Write an ending (opening).
 * Performs a lookup of voltas spanning the measure with MeiExporter::findVoltasInMeasure
 */

bool MeiExporter::writeEnding(const Measure* measure)
{
    std::list<const Volta*> voltas = this->findVoltasInMeasure(measure);
    auto voltaIter = std::find_if(voltas.begin(), voltas.end(), [measure](const Volta* volta) { return volta->startMeasure() == measure; });

    if (voltaIter != voltas.end()) {
        libmei::Ending meiEnding = Convert::endingToMEI(*voltaIter);
        m_currentNode = m_currentNode.append_child();
        meiEnding.Write(m_currentNode, this->getEndingXmlId());
    }
    return true;
}

/**
 * Write an ending (closing).
 * Performs a lookup of voltas spanning the measure with MeiExporter::findVoltasInMeasure
 */

bool MeiExporter::writeEndingEnd(const Measure* measure)
{
    std::list<const Volta*> voltas = this->findVoltasInMeasure(measure);
    auto voltaIter = std::find_if(voltas.begin(), voltas.end(), [measure](const Volta* volta) { return volta->endMeasure() == measure; });

    if (voltaIter != voltas.end()) {
        // non critical assert
        assert(this->isCurrentNode(libmei::Ending()));
        m_currentNode = m_currentNode.parent();
    }
    return true;
}

/**
 * Write a measure and its content.
 * Write the staves and the control events.
 * Prepends a scoreDef change if a changing key signature or time signature is encountered in the content.
 */

bool MeiExporter::writeMeasure(const Measure* measure, int& measureN, bool& isFirst, bool& wasPreviousIrregular)
{
    IF_ASSERT_FAILED(measure) {
        return false;
    }

    bool success = true;

    libmei::Measure meiMeasure = Convert::measureToMEI(measure, measureN, wasPreviousIrregular);
    m_currentNode = m_currentNode.append_child();
    meiMeasure.Write(m_currentNode, this->getMeasureXmlId());

    // Reset keySig and timeSig change
    m_keySig = nullptr;
    m_timeSig = nullptr;

    for (Staff* staff : m_score->staves()) {
        this->writeStaff(staff, measure);
    }

    for (EngravingItem* item : measure->el()) {
        switch (item->type()) {
        case ElementType::JUMP: success = success && this->writeRepeatMark(toJump(item), measure);
            break;
        case ElementType::MARKER: success = success && this->writeRepeatMark(toMarker(item), measure);
            break;
        default: break;
        }
    }

    for (auto controlEvent : m_startingControlEventMap) {
        if (controlEvent.first->isBreath()) {
            success = success && this->writeBreath(dynamic_cast<const Breath*>(controlEvent.first), controlEvent.second);
        } else if (controlEvent.first->isDynamic()) {
            success = success && this->writeDynam(dynamic_cast<const Dynamic*>(controlEvent.first), controlEvent.second);
        } else if (controlEvent.first->isFermata()) {
            success = success && this->writeFermata(dynamic_cast<const Fermata*>(controlEvent.first), controlEvent.second);
        } else if (controlEvent.first->isHarmony()) {
            success = success && this->writeHarm(dynamic_cast<const Harmony*>(controlEvent.first), controlEvent.second);
        } else if (controlEvent.first->isSlur()) {
            success = success && this->writeSlur(dynamic_cast<const Slur*>(controlEvent.first), controlEvent.second);
        } else if (controlEvent.first->isTempoText()) {
            success = success && this->writeTempo(dynamic_cast<const TempoText*>(controlEvent.first), controlEvent.second);
        }
    }
    m_startingControlEventMap.clear();

    for (auto controlEvent : m_tstampControlEventMap) {
        if (controlEvent.first->isFermata()) {
            success = success && this->writeFermata(dynamic_cast<const Fermata*>(controlEvent.first), controlEvent.second.first,
                                                    controlEvent.second.second);
        }
    }
    m_tstampControlEventMap.clear();

    this->addEndidToControlEvents();

    // This will preprend the scoreDef
    if (!isFirst) {
        this->writeScoreDefChange();
    }
    isFirst = false;

    m_currentNode = m_currentNode.parent();

    return success;
}

/**
 * Write a staff and its content.
 * Checks if each voice has some content.
 */

bool MeiExporter::writeStaff(const Staff* staff, const Measure* measure)
{
    IF_ASSERT_FAILED(staff && measure) {
        return false;
    }

    m_currentNode = m_currentNode.append_child();

    libmei::Staff meiStaff;
    meiStaff.SetN(static_cast<int>(staff->idx() + 1));
    meiStaff.Write(m_currentNode, this->getStaffXmlId());

    track_idx_t startTrack = staff2track(staff->idx());
    track_idx_t endTrack = startTrack + VOICES;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        writeLayer(track, staff, measure);
    }

    m_currentNode = m_currentNode.parent();

    return true;
}

/**
 * Write a layer (i.e., voice) and its content.
 */

bool MeiExporter::writeLayer(track_idx_t track, const Staff* staff, const Measure* measure)
{
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    // If there is no voice, only increase the layer@n
    if (!measure->hasVoice(track)) {
        this->getLayerXmlId();
        return true;
    }

    m_currentNode = m_currentNode.append_child();
    libmei::Layer meiLayer;
    meiLayer.SetN(static_cast<int>(track2voice(track) + 1));
    meiLayer.Write(m_currentNode, this->getLayerXmlId());

    for (Segment* seg = measure->first(); seg; seg = seg->next()) {
        if (seg->segmentType() == SegmentType::EndBarLine) {
            this->addFermataToMap(track, seg, measure);
        }

        // Do not go any further than the measure tick (ignore EndBarLine, KeySigAnnounce, TimeSigAnnounce)
        const EngravingItem* item = seg->element(track);
        if (!item || item->generated()) {
            continue;
        }

        if (item->isClef()) {
            this->writeClef(dynamic_cast<const Clef*>(item));
        } else if (item->isChord()) {
            this->writeChord(dynamic_cast<const Chord*>(item), staff);
        } else if (item->isRest()) {
            this->writeRest(dynamic_cast<const Rest*>(item), staff);
        } else if (item->isBarLine()) {
            //
        } else if (item->isKeySig()) {
            if (m_keySig && (seg != m_keySig)) {
                LOGD() << "MeiExporter::writeLayer unexpected KeySig segment";
            }
            m_keySig = seg;
        } else if (item->isTimeSig()) {
            if (m_timeSig && (seg != m_timeSig)) {
                LOGD() << "MeiExporter::writeLayer unexpected TimeSig segment";
            }
            m_timeSig = seg;
        } else {
            LOGD() << "MeiExporter::writeLayer unknown segment type " << item->typeName();
        }
    }

    m_currentNode = m_currentNode.parent();

    return true;
}

//---------------------------------------------------------
// write MEI layer elements
//---------------------------------------------------------

/**
 * Open and close beam and tuplet elements for a ChordRest.
 * When both a beam and a tuplet is opening and closing, check which is the appropriate nesting order.
 * By default, nest the tuplet within the beam.
 * Closing beam and tuplet only change the bool parameters and actually closing the elements is happening in MeiExporter::writeBeamAndTupletEnd
 */

bool MeiExporter::writeBeamAndTuplet(const ChordRest* chordRest, bool& closingBeam, bool& closingTuplet, bool& closingBeamInTuplet)
{
    IF_ASSERT_FAILED(chordRest) {
        return false;
    }

    const Beam* beam = (chordRest->beam()) ? toBeam(chordRest->beam()) : nullptr;
    const Tuplet* tuplet = (chordRest->tuplet()) ? toTuplet(chordRest->tuplet()) : nullptr;
    const Beam* beamInTuplet = nullptr;

    if (beam && tuplet) {
        if ((beam->elements().front() == chordRest) && ((tuplet->elements().front() == chordRest))) {
            if (beam->elements().size() < tuplet->elements().size()) {
                beamInTuplet = beam;
                beam = nullptr;
            }
        }
    }
    if (beam) {
        this->writeBeam(beam, chordRest, closingBeam);
    }
    if (tuplet) {
        this->writeTuplet(tuplet, chordRest, closingTuplet);
    }
    if (beamInTuplet) {
        this->writeBeam(beamInTuplet, chordRest, closingBeamInTuplet);
    }

    // Case when the beam was open within the tuplet but not at the beginning (fewer elements)
    // We need to close it first since it is nested in the tuplet
    if (closingTuplet && closingBeam && beam->elements().size() < tuplet->elements().size()) {
        closingBeam = false;
        closingBeamInTuplet = true;
    }

    return true;
}

/**
 * Close beam and tuplet elements according to the parameters calculated in MeiExporter::writeBeamAndTuplet
 */

bool MeiExporter::writeBeamAndTupletEnd(bool closingBeam, bool closingTuplet, bool closingBeamInTuplet)
{
    // non critical asserts
    if (closingBeamInTuplet) {
        assert(isCurrentNode(libmei::Beam()));
        m_currentNode = m_currentNode.parent();
    }

    if (closingTuplet) {
        assert(isCurrentNode(libmei::Tuplet()));
        m_currentNode = m_currentNode.parent();
    }

    if (closingBeam) {
        assert(isCurrentNode(libmei::Beam()));
        m_currentNode = m_currentNode.parent();
    }

    return true;
}

/**
 * Write a beam if the ChordRest is the first element of the beam.
 * If the ChordRest is the last, sets the closing flag to true.
 * Also set the MEI `@breaksec` on the previous element when the BeamMode is BEGIN16 or BEGIN32
 */

bool MeiExporter::writeBeam(const Beam* beam, const ChordRest* chordRest, bool& closing)
{
    IF_ASSERT_FAILED(beam && chordRest) {
        return false;
    }

    if (beam->elements().front() == chordRest) {
        libmei::Beam meiBeam;
        m_currentNode = m_currentNode.append_child();
        meiBeam.Write(m_currentNode, this->getLayerXmlIdFor(BEAM_L));
    } else if ((chordRest->beamMode() == BeamMode::BEGIN16) || (chordRest->beamMode() == BeamMode::BEGIN32)) {
        pugi::xml_node lastInBeam = this->getLastChordRest(m_currentNode);
        // We have already written one chord/rest element in the beam, the last one is the one we need to modify
        if (lastInBeam) {
            // Create the attribute class to modify the already created XML element
            libmei::InstBeamSecondary beamSecondary;
            beamSecondary.SetBreaksec(Convert::breaksecToMEI(chordRest->beamMode()));
            beamSecondary.WriteBeamSecondary(lastInBeam);
        }
    }

    if (beam->elements().back() == chordRest) {
        closing = true;
    }

    return true;
}

/**
 * Write a clef.
 */

bool MeiExporter::writeClef(const Clef* clef)
{
    IF_ASSERT_FAILED(clef) {
        return false;
    }

    if (clef->isHeader()) {
        return true;
    }

    pugi::xml_node clefNode = m_currentNode.append_child();
    libmei::Clef meiClef = Convert::clefToMEI(clef->clefType());
    meiClef.Write(clefNode, this->getLayerXmlIdFor(UNSPECIFIED_L));

    return true;
}

/**
 * Write a chord (chord or note).
 * If the Chord is a chord, write its notes with MeiExporter::writeNote.
 * Also write the beam and tuplet (opening and closing) and fill the control event list pointing to it.
 */

bool MeiExporter::writeChord(const Chord* chord, const Staff* staff)
{
    IF_ASSERT_FAILED(chord && staff) {
        return false;
    }

    if (chord->graceNotes().size() > 0) {
        this->writeGraceGrp(chord, staff);
    }

    bool closingBeam = false;
    bool closingTuplet = false;
    bool closingBeamInTuplet = false;
    this->writeBeamAndTuplet(chord, closingBeam, closingTuplet, closingBeamInTuplet);

    bool isChord = (chord->notes().size() > 1);
    if (isChord) {
        // We need to create a <chord> before writing the notes
        m_currentNode = m_currentNode.append_child();
        libmei::Chord meiChord;
        meiChord.SetDur(Convert::durToMEI(chord->durationType().type()));
        if (chord->dots()) {
            meiChord.SetDots(chord->dots());
        }
        this->writeBeamTypeAtt(chord, meiChord);
        this->writeStaffIdenAtt(chord, staff, meiChord);
        this->writeStemAtt(chord, meiChord);
        std::string xmlId = this->getLayerXmlIdFor(CHORD_L, (chord->isGrace() ? nullptr : chord->segment()));
        meiChord.Write(m_currentNode, xmlId);
        this->fillControlEventMap(xmlId, chord);
    }

    for (const Note* note : chord->notes()) {
        this->writeNote(note, chord, staff, isChord);
    }

    if (isChord) {
        // This is the end of the <chord> - non critical assert
        assert(isCurrentNode(libmei::Chord()));
        m_currentNode = m_currentNode.parent();
    }

    this->writeBeamAndTupletEnd(closingBeam, closingTuplet, closingBeamInTuplet);

    if (chord->graceNotes().size() > 0) {
        this->writeGraceGrp(chord, staff, true);
    }

    return true;
}

/**
 * Write a graceGrp placed before or after the chord / note.
 * Loop through the notes of the group and write them.
 */

bool MeiExporter::writeGraceGrp(const Chord* chord, const Staff* staff, bool isAfter)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    GraceNotesGroup& graceNotes = (isAfter) ? chord->graceNotesAfter() : chord->graceNotesBefore();

    if (graceNotes.empty()) {
        return true;
    }

    m_currentNode = m_currentNode.append_child();

    libmei::GraceGrp meiGraceGrp;
    auto [meiAttach, meiGrace] = Convert::gracegrpToMEI(isAfter, graceNotes.front()->noteType());
    meiGraceGrp.SetAttach(meiAttach);
    meiGraceGrp.SetGrace(meiGrace);
    meiGraceGrp.Write(m_currentNode, this->getLayerXmlIdFor(GRACEGRP_L));

    for (auto graceChord : graceNotes) {
        this->writeChord(graceChord, staff);
    }

    // non critical assert
    assert(isCurrentNode(meiGraceGrp));
    m_currentNode = m_currentNode.parent();

    return true;
}

/**
 * Write a note (single note or chord note).
 * For single notes, also writes the duration and the beam and stem attributes.
 */

bool MeiExporter::writeNote(const Note* note, const Chord* chord, const Staff* staff, bool isChord)
{
    IF_ASSERT_FAILED(note && chord && staff) {
        return false;
    }

    Interval interval = staff->part()->instrument()->transpose();
    auto [meiNote, meiAccid] = Convert::pitchToMEI(note, note->accidental(), interval);
    pugi::xml_node noteNode = m_currentNode.append_child();
    if (!isChord) {
        meiNote.SetDur(Convert::durToMEI(chord->durationType().type()));
        if (chord->dots()) {
            meiNote.SetDots(chord->dots());
        }
        this->writeBeamTypeAtt(chord, meiNote);
        this->writeStaffIdenAtt(chord, staff, meiNote);
        this->writeStemAtt(chord, meiNote);
    }
    std::string xmlId = this->getLayerXmlIdFor(NOTE_L, (isChord || chord->isGrace() ? nullptr : chord->segment()));
    meiNote.Write(noteNode, xmlId);
    if (!isChord) {
        this->fillControlEventMap(xmlId, chord);
    }

    if (meiAccid.HasAccid() || meiAccid.HasAccidGes()) {
        pugi::xml_node accidNode = noteNode.append_child();
        meiAccid.Write(accidNode, this->getLayerXmlIdFor(ACCID_L));
    }

    return true;
}

/**
 * Write a rest (rest and mRest).
 */

bool MeiExporter::writeRest(const Rest* rest, const Staff* staff)
{
    IF_ASSERT_FAILED(rest) {
        return false;
    }

    // measure rest
    if (rest->durationType() == DurationType::V_MEASURE) {
        pugi::xml_node mRestNode = m_currentNode.append_child();
        libmei::MRest meiMRest;
        meiMRest.Write(mRestNode, this->getLayerXmlIdFor(UNSPECIFIED_L));
    } else {
        bool closingBeam = false;
        bool closingTuplet = false;
        bool closingBeamInTuplet = false;
        this->writeBeamAndTuplet(rest, closingBeam, closingTuplet, closingBeamInTuplet);

        pugi::xml_node restNode = m_currentNode.append_child();
        libmei::Rest meiRest;
        meiRest.SetDur(Convert::durToMEI(rest->durationType().type()));
        if (rest->dots()) {
            meiRest.SetDots(rest->dots());
        }
        this->writeBeamTypeAtt(rest, meiRest);
        this->writeStaffIdenAtt(rest, staff, meiRest);
        std::string xmlId = this->getLayerXmlIdFor(REST_L, rest->segment());
        meiRest.Write(restNode, xmlId);
        this->fillControlEventMap(xmlId, rest);

        this->writeBeamAndTupletEnd(closingBeam, closingTuplet, closingBeamInTuplet);
    }

    return true;
}

/**
 * Write a tuplet if the ChordRest is the first element of the tuplet.
 * If the ChordRest is the last, sets the closing flag to true.
 */

bool MeiExporter::writeTuplet(const Tuplet* tuplet, const EngravingItem* item, bool& closing)
{
    IF_ASSERT_FAILED(tuplet && item) {
        return false;
    }

    if (tuplet->elements().front() == item) {
        // recursive call for hanling nested tuplets
        // nearly works except for closing which is happening to early (after the first note)
        // when a nested tuplet is ending at the same time as its parent
        /**
        if (tuplet->tuplet()) {
            writeTuplet(toTuplet(tuplet->tuplet()), tuplet, closing);
        }
        if (item->isTuplet()) {
            LOGD() << "MeiExporter::writeTuplet nested tuplet export not fully supported";
        }
        */
        libmei::Tuplet meiTuplet = Convert::tupletToMEI(tuplet);
        m_currentNode = m_currentNode.append_child();
        meiTuplet.Write(m_currentNode, this->getLayerXmlIdFor(TUPLET_L));
    }

    if (tuplet->elements().back() == item) {
        closing = true;
    }

    return true;
}

//---------------------------------------------------------
// write MEI control events
//---------------------------------------------------------

/**
 * Write a breath (i.e., breath or caesura).
 */

bool MeiExporter::writeBreath(const Breath* breath, const std::string& startid)
{
    IF_ASSERT_FAILED(breath) {
        return false;
    }

    pugi::xml_node breathNode = m_currentNode.append_child();
    if (breath->isCaesura()) {
        libmei::Caesura meiCaesura = Convert::caesuraToMEI(breath);
        meiCaesura.SetStartid(startid);
        meiCaesura.Write(breathNode, this->getMeasureXmlIdFor(CAESURA_M));
    } else {
        libmei::Breath meiBreath = Convert::breathToMEI(breath);
        meiBreath.SetStartid(startid);
        meiBreath.Write(breathNode, this->getMeasureXmlIdFor(BREATH_M));
    }

    return true;
}

/**
 * Write a dynam and its text content.
 */

bool MeiExporter::writeDynam(const Dynamic* dynamic, const std::string& startid)
{
    IF_ASSERT_FAILED(dynamic) {
        return false;
    }

    StringList meiLines;

    pugi::xml_node dynamNode = m_currentNode.append_child();
    libmei::Dynam meiDynam = Convert::dynamToMEI(dynamic, meiLines);
    meiDynam.SetStartid(startid);
    meiDynam.Write(dynamNode, this->getMeasureXmlIdFor(DYNAM_M));

    this->writeLines(dynamNode, meiLines);

    return true;
}

/**
 * Write a fermata.
 */

bool MeiExporter::writeFermata(const Fermata* fermata, const std::string& startid)
{
    IF_ASSERT_FAILED(fermata) {
        return false;
    }

    pugi::xml_node fermataNode = m_currentNode.append_child();
    libmei::Fermata meiFermata = Convert::fermataToMEI(fermata);
    meiFermata.SetStartid(startid);

    meiFermata.Write(fermataNode, this->getMeasureXmlIdFor(FERMATA_M));

    return true;
}

/**
 * Write a fermata with a staffNs and tsamp
 */

bool MeiExporter::writeFermata(const Fermata* fermata, const libmei::xsdPositiveInteger_List& staffNs, double tstamp)
{
    IF_ASSERT_FAILED(fermata) {
        return false;
    }

    pugi::xml_node fermataNode = m_currentNode.append_child();
    libmei::Fermata meiFermata = Convert::fermataToMEI(fermata);
    meiFermata.SetStaff(staffNs);
    meiFermata.SetTstamp(tstamp);

    meiFermata.Write(fermataNode, this->getMeasureXmlIdFor(FERMATA_M));

    return true;
}

/**
 * Write a harm and its text content.
 */

bool MeiExporter::writeHarm(const Harmony* harmony, const std::string& startid)
{
    IF_ASSERT_FAILED(harmony) {
        return false;
    }

    StringList meiLines;

    pugi::xml_node dynamNode = m_currentNode.append_child();
    libmei::Harm meiHarm = Convert::harmToMEI(harmony, meiLines);
    meiHarm.SetStartid(startid);
    meiHarm.Write(dynamNode, this->getMeasureXmlIdFor(HARM_M));

    this->writeLines(dynamNode, meiLines);

    return true;
}

/**
 * Write a repeatMark from a Jump.
 */

bool MeiExporter::writeRepeatMark(const Jump* jump, const Measure* measure)
{
    IF_ASSERT_FAILED(jump && measure) {
        return false;
    }

    pugi::xml_node repeatMarkNode = m_currentNode.append_child();
    String text;
    libmei::RepeatMark meiRepeatMark = Convert::jumpToMEI(jump, text);

    if (text.size() > 0) {
        repeatMarkNode.text().set(text.toStdString().c_str());
    }

    meiRepeatMark.SetTstamp(0.0);
    std::string xmlId = this->getMeasureXmlIdFor(REPEATMARK_M);
    meiRepeatMark.Write(repeatMarkNode, xmlId);

    // Currently not used - builds a post-processing list to be processing in MeiExporter::addJumpToRepeatMarks
    // this->addToRepeatMarkList(static_cast<const TextBase*>(jump), repeatMarkNode, xmlId);

    return true;
}

/**
 * Write a repeatMark from a Marker.
 */

bool MeiExporter::writeRepeatMark(const Marker* marker, const Measure* measure)
{
    IF_ASSERT_FAILED(marker && measure) {
        return false;
    }

    pugi::xml_node repeatMarkNode = m_currentNode.append_child();
    String text;
    libmei::RepeatMark meiRepeatMark = Convert::markerToMEI(marker, text);

    if (text.size() > 0) {
        repeatMarkNode.text().set(text.toStdString().c_str());
    }

    meiRepeatMark.SetTstamp(0.0);
    std::string xmlId = this->getMeasureXmlIdFor(REPEATMARK_M);
    meiRepeatMark.Write(repeatMarkNode, xmlId);

    // Currently not used.
    // this->addToRepeatMarkList(dynamic_cast<const TextBase*>(marker), repeatMarkNode, xmlId);

    return true;
}

/**
 * Write a slur.
 */

bool MeiExporter::writeSlur(const Slur* slur, const std::string& startid)
{
    IF_ASSERT_FAILED(slur) {
        return false;
    }

    pugi::xml_node slurNode = m_currentNode.append_child();
    libmei::Slur meiSlur = Convert::slurToMEI(slur);
    meiSlur.SetStartid(startid);

    meiSlur.Write(slurNode, this->getMeasureXmlIdFor(SLUR_M));

    // Add the node to the map of open control events
    m_openControlEventMap[slur] = slurNode;

    return true;
}

/**
 * Write a tempo and its text content.
 */

bool MeiExporter::writeTempo(const TempoText* tempoText, const std::string& startid)
{
    IF_ASSERT_FAILED(tempoText) {
        return false;
    }

    StringList meiLines;

    pugi::xml_node tempoNode = m_currentNode.append_child();
    libmei::Tempo meiTempo = Convert::tempoToMEI(tempoText, meiLines);
    meiTempo.SetStartid(startid);
    meiTempo.Write(tempoNode, this->getMeasureXmlIdFor(DYNAM_M));

    this->writeLinesWithSMuFL(tempoNode, meiLines);

    return true;
}

//---------------------------------------------------------
// write MEI attribute classes
//---------------------------------------------------------

/**
 * Write the beam attributes for a ChordRest (i.e., chord, note, rest or space).
 * Uses the `@type` attribute for storing MuseScore beaming added flags.
 */

bool MeiExporter::writeBeamTypeAtt(const ChordRest* chordRest, libmei::AttTyped& typeAtt)
{
    IF_ASSERT_FAILED(chordRest) {
        return false;
    }

    // Make sure we do not add a @type for notes / rests longer the 8th with a hanging beam flag
    if (int(chordRest->durationType().type()) < int(DurationType::V_EIGHTH)) {
        return true;
    }

    switch (chordRest->beamMode()) {
    // BeamMode::BEGIN16 and BEGIN32 is handled in MeiExporter::writeBeam, which will add MEI a @breaksec to the previous element
    // This is BeamMode in MuseScore is on the first note _after_ the break, whereas it is on the last note _before_ it in MEI.
    case (BeamMode::BEGIN):
    case (BeamMode::MID):
    case (BeamMode::NONE):
        typeAtt.SetType(Convert::beamToMEI(chordRest->beamMode(), BEAM_ELEMENT_TYPE));
        break;
    default:
        break;
    }

    return true;
}

/**
 * Write the cross-staff attribute (@staff) for a ChordRest (i.e., chord, note, rest or space).
 */

bool MeiExporter::writeStaffIdenAtt(const ChordRest* chordRest, const Staff* staff, libmei::AttStaffIdent& staffIdentAtt)
{
    if (chordRest->staffMove() != 0) {
        staff_idx_t staffN = staff->idx() + chordRest->staffMove() + 1;
        libmei::xsdPositiveInteger_List staffNs;
        staffNs.push_back(static_cast<int>(staffN));
        staffIdentAtt.SetStaff(staffNs);
    }

    return true;
}

/**
 * Write the stem attributes for a Chord (i.e., chord or note).
 */

bool MeiExporter::writeStemAtt(const Chord* chord, libmei::AttStems& stemsAtt)
{
    IF_ASSERT_FAILED(chord) {
        return false;
    }

    auto [meiStemDir, meiStemLen] = Convert::stemToMEI(chord->stemDirection(), chord->noStem());
    stemsAtt.SetStemDir(meiStemDir);
    stemsAtt.SetStemLen(meiStemLen);

    return true;
}

bool MeiExporter::isCurrentNode(const libmei::Element& element)
{
    return element.m_name == std::string(m_currentNode.name());
}

/**
 * When writing a measure, find voltas (endings) spanning over it.
 * This will then be use to check if the measure is the beginning or the end or a volta.
 */

std::list<const Volta*> MeiExporter::findVoltasInMeasure(const Measure* measure)
{
    std::list<const Volta*> voltas;
    auto spanners = m_score->spannerMap().findOverlapping(measure->tick().ticks(), measure->endTick().ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (spanner && spanner->isVolta()) {
            voltas.push_back(toVolta(spanner));
        }
    }
    return voltas;
}

/**
 * Go through the list of annotations pointing to the ChordRest and map their element to the xmlId.
 * When writing the annotation (i.e., control events) the map is used to generate the `@startid` of the control event.
 * The value in the map already contains the `#`
 * Also add Breath attached to a ChordRest, which are not represented as annotations but di
 */

void MeiExporter::fillControlEventMap(const std::string& xmlId, const engraving::ChordRest* chordRest)
{
    IF_ASSERT_FAILED(chordRest) {
        return;
    }

    track_idx_t trackIdx = chordRest->track();

    for (const EngravingItem* element : chordRest->segment()->annotations()) {
        if (element->track() == trackIdx) {
            m_startingControlEventMap[element] = "#" + xmlId;
        }
    }
    // Breath a handled differently
    const Breath* breath = chordRest->hasBreathMark();
    if (breath) {
        m_startingControlEventMap[breath] = "#" + xmlId;
    }
    // Slurs
    SpannerMap& smap = m_score->spannerMap();
    auto spanners = smap.findOverlapping(chordRest->tick().ticks(), chordRest->tick().ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (spanner && spanner->isSlur()) {
            if (spanner->startCR() == chordRest) {
                m_startingControlEventMap[spanner] = "#" + xmlId;
            } else if (spanner->endCR() == chordRest) {
                m_endingControlEventMap[spanner] = "#" + xmlId;
            }
        }
    }
}

/**
 * Retrieve the `@startid` for a control event.
 * The map has been filled previously by MeiExporter::addToStartIdMap.
 */

std::string MeiExporter::findStartIdFor(const engraving::EngravingItem* item)
{
    std::string xmlId;
    if (m_startingControlEventMap.count(item)) {
        xmlId = m_startingControlEventMap.at(item);
    }
    return xmlId;
}

/**
 * Keep a list of Mei::Exporter::RepeatMark for post-processing jumps.
 * This code is currently not used.
 * During the post-processing, if a m_jumpToXmlId can be determined, it will be added to the XML m_node.
 * See MeiExporter::addJumpToRepeatMarks
 */

void MeiExporter::addToRepeatMarkList(const EngravingItem* repeatMark, pugi::xml_node node, const std::string& xmlId)
{
    IF_ASSERT_FAILED(repeatMark) {
        return;
    }

    m_repeatMarks.push_back(MeiExporter::RepeatMark());
    RepeatMark& repeatMarkItem = m_repeatMarks.back();
    repeatMarkItem.m_repeatMark = repeatMark;
    repeatMarkItem.m_node = node;
    repeatMarkItem.m_xmlId = xmlId;
}

/**
 * Add `@jumpto` to repeatMark MEI elements by unfolding MuseScore jumps.
 * This code is currently unused because `@jumpto` is not available in MEI.
 * The principle is to post-process the list of m_repeatMarks.
 * For each mark, we lookup the `@jumpto` xmlId, which can then be written to the repeatMark node
 */

void MeiExporter::addJumpToRepeatMarks()
{
    if (m_repeatMarks.size() < 1) {
        return;
    }

    for (RepeatMark& item : m_repeatMarks) {
        if (item.m_repeatMark->isJump()) {
            // For each jump, lookup the Marker that has the label matching the jumpTo
            const Jump* jump = toJump(item.m_repeatMark);
            item.m_jumptToLabel = jump->jumpTo().toStdString();
            auto jumpTo = std::find_if(m_repeatMarks.begin(), m_repeatMarks.end(), [jump](RepeatMark& item) {
                if (!item.m_repeatMark->isMarker()) {
                    return false;
                }
                const Marker* marker = toMarker(item.m_repeatMark);
                // Found it
                return marker->label() == jump->jumpTo();
            });
            if (jumpTo != m_repeatMarks.end()) {
                // This is the xmlId we need to add to the (jump) repeatMark
                item.m_jumpToXmlId = jumpTo->m_xmlId;
            }

            // Try to see if we have a playUntil Marker and a continueAt Marker
            auto playUntil = std::find_if(m_repeatMarks.begin(), m_repeatMarks.end(), [jump](RepeatMark& item) {
                if (!item.m_repeatMark->isMarker()) {
                    return false;
                }
                const Marker* marker = toMarker(item.m_repeatMark);
                return marker->label() == jump->playUntil();
            });
            auto continueAt = std::find_if(m_repeatMarks.begin(), m_repeatMarks.end(), [jump](RepeatMark& item) {
                if (!item.m_repeatMark->isMarker()) {
                    return false;
                }
                const Marker* marker = toMarker(item.m_repeatMark);
                return marker->label() == jump->continueAt();
            });
            // If yes, make the playUntil repeatMark jump to the continueAt
            if (playUntil != m_repeatMarks.end() && continueAt != m_repeatMarks.end()) {
                playUntil->m_jumpToXmlId = continueAt->m_xmlId;
            }
        }
    }
    // Add the jumpto attribute for all the repeatMarks for which we filled a jumpToXmlId
    for (RepeatMark& item : m_repeatMarks) {
        if (item.m_jumpToXmlId.size() > 0) {
            item.m_node.append_attribute("jumpto") = item.m_jumpToXmlId.c_str();
        }
    }
}

/**
 * Check if the Segment (barLineEnd type) has a Fermata annotation for the given track.
 * Add the Fermata to the m_tstampControlEventMap with the appropriate @staff and @tstamp values.
 */

bool MeiExporter::addFermataToMap(const track_idx_t track, const Segment* segment, const Measure* measure)
{
    IF_ASSERT_FAILED(segment) {
        return false;
    }

    for (const auto& annotation : segment->annotations()) {
        if (annotation->isFermata() && track == annotation->track()) {
            staff_idx_t staffN = track2staff(track) + 1;
            libmei::xsdPositiveInteger_List staffNs;
            staffNs.push_back(static_cast<int>(staffN));
            double tstamp = Convert::tstampFromFraction(measure->ticks(), measure->timesig());
            m_tstampControlEventMap[toFermata(annotation)] = std::make_pair(staffNs, tstamp);
        }
    }

    return true;
}

/**
 * Return true if the node name matches the name parameter.
 */

bool MeiExporter::isNode(pugi::xml_node node, const String& name)
{
    if (!node) {
        return false;
    }

    String nodeName = String(node.name());
    return nodeName == name;
}

/**
 * Return the last element in node that is a ChordRest (chord, note, or rest)
 */

pugi::xml_node MeiExporter::getLastChordRest(pugi::xml_node node)
{
    pugi::xml_node chordRest;

    for (pugi::xml_node child : node.children()) {
        if (this->isNode(child, u"chord") || this->isNode(child, u"note") || this->isNode(child, u"rest")) {
            chordRest = child;
        }
    }
    return chordRest;
}

/**
 * Go trough the list of control event maps and add endid when the end element has be written.
 */

void MeiExporter::addEndidToControlEvents()
{
    std::list<const EngravingItem*> closedEvents;

    // Go through the list of open control events and see if the end element has been written
    for (auto controlEvent : m_openControlEventMap) {
        // Convenience variable
        const EngravingItem* item = controlEvent.first;
        // Check if we have both:
        // * the end @xml:id (in m_endingControlEventMap)
        // * the control event node (in m_openControlEventMap)
        if (m_endingControlEventMap.count(item) && m_openControlEventMap.count(item)) {
            // Create an attribute class instance to add the @endid
            libmei::InstStartEndId startEndId;
            startEndId.SetEndid(m_endingControlEventMap.at(item));
            startEndId.WriteStartEndId(m_openControlEventMap.at(item));
            // Add it to the list of closed events we can remove from the maps (below)
            closedEvents.push_back(item);
        }
    }

    for (auto item : closedEvents) {
        m_openControlEventMap.erase(item);
        m_endingControlEventMap.erase(item);
    }
}

//---------------------------------------------------------
// geneate XML:IDs
//---------------------------------------------------------

/**
 * Reset all the sub-counters for measure elements (i.e., control events).
 */

void MeiExporter::resetMeasureIDs()
{
    std::fill(m_measureCounterFor.begin(), m_measureCounterFor.end(), 0);
}

/**
 * Reset all the sub-counters for layer elements (e.g., accid, chord, note, etc.).
 */

void MeiExporter::resetLayerIDs()
{
    std::fill(m_layerCounterFor.begin(), m_layerCounterFor.end(), 0);
}

/**
 * Return the current @xml:id for a section.
 */

std::string MeiExporter::getSectionXmlId()
{
    return String("s%1").arg(++m_sectionCounter).toStdString();
}

/**
 * Return the current @xml:id for an ending.
 */

std::string MeiExporter::getEndingXmlId()
{
    return String("e%1").arg(++m_endingCounter).toStdString();
}

/**
 * Return the current @xml:id for a measure.
 * Reset the staff counter and the measure sub-counters
 */

std::string MeiExporter::getMeasureXmlId()
{
    // Reset the staff counter when a new measure starts
    m_staffCounter = 0;
    // Reset the measure sub-counters when a new measure starts
    this->resetMeasureIDs();
    return String("m%1").arg(++m_measureCounter).toStdString();
}

/**
 * Return the current @xml:id for a measure element (i.e., a control event).
 */

std::string MeiExporter::getMeasureXmlIdFor(measureElementCounter elementType)
{
    // m (Measure) / ? Measure element type
    // The measure element abbreviation is given in the MeiExporter::s_measureXmlIdMap
    return String("m%1%2%3").arg(m_measureCounter).arg(MeiExporter::s_measureXmlIdMap.at(elementType)).arg(++(m_measureCounterFor.
                                                                                                              at(elementType))).
           toStdString();
}

/**
 * Return the current @xml:id for a staff.
 * Reset the layer counters
 */

std::string MeiExporter::getStaffXmlId()
{
    // Reset the layer counter when a new staff starts
    m_layerCounter = 0;
    return String("m%1s%2").arg(m_measureCounter).arg(++m_staffCounter).toStdString();
}

/**
 * Return the current @xml:id for a layer.
 * Reset the layer sub-counters
 */

std::string MeiExporter::getLayerXmlId()
{
    // Reset the layer sub-counters when a new layer starts
    this->resetLayerIDs();
    return String("m%1s%2l%3").arg(m_measureCounter).arg(m_staffCounter).arg(++m_layerCounter).toStdString();
}

/**
 * Return the current @xml:id for a layer element.
 */

std::string MeiExporter::getLayerXmlIdFor(layerElementCounter elementType, const Segment* segment)
{
    // If we have a segment, generate a track and time-based xml:id
    if (segment) {
        //return String("m%1s%2l%3t-%4-%5").arg(m_measureCounter).arg(m_staffCounter).arg(m_layerCounter).arg(segment->tick().numerator()).arg(segment->tick().denominator()).toStdString();
        return String("s%1l%2_t%3_%4").arg(m_staffCounter).arg(m_layerCounter).arg(segment->tick().numerator()).arg(
            segment->tick().denominator()).toStdString();
    }
    // m (Measure) / s (Staff) / l (Layer) / ? Layer element type
    // The layer element abbreviation is given in the MeiExporter::s_layerXmlIdMap
    return String("m%1s%2l%3%4%5").arg(m_measureCounter).arg(m_staffCounter).arg(m_layerCounter).arg(MeiExporter::s_layerXmlIdMap.at(
                                                                                                         elementType)).arg(++(
                                                                                                                               m_layerCounterFor
                                                                                                                               .at(
                                                                                                                                   elementType)))
           .toStdString();
}
