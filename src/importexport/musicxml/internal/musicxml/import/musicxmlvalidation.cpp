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

#include "musicxmlvalidation.h"

#ifdef MUSICXML_NO_VALIDATION
using namespace mu;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

Err MusicXmlValidation::validate(const muse::String&, const muse::ByteArray&)
{
    return Err::NoError;
}

#else

#include <QAbstractMessageHandler>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QMessageBox>
#include <QDomDocument>

#include "engraving/dom/mscore.h"

#include "musicxmlsupport.h"

#include "log.h"

using namespace mu;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

//---------------------------------------------------------
//   ValidatorMessageHandler
//---------------------------------------------------------

/**
 Message handler for the MusicXML schema validator QXmlSchemaValidator.
 */

class ValidatorMessageHandler : public QAbstractMessageHandler
{
public:
    ValidatorMessageHandler()
        : QAbstractMessageHandler(0) {}
    QString getErrors() const { return m_errors; }
protected:
    virtual void handleMessage(QtMsgType type, const QString& description, const QUrl& identifier, const QSourceLocation& sourceLocation);
private:
    QString m_errors;
};

void ValidatorMessageHandler::handleMessage(QtMsgType type, const QString& description,
                                            const QUrl& /* identifier */, const QSourceLocation& sourceLocation)
{
    // convert description from html to text
    QDomDocument desc;
    QString contentError;
    int contentLine;
    int contentColumn;
    if (!desc.setContent(description, false, &contentError, &contentLine,
                         &contentColumn)) {
        LOGD("ValidatorMessageHandler: could not parse validation error line %d column %d: %s",
             contentLine, contentColumn, qPrintable(contentError));
        return;
    }

    QDomElement e = desc.documentElement();
    if (e.tagName() != "html") {
        LOGD("ValidatorMessageHandler: description is not html");
        return;
    }

    QString typeStr;
    switch (type) {
    case 0:  typeStr = muse::qtrc("iex_musicxml", "Debug message:");
        break;
    case 1:  typeStr = muse::qtrc("iex_musicxml", "Warning:");
        break;
    case 2:  typeStr = muse::qtrc("iex_musicxml", "Critical error:");
        break;
    case 3:  typeStr = muse::qtrc("iex_musicxml", "Fatal error:");
        break;
    default: typeStr = muse::qtrc("iex_musicxml", "Unknown error:");
        break;
    }

    QString errorStr = typeStr + " " + errorStringWithLocation(sourceLocation.byteOffset(), e.text());

    // append error, separated by newline if necessary
    if (!m_errors.isEmpty()) {
        m_errors += "\n";
    }
    m_errors += errorStr;
}

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
//   musicXmlValidationErrorDialog
//---------------------------------------------------------

/**
 Show a dialog displaying the MusicXML validation error(s)
 and asks the user if he wants to try to load the file anyway.
 Return QMessageBox::Yes (try anyway) or QMessageBox::No (don't)
 */

static int musicXmlValidationErrorDialog(QString text, QString detailedText)
{
    QMessageBox errorDialog;
    errorDialog.setIcon(QMessageBox::Question);
    errorDialog.setText(text);
    errorDialog.setInformativeText(muse::qtrc("iex_musicxml", "Do you want to try to load this file anyway?"));
    errorDialog.setDetailedText(detailedText);
    errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    errorDialog.setDefaultButton(QMessageBox::No);
    return errorDialog.exec();
}

Err MusicXmlValidation::validate(const String& name, const muse::ByteArray& data)
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
        LOGD("importMusicXml() file '%s' is not a valid MusicXML file", muPrintable(name));
        QString strErr = muse::qtrc("iex_musicxml", "File “%1” is not a valid MusicXML file.").arg(name);
        if (MScore::noGui) {
            return Err::NoError;         // might as well try anyhow in converter mode
        }
        if (musicXmlValidationErrorDialog(strErr, messageHandler.getErrors()) != QMessageBox::Yes) {
            return Err::UserAbort;
        }
    }

    return Err::NoError;
}

#endif // MUSICXML_NO_VALIDATION
