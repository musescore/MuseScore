//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "filescorecontroller.h"

#include <QObject>

#include "log.h"
#include "translation.h"

#include "userscoresconfiguration.h"

using namespace mu;
using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::framework;

void FileScoreController::init()
{
    dispatcher()->reg(this, "file-open", this, &FileScoreController::openScore);
    dispatcher()->reg(this, "file-import", this, &FileScoreController::importScore);
    dispatcher()->reg(this, "file-new", this, &FileScoreController::newScore);

    dispatcher()->reg(this, "file-save", this, &FileScoreController::saveScore);
    dispatcher()->reg(this, "file-save-as", this, &FileScoreController::saveScoreAs);
}

void FileScoreController::openScore(const actions::ActionData& args)
{
    io::path scorePath = args.count() > 0 ? args.arg<io::path>(0) : "";

    if (scorePath.empty()) {
        QStringList filter;
        filter << QObject::tr("MuseScore Files") + " (*.mscz *.mscx)";
        scorePath = selectScoreOpenningFile(filter);
        if (scorePath.empty()) {
            return;
        }
    }

    doOpenScore(scorePath);
}

void FileScoreController::importScore()
{
    QString allExt = "*.mscz *.mscx *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx"
                     "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscz, *.mscx,";

    QStringList filter;
    filter << QObject::tr("All Supported Files") + " (" + allExt + ")"
           << QObject::tr("MuseScore Files") + " (*.mscz *.mscx)"
           << QObject::tr("MusicXML Files") + " (*.mxl *.musicxml *.xml)"
           << QObject::tr("MIDI Files") + " (*.mid *.midi *.kar)"
           << QObject::tr("MuseData Files") + " (*.md)"
           << QObject::tr("Capella Files") + " (*.cap *.capx)"
           << QObject::tr("BB Files (experimental)") + " (*.mgu *.sgu)"
           << QObject::tr("Overture / Score Writer Files (experimental)") + " (*.ove *.scw)"
           << QObject::tr("Bagpipe Music Writer Files (experimental)") + " (*.bmw *.bww)"
           << QObject::tr("Guitar Pro Files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)"
           << QObject::tr("Power Tab Editor Files (experimental)") + " (*.ptb)"
           << QObject::tr("MuseScore Backup Files") + " (*.mscz, *.mscx,)";

    io::path scorePath = selectScoreOpenningFile(filter);

    if (scorePath.empty()) {
        return;
    }

    doOpenScore(scorePath);
}

void FileScoreController::newScore()
{
    Ret ret = interactive()->open("musescore://userscores/newscore").ret;

    if (ret) {
        ret = interactive()->open("musescore://notation").ret;
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void FileScoreController::saveScore()
{
    if (!globalContext()->currentMasterNotation()->created().val) {
        doSaveScore();
        return;
    }

    io::path defaultFilePath = defaultSavingFilePath();

    io::path filePath = selectScoreSavingFile(defaultFilePath);
    if (filePath.empty()) {
        return;
    }

    if (io::syffix(filePath).empty()) {
        filePath = filePath + UserScoresConfiguration::DEFAULT_FILE_SUFFIX;
    }

    doSaveScore(filePath);
}

void FileScoreController::saveScoreAs()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath);
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath);
}

io::path FileScoreController::selectScoreOpenningFile(const QStringList& filter)
{
    QString filterStr = filter.join(";;");
    return interactive()->selectOpeningFile(qtrc("userscores", "Score"), "", filterStr);
}

io::path FileScoreController::selectScoreSavingFile(const io::path& defaultFilePath)
{
    QString filter = QObject::tr("MuseScore Files") + " (*.mscz *.mscx)";
    io::path filePath = interactive()->selectSavingFile(qtrc("userscores", "Save Score"), defaultFilePath, filter);

    return filePath;
}

void FileScoreController::doOpenScore(const io::path& filePath)
{
    TRACEFUNC;

    auto notation = notationCreator()->newMasterNotation();
    IF_ASSERT_FAILED(notation) {
        return;
    }

    Ret ret = notation->load(filePath);
    if (!ret) {
        LOGE() << "failed load: " << filePath << ", ret: " << ret.toString();
        //! TODO Show dialog about error
        return;
    }

    if (!globalContext()->containsMasterNotation(filePath)) {
        globalContext()->addMasterNotation(notation);
    }

    globalContext()->setCurrentMasterNotation(notation);

    prependToRecentScoreList(filePath);

    interactive()->open("musescore://notation");
}

void FileScoreController::doSaveScore(const io::path& filePath)
{
    globalContext()->currentMasterNotation()->save(filePath);
}

io::path FileScoreController::defaultSavingFilePath() const
{
    Meta scoreMetaInfo = globalContext()->currentMasterNotation()->metaInfo();

    QString fileName = scoreMetaInfo.title;
    if (fileName.isEmpty()) {
        fileName = scoreMetaInfo.fileName;
    }

    return configuration()->defaultSavingFilePath(fileName.toStdString());
}

void FileScoreController::prependToRecentScoreList(io::path filePath)
{
    QStringList recentScoreList = configuration()->recentScoreList().val;
    QString path = filePath.toQString();

    if (recentScoreList.contains(path)) {
        recentScoreList.removeAll(path);
    }

    recentScoreList.prepend(path);
    configuration()->setRecentScoreList(recentScoreList);
}
