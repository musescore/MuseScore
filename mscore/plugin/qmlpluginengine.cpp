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
#include "api/qmlpluginapi.h"
#include "libmscore/score.h"
#ifndef TESTROOT
#include "musescore.h"
#endif

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

void QmlPluginEngine::beginEndCmd(MuseScore* ms)
      {
#ifdef TESTROOT
      Q_UNUSED(ms); // it is not yet possible to include MuseScore class to tests
#else
      ++cmdCount;

      // TODO: most of plugins are never deleted so receivers usually never decrease
      if (!receivers(SIGNAL(endCmd(const QMap<QString, QVariant>&))))
            return;

      const Score* cs = ms->currentScore();

      endCmdInfo["selectionChanged"] = !cs || cs->selectionChanged();
      endCmdInfo["excerptsChanged"] = !cs || cs->masterScore()->excerptsChanged();
      endCmdInfo["instrumentsChanged"] = !cs || cs->masterScore()->instrumentsChanged();

      // TODO: gather the information on which part of a score has changed?
//       endCmdInfo["startLayoutTick"] = cs ? cs->cmdState().startTick().ticks() : -1;
//       endCmdInfo["endLayoutTick"] = cs ? cs->cmdState().endTick().ticks() : -1;
#endif
      }

//---------------------------------------------------------
//   QmlPluginEngine::endEndCmd
//---------------------------------------------------------

void QmlPluginEngine::endEndCmd(MuseScore*)
      {
#ifndef TESTROOT // it is not yet possible to include MuseScore class to tests
      if (cmdCount >= maxCmdCount) {
            QMessageBox::warning(mscore, tr("Plugin Error"), tr("Score update recursion limit reached (%1)").arg(maxCmdCount));
            recursion = true;
            }

      if (!recursion)
            emit endCmd(endCmdInfo);

      --cmdCount;
      if (!cmdCount)
            recursion = false;
#endif
      }
}
