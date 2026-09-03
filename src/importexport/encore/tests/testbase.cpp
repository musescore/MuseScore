/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// MTest fixture implementation: import an .enc file into a MasterScore, save it, and diff against expected output.

#include "testbase.h"

#include <QProcess>
#include <QTextStream>

#include "io/file.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/segment.h"

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

namespace mu::iex::enc {
extern Err importEncore(MasterScore* score, const QString& name, const EncImportOptions& opts = EncImportOptions {});
}

namespace mu::engraving {
static MasterScore* loadEncore(const QString& path, const iex::enc::EncImportOptions& opts)
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    ScoreLoad sl;
    Err rv = iex::enc::importEncore(score, path, opts);

    if (rv != Err::NoError) {
        LOGE() << "cannot load file at " << path;
        delete score;
        return nullptr;
    }
    for (Score* s : score->scoreList()) {
        s->doLayout();
    }
    return score;
}

Harmony* segmentHarmony(const Segment* seg)
{
    if (!seg) {
        return nullptr;
    }
    for (EngravingItem* ann : seg->annotations()) {
        if (!ann) {
            continue;
        }
        if (ann->isHarmony()) {
            return toHarmony(ann);
        }
        if (ann->isFretDiagram()) {
            if (Harmony* h = toFretDiagram(ann)->harmony()) {
                return h;
            }
        }
    }
    return nullptr;
}

MasterScore* MTest::readEncoreScore(const QString& name)
{
    return loadEncore(root + "/" + name, iex::enc::EncImportOptions {});
}

MasterScore* MTest::readEncoreScoreWithOpts(const QString& name,
                                            const iex::enc::EncImportOptions& opts)
{
    return loadEncore(root + "/" + name, opts);
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
    return rw::RWRegister::writer()->writeScore(score, &file);
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
    p.start(cmd, args);
    if (!p.waitForFinished() || p.exitCode()) {
        QByteArray ba = p.readAll();
        QTextStream outputText(stdout);
        outputText << QString(ba);
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
} // namespace mu::engraving
