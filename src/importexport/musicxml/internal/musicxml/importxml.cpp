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

/**
 MusicXML import.
 */

#include <QBuffer>
#include <QMessageBox>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

#include "global/translation.h"
#include "global/io/file.h"
#include "global/serialization/zipreader.h"
#include "global/serialization/xmlstreamreader.h"

#include "importmxml.h"
#include "musicxmlsupport.h"

#include "engraving/types/types.h"
#include "engraving/dom/masterscore.h"

#include "log.h"

namespace mu::engraving {
//---------------------------------------------------------
//   check assertions for tuplet handling
//---------------------------------------------------------

/**
 Check assertions for tuplet handling. If this fails, MusicXML
 import will almost certainly break in non-obvious ways.
 */

static_assert(int(DurationType::V_BREVE) == int(DurationType::V_LONG) + 1
              && int(DurationType::V_WHOLE) == int(DurationType::V_BREVE) + 1
              && int(DurationType::V_HALF) == int(DurationType::V_WHOLE) + 1
              && int(DurationType::V_QUARTER) == int(DurationType::V_HALF) + 1
              && int(DurationType::V_EIGHTH) == int(DurationType::V_QUARTER) + 1
              && int(DurationType::V_16TH) == int(DurationType::V_EIGHTH) + 1
              && int(DurationType::V_32ND) == int(DurationType::V_16TH) + 1
              && int(DurationType::V_64TH) == int(DurationType::V_32ND) + 1
              && int(DurationType::V_128TH) == int(DurationType::V_64TH) + 1
              && int(DurationType::V_256TH) == int(DurationType::V_128TH) + 1
              && int(DurationType::V_512TH) == int(DurationType::V_256TH) + 1
              && int(DurationType::V_1024TH) == int(DurationType::V_512TH) + 1);

//---------------------------------------------------------
//   initMusicXmlSchema
//    return false on error
//---------------------------------------------------------

static bool initMusicXmlSchema(QXmlSchema& schema)
{
    // read the MusicXML schema from the application resources
    QFile schemaFile(":/schema/musicxml.xsd");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOGE("initMusicXmlSchema() could not open resource musicxml.xsd");
        return false;
    }

    // copy the schema into a QByteArray and fixup xs:imports,
    // using a path to the application resources instead of to www.musicxml.org
    // to prevent downloading from the net
    QByteArray schemaBa;
    QTextStream schemaStream(&schemaFile);
    while (!schemaStream.atEnd()) {
        QString line = schemaStream.readLine();
        if (line.contains("xs:import")) {
            line.replace("http://www.musicxml.org/xsd", "qrc:///schema");
        }
        schemaBa += line.toUtf8();
        schemaBa += "\n";
    }

    // load and validate the schema
    schema.load(schemaBa);
    if (!schema.isValid()) {
        LOGE("initMusicXmlSchema() internal error: MusicXML schema is invalid");
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   musicXMLValidationErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML validation error(s)
 and asks the user if he wants to try to load the file anyway.
 Return QMessageBox::Yes (try anyway) or QMessageBox::No (don't)
 */

static int musicXMLValidationErrorDialog(QString text, QString detailedText)
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
//   doValidate
//---------------------------------------------------------

/**
 Validate MusicXML data from file \a name contained in QIODevice \a dev.
 */

static Err doValidate(const String& name, const ByteArray& data)
{
    //QElapsedTimer t;
    //t.start();

    // initialize the schema
    ValidatorMessageHandler messageHandler;
    QXmlSchema schema;
    schema.setMessageHandler(&messageHandler);
    if (!initMusicXmlSchema(schema)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by initMusicXmlSchema
    }
    // validate the data
    QXmlSchemaValidator validator(schema);
    const QByteArray qdata = data.toQByteArrayNoCopy();
    bool valid = validator.validate(qdata, QUrl::fromLocalFile(name));
    //LOGD("Validation time elapsed: %d ms", t.elapsed());

    if (!valid) {
        LOGD("importMusicXml() file '%s' is not a valid MusicXML file", qPrintable(name));
        QString strErr = qtrc("iex_musicxml", "File “%1” is not a valid MusicXML file.").arg(name);
        if (MScore::noGui) {
            return Err::NoError;         // might as well try anyhow in converter mode
        }
        if (musicXMLValidationErrorDialog(strErr, messageHandler.getErrors()) != QMessageBox::Yes) {
            return Err::UserAbort;
        }
    }

    return Err::NoError;
}

//---------------------------------------------------------
//   doValidateAndImport
//---------------------------------------------------------

/**
 Validate and import MusicXML data from file \a name contained in ByteArray \a data into score \a score.
 */

static Err doValidateAndImport(Score* score, const String& name, const ByteArray& data)
{
    // validate the file
    Err res = doValidate(name, data);
    if (res != Err::NoError) {
        return res;
    }

    // actually do the import
    res = importMusicXMLfromBuffer(score, name, data);
    //LOGD("res %d", static_cast<int>(res));
    return res;
}

//---------------------------------------------------------
//   importMusicXml
//    return Err::File* errors
//---------------------------------------------------------

Err importMusicXml(MasterScore* score, const String& name)
{
    ScoreLoad sl;     // suppress warnings for undo push/pop

    //LOGD("importMusicXml(%p, %s)", score, qPrintable(name));

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
    return doValidateAndImport(score, name, data);
}

//---------------------------------------------------------
//   importCompressedMusicXml
//    return false on error
//---------------------------------------------------------

/**
 Import compressed MusicXML file \a name into the Score.
 */

Err importCompressedMusicXml(MasterScore* score, const String& name)
{
    //LOGD("importCompressedMusicXml(%p, %s)", score, qPrintable(name));

    if (!io::File::exists(name)) {
        return Err::FileNotFound;
    }

    // extract the root file
    ByteArray data;
    if (!extractRootfile(name, data)) {
        return Err::FileBadFormat;      // appropriate error message has been printed by extractRootfile
    }

    // and import it
    return doValidateAndImport(score, name, data);
}

//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

// TODO: move somewhere else

VoiceDesc::VoiceDesc()
    : m_staff(-1), m_voice(-1), m_overlaps(false)
{
    for (int i = 0; i < MAX_STAVES; ++i) {
        m_chordRests[i] =  0;
        m_staffAlloc[i] = -1;
        m_voices[i]     = -1;
    }
}

void VoiceDesc::incrChordRests(int s)
{
    if (0 <= s && s < MAX_STAVES) {
        m_chordRests[s]++;
    }
}

int VoiceDesc::numberChordRests() const
{
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += m_chordRests[i];
    }
    return res;
}

int VoiceDesc::preferredStaff() const
{
    int max = 0;
    int res = 0;
    for (int i = 0; i < MAX_STAVES; ++i) {
        if (m_chordRests[i] > max) {
            max = m_chordRests[i];
            res = i;
        }
    }
    return res;
}

String VoiceDesc::toString() const
{
    String res = u"[";
    for (int i = 0; i < MAX_STAVES; ++i) {
        res += String(u" %1").arg(m_chordRests[i]);
    }
    res += String(u" ] overlaps %1").arg(m_overlaps);
    if (m_overlaps) {
        res += u" staffAlloc [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_staffAlloc[i]);
        }
        res += u" ] voices [";
        for (int i = 0; i < MAX_STAVES; ++i) {
            res += String(u" %1").arg(m_voices[i]);
        }
        res += u" ]";
    } else {
        res += String(u" staff %1 voice %2").arg(m_staff + 1).arg(m_voice + 1);
    }
    return res;
}
} // namespace Ms
