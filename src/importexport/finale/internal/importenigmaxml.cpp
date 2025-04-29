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
#include "importenigmaxml.h"
#include "dom/sig.h"

#include <zlib.h>
#include <vector>
#include <exception>

#include <QFile>
#include <QFileInfo>

#include "musx/musx.h"

#include "global/io/file.h"
#include "global/serialization/zipreader.h"

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

static size_t readGzipUncompressedSize(const ByteArray& gzDataInput)
{
    if (gzDataInput.size() < 18) { // Must be at least 18 bytes
        return 0;
    }

    const uint8_t* p = reinterpret_cast<const uint8_t*>(gzDataInput.constData());
    size_t n = gzDataInput.size();

    uint32_t isize = static_cast<uint32_t>(p[n - 4])
                     | (static_cast<uint32_t>(p[n - 3]) << 8)
                     | (static_cast<uint32_t>(p[n - 2]) << 16)
                     | (static_cast<uint32_t>(p[n - 1]) << 24);
    return static_cast<size_t>(isize);
}

static bool gunzipBuffer(const ByteArray& gzDataInput, ByteArray& output)
{
    constexpr size_t chunkSize = 262144; // 256 KB chunks
    output.clear();
    output.reserve(readGzipUncompressedSize(gzDataInput));

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(gzDataInput.constData());
    stream.avail_in = static_cast<uInt>(gzDataInput.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        LOGE() << "inflateInit2 failed";
        return false;
    }

    int ret;
    do {
        size_t oldSize = output.size();
        output.resize(oldSize + chunkSize);
        stream.next_out = output.data() + oldSize;
        stream.avail_out = static_cast<uInt>(chunkSize);

        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            LOGE() << "inflate failed";
            inflateEnd(&stream);
            return false;
        }
    } while (ret != Z_STREAM_END);

    output.resize(output.size() - stream.avail_out); // Trim extra
    inflateEnd(&stream);
    return true;
}

//---------------------------------------------------------
//   importEnigmaXmlfromBuffer
//---------------------------------------------------------

Err importEnigmaXmlfromBuffer(Score* score, ByteArray&& data)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::qt::Document>(data.constChar(), data.size());

    data.clear(); // free up data now that it isn't needed

    EnigmaXmlImporter importer(score, doc);
    importer.import();

    score->setUpTempoMap(); //??
    return engraving::Err::NoError;
}

//---------------------------------------------------------
//   extractScoreFile
//---------------------------------------------------------

/**
Extract EnigmaXml from compressed musx file \a name, return true if OK and false on error.
*/

