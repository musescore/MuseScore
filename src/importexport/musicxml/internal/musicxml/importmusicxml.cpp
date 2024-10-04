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

#include "importmusicxml.h"
#include "importmusicxmllogger.h"
#include "importmusicxmlpass1.h"
#include "importmusicxmlpass2.h"

#include "engraving/dom/part.h"
#include "engraving/dom/score.h"

#ifndef MUSICXML_NO_INTERACTIVE
using namespace mu;
#endif

using namespace muse;

namespace mu::engraving {
//---------------------------------------------------------
//   musicXMLImportErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML import error(s).
 */
#ifndef MUSICXML_NO_INTERACTIVE
static IInteractive::Button musicXMLImportErrorDialog(const String& text, const String& detailedText)
{
    auto interactive = modularity::fixmeIoc()->resolve<IInteractive>("musicxml");

    std::string msg = text.toStdString();
    msg += '\n';
    msg += muse::trc("iex_musicxml", "Do you want to try to load this file anyway?");
    msg += '\n';
    msg += '\n';
    msg += detailedText.toStdString();

    IInteractive::Result ret = interactive->question(text.toStdString(),
                                                     msg,
                                                     { IInteractive::Button::Yes, IInteractive::Button::No },
                                                     IInteractive::Button::No
                                                     );

    return ret.standardButton();
}

#endif

//---------------------------------------------------------
//   importMusicXMLfromBuffer
//---------------------------------------------------------

Err importMusicXMLfromBuffer(Score* score, const String& /*name*/, const ByteArray& data)
{
    MusicXMLLogger logger;
    logger.setLoggingLevel(MusicXMLLogger::Level::MXML_ERROR);   // errors only
    //logger.setLoggingLevel(MusicXMLLogger::Level::MXML_INFO);
    //logger.setLoggingLevel(MusicXMLLogger::Level::MXML_TRACE); // also include tracing

    // pass 1
    MusicXMLParserPass1 pass1(score, &logger);
    Err res = pass1.parse(data);
    const String pass1_errors = pass1.errors();

    // pass 2
    MusicXMLParserPass2 pass2(score, pass1, &logger);
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
            if (musicXMLImportErrorDialog(text, pass1.errors() + pass2.errors()) != IInteractive::Button::Yes) {
                res = Err::UserAbort;
            }
        }
#endif
    }

    return res;
}
} // namespace Ms
