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
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/page.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/xml.h"
#include "libmscore/excerpt.h"
#include "thirdparty/qzip/qzipreader_p.h"

#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"

#include "framework/global/globalmodule.h"
#include "framework/fonts/fontsmodule.h"

using namespace mu::engraving;

static void initMyResources()
{
//    Q_INIT_RESOURCE(mtest);
}

namespace Ms {
//---------------------------------------------------------
//   writeReadElement
//    writes and element and reads it back
//---------------------------------------------------------

Element* MTest::writeReadElement(Element* element)
{
    //
    // write element
    //
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    XmlWriter xml(element->score(), &buffer);
    xml.header();
    element->write(xml);
    buffer.close();

    //
    // read element
    //

    XmlReader e(buffer.buffer());
    e.readNextStartElement();
    element = Element::name2Element(e.name(), score);
    element->read(e);
    return element;
}

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

MTest::MTest()
    : ed(0)
{
    MScore::testMode = true;
}

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

MasterScore* MTest::readScore(const QString& name)
{
    QString path = root + "/" + name;
    return readCreatedScore(path);
}

//---------------------------------------------------------
//   readCreatedScore
//---------------------------------------------------------

MasterScore* MTest::readCreatedScore(const QString& name)
{
    MasterScore* score = mu::engraving::compat::ScoreAccess::createMasterScoreWithBaseStyle();
    QFileInfo fi(name);
    score->setName(fi.completeBaseName());
    QString csl  = fi.suffix().toLower();

    ScoreLoad sl;
    Score::FileError rv;
    if (csl == "mscz" || csl == "mscx") {
        rv = compat::loadMsczOrMscx(score, name, false);
    } else {
        rv = Score::FileError::FILE_UNKNOWN_TYPE;
    }

    if (rv != Score::FileError::FILE_NO_ERROR) {
        QWARN(qPrintable(QString("readScore: cannot load <%1> type <%2>\n").arg(name).arg(csl)));
        delete score;
        score = 0;
    } else {
        for (Score* s : score->scoreList()) {
            s->doLayout();
        }
    }
    return score;
}

//---------------------------------------------------------
//   saveScore
//---------------------------------------------------------

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

//---------------------------------------------------------
//   compareFiles
//---------------------------------------------------------

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

//---------------------------------------------------------
//   saveCompareScore
//---------------------------------------------------------

// bool MTest::saveCompareScore(MasterScore* score, const QString& saveName, const QString& compareWith) const
bool MTest::saveCompareScore(Score* score, const QString& saveName, const QString& compareWith) const
{
    if (!saveScore(score, saveName)) {
        return false;
    }
    return compareFiles(saveName, compareWith);
}

//---------------------------------------------------------
//   saveMimeData
//---------------------------------------------------------

bool MTest::saveMimeData(QByteArray mimeData, const QString& saveName)
{
    QFile f(saveName);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }

    f.write(mimeData);
    return f.error() == QFile::NoError;
}

//---------------------------------------------------------
//   saveCompareMimeData
//---------------------------------------------------------

bool MTest::saveCompareMimeData(QByteArray mimeData, const QString& saveName, const QString& compareWith)
{
    saveMimeData(mimeData, saveName);
    return compareFiles(saveName, compareWith);
}

QString MTest::rootPath()
{
    return QString(engraving_tests_DATA_ROOT);
}

//---------------------------------------------------------
//   initMTest
//---------------------------------------------------------

void MTest::initMTest()
{
    initMyResources();
//      DPI  = 120;
//      PDPI = 120;
    MScore::noGui = true;

    // synti  = new MasterSynthesizer();
    mscore = new MScore;
    new MuseScoreCore;
    mscore->init();

    root = rootPath();
    loadInstrumentTemplates(":/data/instruments.xml");
    score = readScore("test.mscx");
}
}
