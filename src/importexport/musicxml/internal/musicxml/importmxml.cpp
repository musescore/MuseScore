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

#include <QMessageBox>

#include "translation.h"

#include "importmxml.h"
#include "importmxmllogger.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"

#include "engraving/dom/part.h"
#include "engraving/dom/score.h"

namespace mu::engraving {
//---------------------------------------------------------
//   musicXMLImportErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML import error(s).
 */

static int musicXMLImportErrorDialog(QString text, QString detailedText)
{
    QMessageBox errorDialog;
    errorDialog.setIcon(QMessageBox::Question);
    errorDialog.setText(text);
    errorDialog.setInformativeText(qtrc("iex_musicxml", "Do you want to try to load this file anyway?"));
    errorDialog.setDetailedText(detailedText);
    errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    errorDialog.setDefaultButton(QMessageBox::No);
    return errorDialog.exec();
}

static void updateNamesForAccidentals(Instrument* inst)
{
    auto replace = [](String name) {
        name = name.replace(std::regex(
                                R"(((?:^|\s)([A-Ga-g]|[Uu][Tt]|[Dd][Oo]|[Rr][EeÉé]|[MmSsTt][Ii]|[FfLl][Aa]|[Ss][Oo][Ll]))b(?=\s|$))"),
                            String::fromStdString(R"($1♭)"));

        name = name.replace(std::regex(
                                R"(((?:^|\s)([A-Ga-g]|[Uu][Tt]|[Dd][Oo]|[Rr][EeÉé]|[MmSsTt][Ii]|[FfLl][Aa]|[Ss][Oo][Ll]))#(?=\s|$))"),
                            String::fromStdString(R"($1♯)"));

        return name;
    };
    // change staff names from simple text (eg 'Eb') to text using accidental symbols (eg 'E♭')

    // Instrument::longNames() is const af so we need to make a deep copy, update it, and then set it again
    StaffNameList longNamesCopy = inst->longNames();
    for (StaffName& sn : longNamesCopy) {
        sn.setName(replace(sn.name()));
    }
    StaffNameList shortNamesCopy = inst->shortNames();
    for (StaffName& sn : shortNamesCopy) {
        sn.setName(replace(sn.name()));
    }
    inst->setLongNames(longNamesCopy);
    inst->setShortNames(shortNamesCopy);
}

//---------------------------------------------------------
//   importMusicXMLfromBuffer
//---------------------------------------------------------

Err importMusicXMLfromBuffer(Score* score, const QString& /*name*/, QIODevice* dev)
{
    //LOGD("importMusicXMLfromBuffer(score %p, name '%s', dev %p)",
    //       score, qPrintable(name), dev);

    MxmlLogger logger;
    logger.setLoggingLevel(MxmlLogger::Level::MXML_ERROR);   // errors only
    //logger.setLoggingLevel(MxmlLogger::Level::MXML_INFO);
    //logger.setLoggingLevel(MxmlLogger::Level::MXML_TRACE); // also include tracing

    // pass 1
    dev->seek(0);
    MusicXMLParserPass1 pass1(score, &logger);
    Err res = pass1.parse(dev);
    const auto pass1_errors = pass1.errors();

    // pass 2
    MusicXMLParserPass2 pass2(score, pass1, &logger);
    if (res == Err::NoError) {
        dev->seek(0);
        res = pass2.parse(dev);
    }

    for (const Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
            updateNamesForAccidentals(pair.second);
        }
    }

    // report result
    const auto pass2_errors = pass2.errors();
    if (!(pass1_errors.isEmpty() && pass2_errors.isEmpty())) {
        if (!MScore::noGui) {
            const QString text = qtrc("iex_musicxml", "%n error(s) found, import may be incomplete.",
                                      nullptr, pass1_errors.count() + pass2_errors.count());
            if (musicXMLImportErrorDialog(text, pass1.errors() + pass2.errors()) != QMessageBox::Yes) {
                res = Err::UserAbort;
            }
        }
    }

    return res;
}
} // namespace Ms
