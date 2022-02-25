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

#ifndef __MUSESCORECORE_H__
#define __MUSESCORECORE_H__

#include <QList>

namespace Ms {
class MasterScore;
class Score;
enum class SaveReplacePolicy;

//---------------------------------------------------------
//   class MuseScoreCore
//---------------------------------------------------------

class MuseScoreCore
{
protected:
    Score* cs  { 0 };                // current score
    QList<MasterScore*> scoreList;

public:
    MuseScoreCore() = default;
    virtual ~MuseScoreCore() = default;
    Score* currentScore() const { return cs; }
    void setCurrentScore(Score* score) { cs = score; }

    virtual bool saveAs(Score*, bool /*saveCopy*/, const QString& /*path*/, const QString& /*ext*/,
                        SaveReplacePolicy* /*replacePolicy*/ = nullptr) { return false; }
    virtual void closeScore(Score*) {}
    virtual void setCurrentView(int /*tabIdx*/, int /*idx*/) {}

    virtual int appendScore(MasterScore* s) { scoreList.append(s); return 0; }
    virtual MasterScore* openScore(const QString& /*fn*/, bool /*switchTab*/, bool considerInCurrentSession = true,
                                   const QString& /*withFilename*/ = "") { Q_UNUSED(considerInCurrentSession); return 0; }
    QList<MasterScore*>& scores() { return scoreList; }
};
} // namespace Ms
#endif
