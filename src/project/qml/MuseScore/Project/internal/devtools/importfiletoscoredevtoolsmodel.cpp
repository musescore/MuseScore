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

#include "importfiletoscoredevtoolsmodel.h"

using namespace mu::project;
using namespace muse;

ImportFileToScoreDevToolsModel::ImportFileToScoreDevToolsModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ImportFileToScoreDevToolsModel::init()
{
    importFileToScoreScenario()->importFinished().onReceive(this, [this](const Ret& ret, const io::path_t& path) {
        appendLog(QString("importFinished: ret = \"%1\", path = \"%2\"")
                  .arg(QString::fromStdString(ret.toString()))
                  .arg(path.toQString()));
    });
}

QString ImportFileToScoreDevToolsModel::log() const
{
    return m_log;
}

void ImportFileToScoreDevToolsModel::appendLog(const QString& line)
{
    m_log += line + "\n";
    emit logChanged();
}

void ImportFileToScoreDevToolsModel::selectAndImportFiles()
{
    appendLog("selectFilesToImport() called");

    importFileToScoreScenario()->selectFilesToImport()
    .onResolve(this, [this](const io::paths_t& paths) {
        QStringList pathStrings;
        for (const io::path_t& path : paths) {
            pathStrings << path.toQString();
        }

        appendLog("selectFilesToImport() resolved: " + pathStrings.join(", "));

        bool started = importFileToScoreScenario()->importFiles(paths);
        appendLog(QString("importFiles() returned %1").arg(started ? "true" : "false"));
    })
    .onReject(this, [this](int code, const std::string& msg) {
        appendLog(QString("selectFilesToImport() rejected: code = %1, msg = \"%2\"")
                  .arg(code)
                  .arg(QString::fromStdString(msg)));
    });
}

void ImportFileToScoreDevToolsModel::checkImportInProgress()
{
    appendLog(QString("isImportInProgress() returned %1").arg(importFileToScoreScenario()->isImportInProgress() ? "true" : "false"));
}

void ImportFileToScoreDevToolsModel::clearLog()
{
    m_log.clear();
    emit logChanged();
}