static bool extractScoreFile(const String& name, ByteArray& data)
{
    ZipReader zip(name);
    data.clear();

    ByteArray gzipData = zip.fileData("score.dat");
    if (gzipData.empty()) {
        LOGE() << "no EngimaXML found: " << name;
        return false;
    }

    constexpr static uint32_t INITIAL_STATE = 0x28006D45; // arbitrary initial value for algorithm
    constexpr static uint32_t RESET_LIMIT = 0x20000; // reset value corresponding (probably) to an internal Finale buffer size

    uint32_t state = INITIAL_STATE;
    for (size_t i = 0; i < gzipData.size(); i++) {
        if (i % RESET_LIMIT == 0) {
            state = INITIAL_STATE;
        }
        // this algorithm is BSD rand()!
        state = state * 0x41c64e6d + 0x3039;
        uint16_t upper = state >> 16;
        uint8_t c = uint8_t(upper + upper / 255);
        gzipData[i] ^= c;
    }

    if (!gunzipBuffer(gzipData, data)) {
        LOGE() << "unable to extract Enigmaxml from file: " << name;
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   importMusx
//---------------------------------------------------------

Err importMusx(MasterScore* score, const QString& name)
{

    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    if (!extractScoreFile(name, data)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractScoreFile
    }

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

//---------------------------------------------------------
//   importEnigmaXml
//---------------------------------------------------------

Err importEnigmaXml(MasterScore* score, const QString& name)
{
    io::File xmlFile(name);
    if (!xmlFile.exists()) {
        return Err::FileNotFound;
    }

    if (!xmlFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open EngimaXML file: " << name;
        return Err::FileOpenError;
    }

    ByteArray data = xmlFile.readAll();
    xmlFile.close();

    return importEnigmaXmlfromBuffer(score, std::move(data));
}

static std::string trimNewLineFromString(const std::string& src)
{
    size_t pos = src.find('\n');
    if (pos != std::string::npos) {
        return src.substr(0, pos);  // Truncate at the newline, excluding it
    }
    return src;
}

Staff* EnigmaXmlImporter::createStaff(Part* part, const std::shared_ptr<const others::Staff> musxStaff)
{
    Staff* s = Factory::createStaff(part);
    /// @todo This staffLines setting will move to wherever we parse staff styles
    if (musxStaff->staffLines.has_value()) {
        s->setLines(Fraction(0, 1), musxStaff->staffLines.value());
    } else if (musxStaff->customStaff.has_value()) {
        s->setLines(Fraction(0, 1), musxStaff->customStaff.value().size());
    }
    auto calcBarlineOffsetHalfSpaces = [](Evpu offset, bool isCustom, bool forTop) -> int {
        if (isCustom) {
            if (forTop) {
                offset -= 48;
            } else {
                offset += 48;
            }
        }
        double halfSpaces = (double(offset) * 2.0) / EVPU_PER_SPACE;
        return int(std::lround(halfSpaces));
    };
    s->setBarLineFrom(calcBarlineOffsetHalfSpaces(musxStaff->topBarlineOffset, musxStaff->customStaff.has_value(), true));
    s->setBarLineTo(calcBarlineOffsetHalfSpaces(musxStaff->botBarlineOffset, musxStaff->customStaff.has_value(), false));
    s->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);
    m_staff2Inst.emplace(m_score->nstaves(), InstCmper(musxStaff->getCmper()));
    m_score->appendStaff(s);
    return s;
}

void EnigmaXmlImporter::import()
{
    importParts();
    importMeasures();
}

static Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig)
{
    auto [count, noteType] = simpleMusxTimeSig;
    if (count.remainder()) {
        if ((Edu(noteType) % count.denominator()) == 0) {
            noteType = musx::dom::NoteType(Edu(noteType) / count.denominator());
            count *= count.denominator();
        } else {
            LOGE() << "Time signature has fractional portion that could not be reduced.";
            return Fraction(4, 4);
        }
    }
    return Fraction(count.numerator(),  musx::util::Fraction::fromEdu(Edu(noteType)).denominator());
}

void EnigmaXmlImporter::importMeasures()
{
    Fraction currTimeSig = Fraction(4, 4); // default time signature
    m_score->sigmap()->clear();
    m_score->sigmap()->add(0, currTimeSig);     // default time signature

    auto musxMeasures = m_doc->getOthers()->getArray<others::Measure>(SCORE_PARTID);
    int counter = 0; // DBG
    for (const auto& musxMeasure : musxMeasures) {
        Fraction tick{ 0, 1 };
        auto lastMeasure = m_score->measures()->last();
        if (lastMeasure) {
            tick = lastMeasure->tick() + lastMeasure->ticks();
        }

        Measure* measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        auto musxTimeSig = musxMeasure->createTimeSignature()->calcSimplified();
        auto scoreTimeSig = simpleMusxTimeSigToFraction(musxTimeSig);
        if (scoreTimeSig != currTimeSig) {
            m_score->sigmap()->add(tick.ticks(), scoreTimeSig);
            currTimeSig = scoreTimeSig;
        }
        measure->setTick(tick);
        measure->setTimesig(scoreTimeSig);
        measure->setTicks(scoreTimeSig);
        m_score->measures()->add(measure);
        for (mu::engraving::Staff* staff : m_score->staves()) {
            mu::engraving::staff_idx_t staffIdx = staff->idx();
            // for now, add a full measure rest.
            mu::engraving::Segment* seg = measure->getSegment(mu::engraving::SegmentType::ChordRest, tick);
            Rest* rest = mu::engraving::Factory::createRest(seg, mu::engraving::TDuration(mu::engraving::DurationType::V_MEASURE));
            rest->setScore(m_score);
            rest->setTicks(measure->ticks());
            rest->setTrack(staffIdx * VOICES);
            seg->add(rest);
        }
        if (++counter >= 100) break; //DBG
    }

    /// @todo maybe move this to separate function
    const TimeSigMap& sigmap = *m_score->sigmap();

    for (auto is = sigmap.cbegin(); is != sigmap.cend(); ++is) {
        const SigEvent& se = is->second;
        const int tick = is->first;
        Measure* m = m_score->tick2measure(Fraction::fromTicks(tick));
        if (!m) {
            continue;
        }
        Fraction newTimeSig = se.timesig();
        for (size_t staffIdx = 0; staffIdx < m_score->nstaves(); ++staffIdx) {
            Segment* seg = m->getSegment(SegmentType::TimeSig, Fraction::fromTicks(tick));
            TimeSig* ts = Factory::createTimeSig(seg);
            ts->setSig(newTimeSig);
            ts->setTrack(static_cast<int>(staffIdx) * VOICES);
            seg->add(ts);
        }
        if (newTimeSig != se.timesig()) {     // was a pickup measure - skip next timesig
            ++is;
        }
    }
}

void EnigmaXmlImporter::importParts()
{
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    int partNumber = 0;
    for (const auto& item : scrollView) {
        auto staff = item->getStaff();
        if (!staff)
            continue; // safety check
        auto multiStaffInst = staff->getMultiStaffInstGroup();
        if (multiStaffInst && m_inst2Part.find(staff->getCmper()) != m_inst2Part.end()) {
            continue;
        }

        Part* part = new Part(m_score);

        QString id = QString("P%1").arg(++partNumber);
        part->setId(id);

        auto fullName = staff->getFullInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        if (!fullName.empty()) {
            part->setPartName(QString::fromStdString(trimNewLineFromString(fullName)));
            part->setLongName(QString::fromStdString(trimNewLineFromString(fullName)));
        }

        auto abrvName = staff->getAbbreviatedInstrumentName(musx::util::EnigmaString::AccidentalStyle::Unicode);
        if (!abrvName.empty()) {
            part->setShortName(QString::fromStdString(trimNewLineFromString(abrvName)));
        }

        if (multiStaffInst) {
            m_part2Inst.emplace(id, multiStaffInst->staffNums);
            for (auto inst : multiStaffInst->staffNums) {
                if (auto instStaff = m_doc->getOthers()->get<others::Staff>(SCORE_PARTID, inst)) {
                    createStaff(part, instStaff);
                    m_inst2Part.emplace(inst, id);
                }
            }
        } else {
            createStaff(part, staff);
            m_part2Inst.emplace(id, std::vector<InstCmper>({ InstCmper(staff->getCmper()) }));
            m_inst2Part.emplace(staff->getCmper(), id);
        }
        m_score->appendPart(part);
    }
}

}
