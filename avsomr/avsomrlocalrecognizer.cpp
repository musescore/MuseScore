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

#include "avsomrlocalrecognizer.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcess>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include "avslog.h"
#include "avsomrlocal.h"

using namespace Ms::Avs;

AvsOmrLocalRecognizer::AvsOmrLocalRecognizer()
{
}

AvsOmrLocalRecognizer::~AvsOmrLocalRecognizer()
{
}

//---------------------------------------------------------
//   type
//---------------------------------------------------------

QString AvsOmrLocalRecognizer::type() const
{
    return "local";
}

//---------------------------------------------------------
//   avsOmrLocal
//---------------------------------------------------------

AvsOmrLocal* AvsOmrLocalRecognizer::avsOmrLocal() const
{
    return AvsOmrLocal::instance();
}

//---------------------------------------------------------
//   makeBuildPath
//---------------------------------------------------------

QString AvsOmrLocalRecognizer::makeBuildPath() const
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString path = tempPath
                   + "/" + QCoreApplication::applicationName()
                   + "/avsomr/build";

    return path;
}

//---------------------------------------------------------
//   isAvailable
//---------------------------------------------------------

bool AvsOmrLocalRecognizer::isAvailable() const
{
    return avsOmrLocal()->isInstalled();
}

//---------------------------------------------------------
//   recognize
//---------------------------------------------------------

bool AvsOmrLocalRecognizer::recognize(const QString& filePath, QByteArray* avsFileData, const OnStep& onStep)
{
    auto step = [&onStep](Step::Type t, uint perc, uint16_t percMax, Ret ret = Ret::Ok) {
                    if (!ret) {
                        LOGE() << "failed step: " << t << ", ret: " << ret.formatedText();
                    } else {
                        LOGI() << "success step: " << t << ", ret: " << ret.formatedText();
                    }

                    if (onStep) {
                        onStep(Step(t, perc, percMax, ret));
                    }
                };

    step(Step::PrepareStart, 1, 10);

    Ret ret = avsOmrLocal()->checkInstallOrUpdate(true);

    const QString buildDir = makeBuildPath();
    if (ret) {
        ret = cleanDir(buildDir);
    }

    step(Step::PrepareFinish, 10, 10, ret);
    if (!ret) {
        return false;
    }

    step(Step::ProcessingStart, 11, 90);
    ret = avsOmrLocal()->build(filePath, buildDir);
    step(Step::ProcessingFinish, 90, 90, ret);
    if (!ret) {
        return false;
    }

    step(Step::LoadStart, 91, 100);
    QString avsPath = avsOmrLocal()->makeAvsFilePath(buildDir, QFileInfo(filePath).baseName());
    ret = readFile(avsFileData, avsPath);
    if (!ret) {
        //! NOTE If we cannot read the resulting file,
        //! then this means failed to execute
        ret = Ret::LocalFailedExec;
    }
    step(Step::LoadFinish, 100, 100, ret);
    if (!ret) {
        return false;
    }

    cleanDir(buildDir);

    return true;
}

//---------------------------------------------------------
//   cleanDir
//---------------------------------------------------------

Ret AvsOmrLocalRecognizer::cleanDir(const QString& dirPath)
{
    return QDir(dirPath).removeRecursively() ? Ret::Ok : Ret::FailedClearDir;
}

//---------------------------------------------------------
//   readFile
//---------------------------------------------------------

Ret AvsOmrLocalRecognizer::readFile(QByteArray* avsData, const QString& avsPath)
{
    IF_ASSERT(avsData) {
        return Ret::FailedReadFile;
    }

    QFile avsFile(avsPath);
    if (!avsFile.exists()) {
        LOGE() << "not found avs file: " << avsPath;
        return Ret::FailedReadFile;
    }

    if (!avsFile.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open avs file: " << avsPath;
        return Ret::FailedReadFile;
    }

    *avsData = avsFile.readAll();

    return Ret::Ok;
}
