//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __QMLPLUGINENGINE_H__
#define __QMLPLUGINENGINE_H__

#include "../qml/msqmlengine.h"
#include "libmscore/score.h"

namespace Ms {
class MuseScore;

//---------------------------------------------------------
//   QmlPluginEngine
//---------------------------------------------------------

class QmlPluginEngine : public MsQmlEngine
{
    Q_OBJECT

    QMap<QString, QVariant> endCmdInfo;
    int cmdCount = 0;
    bool recursion = false;
    bool undoRedo = false;

    ScoreContentState lastScoreState;
    ScoreContentState currScoreState;

signals:
    void endCmd(const QMap<QString, QVariant>& changes);
public:
    QmlPluginEngine(QObject* parent = nullptr);

    void beginEndCmd(MuseScore*, bool undoRedo);
    void endEndCmd(MuseScore*);

    bool inScoreChangeActionHandler() const;
};
} // namespace Ms
#endif
