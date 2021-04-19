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
#ifndef MS_SCORE_PREFERENCES_H
#define MS_SCORE_PREFERENCES_H

#include <QString>

namespace Ms {
class ScorePreferences
{
public:

    static ScorePreferences& instance();

    QString backupDirPath() const;
    void setBackupDirPath(const QString& path);

    QString defaultStyleFilePath() const;
    void setDefaultStyleFilePath(const QString& path);

private:
    QString m_backupDirPath;
    QString m_defaultStyleFilePath;
};
}

inline Ms::ScorePreferences& preferences()
{
    return Ms::ScorePreferences::instance();
}

#endif // MS_SCORE_PREFERENCES_H
