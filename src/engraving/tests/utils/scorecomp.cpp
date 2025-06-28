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

#include "scorecomp.h"

#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QDebug>

#include "scorerw.h"

using namespace muse::io;
using namespace mu::engraving;

bool ScoreComp::saveCompareScore(Score* score, const String& saveName, const String& compareWithLocalPath)
{
    if (!ScoreRW::saveScore(score, saveName)) {
        return false;
    }

    return compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

bool ScoreComp::saveCompareMimeData(muse::ByteArray mimeData, const muse::String& saveName, const muse::String& compareWithLocalPath)
{
    if (!ScoreRW::saveMimeData(mimeData, saveName)) {
        return false;
    }

    return compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

bool ScoreComp::compareFiles(const String& fullPath1, const String& fullPath2)
{
    QString cmd = "diff";
    QStringList args;
    args.append("-u");
    args.append("--strip-trailing-cr");
    args.append(fullPath1);
    args.append(fullPath2);

    QProcess p;
    p.start(cmd, args);
    if (!p.waitForStarted()) {
        QTextStream err(stderr);
        err << p.program() << " failed to start:\n";
        err << "| " << p.errorString() << "\n";

        return false;
    }
    if (!p.waitForFinished()) {
        QTextStream err(stderr);
        err << p.program() << " failed to finish:\n";
        err << "| " << p.errorString() << "\n";

        return false;
    }
    // QProcess::exitCode() is only valid when QProcess::exitStatus() == NormalExit
    if (p.exitStatus() != QProcess::NormalExit) {
        QTextStream err(stderr);
        err << p.program() << " exited abnormally\n";
        err << "| " << p.errorString() << "\n";

        return false;
    }

    if (const int code = p.exitCode()) {
        QTextStream err(stderr);
        QTextStream out(stdout);
        out << p.readAllStandardOutput();
        err << p.readAllStandardError();

        err << String("%1 %2 failed with code: %3 \n")
            .arg(p.program(), p.arguments().join(' '))
            .arg(code);

        QFile file(QStringLiteral(BINARY_DIR) + "/Testing/Temporary/failed_test_reference_files.txt");
        if (!file.open(QIODevice::Append)) {
            return false;
        }
        QString pathToPrint = fullPath1.contains(QStringLiteral("_ref")) ? fullPath1 : fullPath2;
        // Extract only the portion after "/src"
        int srcIndex = pathToPrint.indexOf("/src");
        if (srcIndex != -1) {
            pathToPrint = pathToPrint.mid(srcIndex);
        }

        out << pathToPrint << "\n";

        err << BINARY_DIR;
        return false;
    }

    return true;
}
