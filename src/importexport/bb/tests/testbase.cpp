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

#include "testbase.h"

#include <QtTest/QtTest>
#include <QTextStream>

#include "config.h"
#include "libmscore/score.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/musescoreCore.h"

#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"

using namespace mu::engraving;

namespace Ms {
extern Score::FileError importBB(MasterScore* score, const QString& name);
}

namespace Ms {
MTest::MTest()
{
    MScore::testMode = true;
}

MasterScore* MTest::readScore(const QString& name)
{
    QString path = root + "/" + name;
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    QFileInfo fi(path);
    score->setName(fi.completeBaseName());
    QString csl  = fi.suffix().toLower();

    ScoreLoad sl;
    Score::FileError rv;
    if (csl == "mscz" || csl == "mscx") {
        rv = compat::loadMsczOrMscx(score, path, false);
    } else if (csl == "sgu") {
        rv = importBB(score, path);
    } else {
        rv = Score::FileError::FILE_UNKNOWN_TYPE;
    }

    if (rv != Score::FileError::FILE_NO_ERROR) {
        QWARN(qPrintable(QString("readScore: cannot load <%1> type <%2>\n").arg(path).arg(csl)));
        delete score;
        score = 0;
    } else {
        for (Score* s : score->scoreList()) {
            s->doLayout();
        }
    }
    return score;
}

bool MTest::saveScore(Score* score, const QString& name) const
{
    QFile file(name);
    if (file.exists()) {
        file.remove();
    }

    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    return score->Score::writeScore(&file, false);
}

bool MTest::compareFilesFromPaths(const QString& f1, const QString& f2)
{
    QString cmd = "diff";
    QStringList args;
    args.append("-u");
    args.append("--strip-trailing-cr");
    args.append(f2);
    args.append(f1);
    QProcess p;
    qDebug() << "Running " << cmd << " with arg1: " << QFileInfo(f2).fileName() << " and arg2: "
             << QFileInfo(f1).fileName();
    p.start(cmd, args);
    if (!p.waitForFinished() || p.exitCode()) {
        QByteArray ba = p.readAll();
        //qDebug("%s", qPrintable(ba));
        //qDebug("   <diff -u %s %s failed", qPrintable(compareWith),
        //   qPrintable(QString(root + "/" + saveName)));
        QTextStream outputText(stdout);
        outputText << QString(ba);
        outputText << QString("   <diff -u %1 %2 failed").arg(f2).arg(f1);
        return false;
    }
    return true;
}

bool MTest::compareFiles(const QString& saveName, const QString& compareWith) const
{
    return compareFilesFromPaths(saveName, root + "/" + compareWith);
}

bool MTest::saveCompareScore(Score* score, const QString& saveName, const QString& compareWith) const
{
    if (!saveScore(score, saveName)) {
        return false;
    }
    return compareFiles(saveName, compareWith);
}

void MTest::initMTest(const QString& rootDir)
{
    MScore::noGui = true;

    mscore = new MScore;
    new MuseScoreCore;
    mscore->init();

    root = rootDir;
    loadInstrumentTemplates(":/data/instruments.xml");
}
}
