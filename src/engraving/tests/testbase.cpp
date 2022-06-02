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

#include <QProcess>
#include <QTextStream>

#include "io/file.h"
#include "io/buffer.h"

#include "config.h"
#include "libmscore/masterscore.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/factory.h"

#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/writescorehook.h"
#include "engraving/infrastructure/io/localfileinfoprovider.h"
#include "engraving/rw/xml.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

static void initMyResources()
{
//    Q_INIT_RESOURCE(mtest);
}

namespace mu::engraving {
//---------------------------------------------------------
//   writeReadElement
//    writes and element and reads it back
//---------------------------------------------------------

EngravingItem* MTest::writeReadElement(EngravingItem* element)
{
    //
    // write element
    //
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);
    xml.startDocument();
    element->write(xml);
    buffer.close();

    //
    // read element
    //

    XmlReader e(buffer.data());
    e.readNextStartElement();
    element = Factory::createItemByName(e.name(), score->dummy());
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
    MasterScore* score_ = mu::engraving::compat::ScoreAccess::createMasterScoreWithBaseStyle();
    io::path_t path = name;
    score_->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));
    std::string suffix = io::suffix(path);

    ScoreLoad sl;
    Score::FileError rv;
    if (suffix == "mscz" || suffix == "mscx") {
        rv = compat::loadMsczOrMscx(score_, name, false);
    } else {
        rv = Score::FileError::FILE_UNKNOWN_TYPE;
    }

    if (rv != Score::FileError::FILE_NO_ERROR) {
        LOGE() << "cannot load file at " << path;
        delete score_;
        score_ = nullptr;
    } else {
        for (Score* s : score->scoreList()) {
            s->doLayout();
        }
    }

    return score_;
}

//---------------------------------------------------------
//   saveScore
//---------------------------------------------------------

bool MTest::saveScore(Score* score_, const QString& name) const
{
    File file(name);
    if (file.exists()) {
        file.remove();
    }

    if (!file.open(IODevice::ReadWrite)) {
        return false;
    }

    compat::WriteScoreHook hook;
    return score_->writeScore(&file, false, false, hook);
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
    LOGD() << "Running " << cmd << " with arg1: " << f2 << " and arg2: " << f1;
    p.start(cmd, args);
    if (!p.waitForFinished() || p.exitCode()) {
        QByteArray ba = p.readAll();
        //LOGD("%s", qPrintable(ba));
        //LOGD("   <diff -u %s %s failed", qPrintable(compareWith),
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
bool MTest::saveCompareScore(Score* score_, const QString& saveName, const QString& compareWith) const
{
    if (!saveScore(score_, saveName)) {
        return false;
    }
    return compareFiles(saveName, compareWith);
}

//---------------------------------------------------------
//   saveMimeData
//---------------------------------------------------------

bool MTest::saveMimeData(QByteArray mimeData, const QString& saveName)
{
    File f(saveName);
    if (!f.open(IODevice::WriteOnly)) {
        return false;
    }

    size_t size = f.write(mimeData);
    return size == static_cast<size_t>(mimeData.size());
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
    MScore::_error = mu::engraving::MsError::MS_NO_ERROR;
}
}
