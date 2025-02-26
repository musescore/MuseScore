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

#include "global/translation.h"

#ifndef MUSICXML_NO_INTERACTIVE
#include "modularity/ioc.h"
#include "global/iinteractive.h"
#endif

#include "global/io/file.h"
#include "global/serialization/zipreader.h"
#include "global/serialization/xmlstreamreader.h"

#include "engraving/types/types.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/engravingerrors.h"

#include "importmusicxml.h"
#include "importmusicxmllogger.h"
#include "importmusicxmlpass1.h"
#include "importmusicxmlpass2.h"
#include "musicxmlvalidation.h"

#ifndef MUSICXML_NO_INTERACTIVE
using namespace mu;
#endif

using namespace muse;
using namespace mu::engraving;

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   musicXmlImportErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML import error(s).
 */
#ifndef MUSICXML_NO_INTERACTIVE
static IInteractive::Button musicXmlImportErrorDialog(const String& text, const String& detailedText)
{
    auto interactive = modularity::fixmeIoc()->resolve<IInteractive>("musicxml");

    std::string msg = text.toStdString();
    msg += '\n';
    msg += muse::trc("iex_musicxml", "Do you want to try to load this file anyway?");
    msg += '\n';
    msg += '\n';
    msg += detailedText.toStdString();

    IInteractive::Result ret = interactive->questionSync(text.toStdString(),
                                                         msg,
                                                         { IInteractive::Button::Yes, IInteractive::Button::No },
                                                         IInteractive::Button::No
                                                         );

    return ret.standardButton();
}

#endif

//---------------------------------------------------------
//   importMusicXmlfromBuffer
//---------------------------------------------------------

Err importMusicXmlfromBuffer(Score* score, const String& /*name*/, const ByteArray& data)
{
    MusicXmlLogger logger;
    logger.setLoggingLevel(MusicXmlLogger::Level::MXML_ERROR);   // errors only
    //logger.setLoggingLevel(MusicXmlLogger::Level::MXML_INFO);
    //logger.setLoggingLevel(MusicXmlLogger::Level::MXML_TRACE); // also include tracing

    // pass 1
    MusicXmlParserPass1 pass1(score, &logger);
    Err res = pass1.parse(data);
    const String pass1_errors = pass1.errors();

    // pass 2
    MusicXmlParserPass2 pass2(score, pass1, &logger);
    if (res == Err::NoError) {
        res = pass2.parse(data);
    }

    for (const Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }

    // report result
    const String pass2_errors = pass2.errors();
    if (!(pass1_errors.isEmpty() && pass2_errors.isEmpty())) {
#ifndef MUSICXML_NO_INTERACTIVE
        if (!MScore::noGui) {
            const String text = muse::mtrc("iex_musicxml", "%n error(s) found, import may be incomplete.",
                                           nullptr, int(pass1_errors.size() + pass2_errors.size()));
            if (musicXmlImportErrorDialog(text, pass1.errors() + pass2.errors()) != IInteractive::Button::Yes) {
                res = Err::UserAbort;
            }
        }
#endif
    }

    return res;
}

//---------------------------------------------------------
//   check assertions for tuplet handling
//---------------------------------------------------------

/**
 Check assertions for tuplet handling. If this fails, MusicXML
 import will almost certainly break in non-obvious ways.
 */

static_assert(int(DurationType::V_BREVE) == int(DurationType::V_LONG) - 1
              && int(DurationType::V_WHOLE) == int(DurationType::V_BREVE) - 1
              && int(DurationType::V_HALF) == int(DurationType::V_WHOLE) - 1
              && int(DurationType::V_QUARTER) == int(DurationType::V_HALF) - 1
              && int(DurationType::V_EIGHTH) == int(DurationType::V_QUARTER) - 1
              && int(DurationType::V_16TH) == int(DurationType::V_EIGHTH) - 1
              && int(DurationType::V_32ND) == int(DurationType::V_16TH) - 1
              && int(DurationType::V_64TH) == int(DurationType::V_32ND) - 1
              && int(DurationType::V_128TH) == int(DurationType::V_64TH) - 1
              && int(DurationType::V_256TH) == int(DurationType::V_128TH) - 1
              && int(DurationType::V_512TH) == int(DurationType::V_256TH) - 1
              && int(DurationType::V_1024TH) == int(DurationType::V_512TH) - 1);

//---------------------------------------------------------
//   extractRootfile
//---------------------------------------------------------

/**
Extract rootfile from compressed MusicXML file \a qf, return true if OK and false on error.
*/

static bool extractRootfile(const String& name, ByteArray& data)
{
    ZipReader zip(name);

    ByteArray ba = zip.fileData("META-INF/container.xml");

    XmlStreamReader e(ba);

    String rootfile;
    while (e.readNextStartElement()) {
        if (e.name() != "container") {
            break;
        }
        while (e.readNextStartElement()) {
            if (e.name() != "rootfiles") {
                break;
            }
            while (e.readNextStartElement()) {
                if (e.name() == "rootfile") {
                    rootfile = e.attribute("full-path");
                    break;
                }
            }
        }
    }

    data = zip.fileData(rootfile.toStdString());
    return true;
}

//---------------------------------------------------------
//   doValidateAndImport
//---------------------------------------------------------

/**
 Validate and import MusicXML data from file \a name contained in ByteArray \a data into score \a score.
 */

static Err doValidateAndImport(Score* score, const String& name, const ByteArray& data, bool forceMode)
{
    Err res;

    if (!forceMode) {
        // Validate the file
        res = MusicXmlValidation::validate(name, data);
        if (res != Err::NoError) {
            return res;
        }
    }

    // actually do the import
    res = importMusicXmlfromBuffer(score, name, data);
    //LOGD("res %d", static_cast<int>(res));
    return res;
}

//---------------------------------------------------------
//   importMusicXml
//    return Err::File* errors
//---------------------------------------------------------

Err importMusicXml(MasterScore* score, const String& name, bool forceMode)
{
    ScoreLoad sl;     // suppress warnings for undo push/pop

    //LOGD("importMusicXml(%p, %s)", score, muPrintable(name));

    // open the MusicXML file
    io::File xmlFile(name);
    if (!xmlFile.exists()) {
        return Err::FileNotFound;
    }

    if (!xmlFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open MusicXML file: " << name;
        return Err::FileOpenError;
    }

    const ByteArray data = xmlFile.readAll();
    xmlFile.close();

    // and import it
    return doValidateAndImport(score, name, data, forceMode);
}

//---------------------------------------------------------
//   importCompressedMusicXml
//    return false on error
//---------------------------------------------------------

/**
 Import compressed MusicXML file \a name into the Score.
 */

Err importCompressedMusicXml(MasterScore* score, const String& name, bool forceMode)
{
    //LOGD("importCompressedMusicXml(%p, %s)", score, muPrintable(name));

    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    if (!extractRootfile(name, data)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractRootfile
    }

    // and import it
    return doValidateAndImport(score, name, data, forceMode);
}
} // namespace Ms
