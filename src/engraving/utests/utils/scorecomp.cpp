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

#include "scorecomp.h"

#include <QProcess>
#include <QTextStream>

#include "scorerw.h"

using namespace mu::engraving;

bool ScoreComp::saveCompareScore(Ms::Score* score, const QString& saveName, const QString& compareWithLocalPath)
{
    if (!ScoreRW::saveScore(score, saveName)) {
        return false;
    }
    return compareFiles(saveName, ScoreRW::rootPath() + "/" + compareWithLocalPath);
}

bool ScoreComp::saveCompareMimeData(QByteArray mimeData, const QString& saveName, const QString& compareWithLocalPath)
{
    if (!ScoreRW::saveMimeData(mimeData, saveName)) {
        return false;
    }
    return compareFiles(saveName, ScoreRW::rootPath() + "/" + compareWithLocalPath);
}

bool ScoreComp::compareFiles(const QString& fullPath1, const QString& fullPath2)
{
    QString cmd = "diff";
    QStringList args;
    args.append("-u");
    args.append("--strip-trailing-cr");
    args.append(fullPath1);
    args.append(fullPath2);

    QProcess p;
    p.start(cmd, args);
    if (!p.waitForFinished()) {
        QTextStream outputText(stdout);
        outputText << "diff failed finished";
        return false;
    }

    int code = p.exitCode();
    if (code) {
        QByteArray ba = p.readAll();
        QTextStream outputText(stdout);
        outputText << QString(ba);
        outputText << QString("   <diff -u %1 %2 failed, code: %3 \n").arg(fullPath1, fullPath2).arg(code);
        return false;
    }

    return true;
}
