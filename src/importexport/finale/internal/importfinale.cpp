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
#include "importfinale.h"
#include "thirdparty/ezgz.hpp"

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
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

//---------------------------------------------------------
//   importEnigmaXmlfromBuffer
//---------------------------------------------------------

Err importEnigmaXmlfromBuffer(Score* score, ByteArray&& data)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::qt::Document>(reinterpret_cast<const char *>(data.constData()), data.size());

    data.clear(); // free up data now that it isn't needed

    MusxImporter importer(score, doc);
    importer.importParts();  // just create Parts for now

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

    try {
        std::vector<char> result = EzGz::IGzFile<>({ reinterpret_cast<uint8_t*>(gzipData.data()), gzipData.size() }).readAll();
        data = ByteArray(result.data(), result.size());
    } catch (const std::exception &ex) {
        LOGE() << "unable to extract Enigmaxml from file: " << name;
        LOGE() << " (exception: " << ex.what() << ")";
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

void MusxImporter::importParts()
{
    auto scrollView = m_doc->getOthers()->getArray<others::InstrumentUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    int partNumber = 0;
    for (const auto& item : scrollView) {
        auto staff = item->getStaff();
        if (!staff)
            continue; // safety check
        auto multiStaffInst = staff->getMultiStaffInstGroup();
        /// @todo skip staves in multiStaffInsts after the first staff

        Part* part = new Part(m_score);
        //part->setTrack(m_score->nstaves() * VOICES);

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


        int numStaves = multiStaffInst ? multiStaffInst->staffNums.size() : 1;
        for (int x = 1; x <= numStaves; x++) {
            Staff* s = Factory::createStaff(part);
            m_score->appendStaff(s);
        }
        m_score->appendPart(part);
    }
}

}
