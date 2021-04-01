//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "importpreferencesmodel.h"

#include <QTextCodec>

#include "libmscore/mscore.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;

ImportPreferencesModel::ImportPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

void ImportPreferencesModel::load()
{
}

QVariantList ImportPreferencesModel::charsets() const
{
    QList<QByteArray> charsets = QTextCodec::availableCodecs();
    std::sort(charsets.begin(), charsets.end());

    QVariantList result;
    for (QByteArray charset: charsets) {
        result << QString(charset);
    }

    return result;
}

QVariantList ImportPreferencesModel::shortestNotes() const
{
    QVariantList result = {
        QVariantMap { { "title", qtrc("appshell", "Quarter") }, { "value", division() } },
        QVariantMap { { "title", qtrc("appshell", "Eighth") }, { "value", division() / 2 } },
        QVariantMap { { "title", qtrc("appshell", "16th") }, { "value", division() / 4 } },
        QVariantMap { { "title", qtrc("appshell", "32th") }, { "value", division() / 8 } },
        QVariantMap { { "title", qtrc("appshell", "64th") }, { "value", division() / 16 } },
        QVariantMap { { "title", qtrc("appshell", "128th") }, { "value", division() / 32 } },
        QVariantMap { { "title", qtrc("appshell", "256h") }, { "value", division() / 64 } },
        QVariantMap { { "title", qtrc("appshell", "512th") }, { "value", division() / 128 } },
        QVariantMap { { "title", qtrc("appshell", "1024th") }, { "value", division() / 256 } }
    };

    return result;
}

QString ImportPreferencesModel::stylePathFilter() const
{
    return qtrc("appshell", "MuseScore Style File") + " (*.mss)";
}

QString ImportPreferencesModel::styleChooseTitle() const
{
    return qtrc("appshell", "Choose Default Style for Imports");
}

QString ImportPreferencesModel::fileDirectory(const QString& filePath) const
{
    return io::dirpath(filePath.toStdString()).toQString();
}

QString ImportPreferencesModel::styleFileImportPath() const
{
    return musicXmlConfiguration()->styleFileImportPath().toQString();
}

QString ImportPreferencesModel::currentGuitarProCharset() const
{
    return QString::fromStdString(guitarProConfiguration()->importGuitarProCharset());
}

QString ImportPreferencesModel::currentOvertuneCharset() const
{
    return QString::fromStdString(oveConfiguration()->importOvertuneCharset());
}

bool ImportPreferencesModel::importLayout() const
{
    return musicXmlConfiguration()->musicxmlImportLayout();
}

bool ImportPreferencesModel::importBreaks() const
{
    return musicXmlConfiguration()->musicxmlImportBreaks();
}

bool ImportPreferencesModel::needUseDefaultFont() const
{
    return musicXmlConfiguration()->needUseDefaultFont();
}

int ImportPreferencesModel::currentShortestNote() const
{
    return midiImportConfiguration()->midiShortestNote();
}

bool ImportPreferencesModel::needAskAboutApplyingNewStyle() const
{
    return musicXmlConfiguration()->needAskAboutApplyingNewStyle();
}

void ImportPreferencesModel::setStyleFileImportPath(QString path)
{
    if (path == styleFileImportPath()) {
        return;
    }

    musicXmlConfiguration()->setStyleFileImportPath(path.toStdString());
    emit styleFileImportPathChanged(path);
}

void ImportPreferencesModel::setCurrentGuitarProCharset(QString charset)
{
    if (charset == currentGuitarProCharset()) {
        return;
    }

    guitarProConfiguration()->setImportGuitarProCharset(charset.toStdString());
    emit currentGuitarProCharsetChanged(charset);
}

void ImportPreferencesModel::setCurrentOvertuneCharset(QString charset)
{
    if (charset == currentOvertuneCharset()) {
        return;
    }

    oveConfiguration()->setImportOvertuneCharset(charset.toStdString());
    emit currentOvertuneCharsetChanged(charset);
}

void ImportPreferencesModel::setImportLayout(bool import)
{
    if (import == importLayout()) {
        return;
    }

    musicXmlConfiguration()->setMusicxmlImportLayout(import);
    emit importLayoutChanged(import);
}

void ImportPreferencesModel::setImportBreaks(bool import)
{
    if (import == importBreaks()) {
        return;
    }

    musicXmlConfiguration()->setMusicxmlImportBreaks(import);
    emit importBreaksChanged(import);
}

void ImportPreferencesModel::setNeedUseDefaultFont(bool value)
{
    if (value == needUseDefaultFont()) {
        return;
    }

    musicXmlConfiguration()->setNeedUseDefaultFont(value);
    emit needUseDefaultFontChanged(value);
}

void ImportPreferencesModel::setCurrentShortestNote(int note)
{
    if (note == currentShortestNote()) {
        return;
    }

    midiImportConfiguration()->setMidiShortestNote(note);
    emit currentShortestNoteChanged(note);
}

void ImportPreferencesModel::setNeedAskAboutApplyingNewStyle(bool value)
{
    if (value == needAskAboutApplyingNewStyle()) {
        return;
    }

    musicXmlConfiguration()->setNeedAskAboutApplyingNewStyle(value);
    emit needAskAboutApplyingNewStyleChanged(value);
}

int ImportPreferencesModel::division() const
{
    return notationConfiguration()->notationDivision();
}
