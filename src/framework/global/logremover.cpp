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
#include "logremover.h"

#include <QDirIterator>
#include <QFile>

using namespace mu;

void LogRemover::removeLogs(const io::path& logsDir, int olderThanDays, const QString& pattern)
{
    //! NOTE If the pattern changes,
    //! then we need to change the implementation of `scanDir` and `parseDate` functions.
    Q_ASSERT(pattern == "MuseScore_yyMMdd_HHmmss.log");
    if (pattern != "MuseScore_yyMMdd_HHmmss.log") {
        return;
    }

    QStringList files;
    scanDir(logsDir, files);

    QDate currentDate = QDate::currentDate();

    QStringList toRemoveFiles;
    for (const QString& file : files) {
        QDate date = parseDate(file);
        if (!date.isValid()) {
            continue;
        }

        int days = date.daysTo(currentDate);
        if (days >= olderThanDays) {
            toRemoveFiles << file;
        }
    }

    removeFiles(toRemoveFiles);
}

QDate LogRemover::parseDate(const QString& fileName)
{
    int endIdx = fileName.lastIndexOf('_');
    if (endIdx == -1) {
        return QDate();
    }

    int startIdx = fileName.lastIndexOf('_', endIdx - 1);
    if (startIdx == -1) {
        return QDate();
    }

    QString dateStr = fileName.mid(startIdx + 1, (endIdx - startIdx - 1));
    QDate date = QDate::fromString(dateStr, "yyMMdd");

    //! NOTE We get `1921`, but it should be `2021`
    if (date.year() < 2000) {
        date = date.addYears(100);
    }
    return date;
}

void LogRemover::removeFiles(const QStringList& files)
{
    for (const QString& file : files) {
        QFile::remove(file);
    }
}

void LogRemover::scanDir(const io::path& logsDir, QStringList& files)
{
    QDirIterator it(logsDir.toQString(), { "*.log" }, QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files);
    while (it.hasNext()) {
        files.push_back(it.next());
    }
}
