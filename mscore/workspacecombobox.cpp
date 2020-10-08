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

#include "workspacecombobox.h"

#include "musescore.h"
#include "preferences.h"
#include "workspace.h"

namespace Ms {

//---------------------------------------------------------
//   WorkspaceComboBox
//---------------------------------------------------------

WorkspaceComboBox::WorkspaceComboBox(MuseScore* mScore, QWidget* parent)
   : QComboBox(parent), _mscore(mScore)
      {
      retranslate();
      connect(_mscore, &MuseScore::workspacesChanged, this, &WorkspaceComboBox::updateWorkspaces);
      connect(
         this, QOverload<int>::of(&WorkspaceComboBox::activated),
         this, &WorkspaceComboBox::workspaceSelected
         );
      }

//---------------------------------------------------------
//   WorkspaceComboBox::changeEvent
//---------------------------------------------------------

void WorkspaceComboBox::changeEvent(QEvent* e)
      {
      QComboBox::changeEvent(e);
      switch(e->type()) {
            case QEvent::LanguageChange:
                  retranslate();
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   WorkspaceComboBox::retranslate
//---------------------------------------------------------

void WorkspaceComboBox::retranslate()
      {
      setToolTip(tr("Select workspace"));
      updateWorkspaces();
      }

//---------------------------------------------------------
//   WorkspaceComboBox::updateWorkspaces
//---------------------------------------------------------

void WorkspaceComboBox::updateWorkspaces()
      {
      if (blockUpdateWorkspaces)
            return;

      clear();
      const QList<Workspace*> pl = WorkspacesManager::visibleWorkspaces();
      int idx = 0;
      int curIdx = -1;
      for (Workspace* p : pl) {
            addItem(qApp->translate("Ms::Workspace", p->name().toUtf8()), p->path());
            if (p->id() == preferences.getString(PREF_APP_WORKSPACE))
                  curIdx = idx;
            ++idx;
            }

      //select first workspace in the list if the stored workspace vanished
      Q_ASSERT(!pl.isEmpty());
      if (curIdx == -1)
            curIdx = 0;

      setCurrentIndex(curIdx);
      }

//---------------------------------------------------------
//   WorkspaceComboBox::workspaceSelected
//---------------------------------------------------------

void WorkspaceComboBox::workspaceSelected(int idx)
      {
      if (idx < 0)
            return;

      Workspace* w = WorkspacesManager::visibleWorkspaces().at(idx);
      if (w != WorkspacesManager::currentWorkspace()) {
            blockUpdateWorkspaces = true;
            _mscore->changeWorkspace(w);
            blockUpdateWorkspaces = false;
            }
      }

} // namespace Ms
