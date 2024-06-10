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

#include "testbase.h"

#include <QProcess>
#include <QTextStream>

#include "io/file.h"

#include "engraving/dom/masterscore.h"

#include "engraving/engravingerrors.h"
#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/writescorehook.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/rwregister.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::bb {
extern Err importBB(MasterScore* score, const QString& name);
}

namespace mu::engraving {
MasterScore* MTest::readScore(const QString& name)
{
    QString path = root + "/" + name;
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));
    std::string suffix = muse::io::suffix(path);

    ScoreLoad sl;
    Err rv;
    if (suffix == "mscz" || suffix == "mscx") {
        rv = static_cast<Err>(compat::loadMsczOrMscx(score, path, false).code());
    } else if (suffix == "sgu") {
        rv = iex::bb::importBB(score, path);
    } else {
        rv = Err::FileUnknownType;
    }

    if (rv != Err::NoError) {
        LOGE() << "cannot load file at " << path;
        delete score;
        score = nullptr;
    } else {
        for (Score* s : score->scoreList()) {
            s->doLayout();
        }
    }

    return score;
}

bool MTest::saveScore(Score* score, const QString& name) const
{
    File file(name);
    if (file.exists()) {
        file.remove();
    }

    if (!file.open(IODevice::ReadWrite)) {
        return false;
    }

    return rw::RWRegister::writer(score->iocContext())->writeScore(score, &file, false);
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

bool MTest::saveCompareScore(Score* score, const QString& saveName, const QString& compareWith) const
{
    if (!saveScore(score, saveName)) {
        return false;
    }
    return compareFiles(saveName, compareWith);
}

void MTest::setRootDir(const QString& rootDir)
{
    root = rootDir;
}
}
