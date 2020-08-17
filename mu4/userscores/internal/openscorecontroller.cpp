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
#include "openscorecontroller.h"

#include <QObject>

#include "log.h"

using namespace mu;
using namespace mu::userscores;
using namespace mu::notation;

void OpenScoreController::init()
{
    dispatcher()->reg(this, "file-open", this, &OpenScoreController::openScore);
    dispatcher()->reg(this, "file-import", this, &OpenScoreController::importScore);
    dispatcher()->reg(this, "file-new", this, &OpenScoreController::newScore);
}

void OpenScoreController::openScore(const actions::ActionData& args)
{
    io::path scorePath = args.count() > 0 ? args.arg<io::path>(0) : "";

    if (scorePath.empty()) {
        QStringList filter;
        filter << QObject::tr("MuseScore Files") + " (*.mscz *.mscx)";
        scorePath = selectScoreFile(filter);
        if (scorePath.empty()) {
            return;
        }
    }

    doOpenScore(scorePath);
}

void OpenScoreController::importScore()
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

    io::path scorePath = selectScoreFile(filter);

    if (scorePath.empty()) {
        return;
    }

    doOpenScore(scorePath);
}

void OpenScoreController::newScore()
{
    Ret ret = interactive()->open("musescore://userscores/newscore").ret;

    if (ret) {
        ret = interactive()->open("musescore://notation").ret;
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

io::path OpenScoreController::selectScoreFile(const QStringList& filter)
{
    QString filterStr = filter.join(";;");
    return interactive()->selectOpeningFile("Score", "", filterStr);
}

void OpenScoreController::doOpenScore(const io::path& filePath)
{
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

void OpenScoreController::prependToRecentScoreList(io::path filePath)
{
    QStringList recentScoreList = configuration()->recentScoreList().val;
    QString path = filePath.toQString();

    if (recentScoreList.contains(path)) {
        recentScoreList.removeAll(path);
    }

    recentScoreList.prepend(path);
    configuration()->setRecentScoreList(recentScoreList);
}
