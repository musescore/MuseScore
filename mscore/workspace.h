//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2011 Werner Schweer and others
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

#ifndef __WORKSPACE_H__
#define __WORKSPACE_H__

#include <unordered_map>

namespace Ms {

class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   Workspace
//---------------------------------------------------------

class Workspace : public QObject {
      Q_OBJECT

      static const char* advancedWorkspaceTranslatableName;
      static const char* basicWorkspaceTranslatableName;

      static QList<Workspace*> _workspaces;
      static QList<QPair<QAction*, QString>> actionToStringList;
      static QList<QPair<QMenu*, QString>> menuToStringList;

      static void writeMenuBar(XmlWriter& xml, QMenuBar* mb = 0);
      static void writeMenu(XmlWriter& xml, QMenu* menu);
      static void addRemainingFromMenu(QMenu* menu);

      void readMenu(XmlReader& e, QMenu* menu);

      static QString findStringFromAction(QAction* action);
      static QAction* findActionFromString(QString string);
      static QString findStringFromMenu(QMenu* menu);

      QString _name;
      QString _translatableName;
      QString _path;
      bool _dirty;
      bool _readOnly;

      bool saveComponents;
      bool saveToolbars;
      bool saveMenuBar;

      void readGlobalToolBar();
      void readGlobalMenuBar();
      void readGlobalGUIState();

   public slots:
      void setDirty(bool val = true) { _dirty = val;    }

   public:
      Workspace();
      Workspace(const QString& n, const QString& p, bool d, bool r)
         : _name(n), _path(p), _dirty(d), _readOnly(r) {}

      QString path() const           { return _path;  }
      void setPath(const QString& s) { _path = s;     }
      QString name() const           { return _name;  }
      void setName(const QString& s) { _name = s;     }
      const QString& translatableName() const  { return _translatableName;    }
      void setTranslatableName(QString trName) { _translatableName = trName;  }
      void rename(const QString& s);
      bool dirty() const             { return _dirty; }

      void save();
      void write();
      void read(XmlReader&);
      void read();
      bool readOnly() const          { return _readOnly;  }
      void setReadOnly(bool val)     { _readOnly = val;   }

      static void initWorkspace();
      static Workspace* currentWorkspace;
      static QList<Workspace*>& workspaces();
      static Workspace* createNewWorkspace(const QString& name);
      static bool workspacesRead;
      static QList<Workspace*>& refreshWorkspaces();

      static void addActionAndString(QAction* action, QString string);
      static void addRemainingFromMenuBar(QMenuBar* mb);
      static void addMenuAndString(QMenu* menu, QString string);
      static QMenu* findMenuFromString(QString string);

      bool getSaveComponents()       { return saveComponents; }
      void setSaveComponents(bool s) { saveComponents = s;    }
      bool getSaveToolbars()         { return saveToolbars;   }
      void setSaveToolbars(bool s)   { saveToolbars = s;      }
      bool getSaveMenuBar()          { return saveMenuBar;    }
      void setSaveMenuBar(bool s)    { saveMenuBar = s;       }

      static void writeGlobalMenuBar(QMenuBar* mb);
      static void writeGlobalToolBar();
      static void writeGlobalGUIState();

      static void retranslate(QList<Workspace*>* workspacesList = 0);
      };

}
#endif

