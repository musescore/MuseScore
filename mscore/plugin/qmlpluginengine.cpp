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

#include "qmlpluginengine.h"
#include "mu4/plugins/api/qmlpluginapi.h"
#include "libmscore/score.h"
#include "musescore.h"

namespace Ms {
static constexpr int maxCmdCount = 10; // recursion prevention

//---------------------------------------------------------
//   QmlPluginEngine
//---------------------------------------------------------

QmlPluginEngine::QmlPluginEngine(QObject* parent)
    : MsQmlEngine(parent)
{
    PluginAPI::PluginAPI::registerQmlTypes();
}

//---------------------------------------------------------
//   QmlPluginEngine::beginEndCmd
//---------------------------------------------------------

void QmlPluginEngine::beginEndCmd(MuseScore* ms, bool inUndoRedo)
{
    ++cmdCount;

    if (inUndoRedo) {
        undoRedo = true;
    }

    const Score* cs = ms->currentScore();

    // score and excerpts have united undo stack so we are better to track master score
    currScoreState = cs ? cs->masterScore()->state() : ScoreContentState();

    // TODO: most of plugins are never deleted so receivers usually never decrease
    if (!receivers(SIGNAL(endCmd(const QMap<QString,QVariant>&)))) {
        return;
    }

    endCmdInfo["selectionChanged"] = !cs || cs->selectionChanged();
    endCmdInfo["excerptsChanged"] = !cs || cs->masterScore()->excerptsChanged();
    endCmdInfo["instrumentsChanged"] = !cs || cs->masterScore()->instrumentsChanged();

    endCmdInfo["startLayoutTick"] = cs ? cs->cmdState().startTick().ticks() : -1;
    endCmdInfo["endLayoutTick"] = cs ? cs->cmdState().endTick().ticks() : -1;

    endCmdInfo["undoRedo"] = undoRedo;
}

//---------------------------------------------------------
//   QmlPluginEngine::endEndCmd
//---------------------------------------------------------

void QmlPluginEngine::endEndCmd(MuseScore*)
{
    if (cmdCount >= maxCmdCount) {
        QMessageBox::warning(mscore, tr("Plugin Error"), tr("Score update recursion limit reached (%1)").arg(
                                 maxCmdCount));
        recursion = true;
    }

    if (!recursion) {
        emit endCmd(endCmdInfo);
    }

    --cmdCount;
    if (!cmdCount) {
        recursion = false;
        undoRedo = false;
        lastScoreState = currScoreState;
    }
}

//---------------------------------------------------------
//   QmlPluginEngine::inScoreChangeActionHandler
///   Returns \p true if the engine is in process of
///   handling endCmd() call which is a result of score
///   change user action (not undo/redo or simple selection
///   changes/mouse clicks etc.)
//---------------------------------------------------------

bool QmlPluginEngine::inScoreChangeActionHandler() const
{
    return cmdCount > 0 && !undoRedo && currScoreState.isNewerThan(lastScoreState);
}
}
