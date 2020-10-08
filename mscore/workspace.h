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

struct PaletteTree;
class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   Workspace
//---------------------------------------------------------
class Workspace : public QObject {
      Q_OBJECT

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
      QString _sourceWorkspaceName;
      QString _path;
      bool _dirty;
      bool _readOnly;

      QTimer _saveTimer;

      bool saveComponents { false };
      bool saveToolbars   { false };
      bool saveMenuBar    { false };

      void readGlobalToolBar();
      void readGlobalMenuBar();
      void readGlobalGUIState();

   private slots:
      void ensureWorkspaceSaved();

   public slots:
      void setDirty(bool val);
      void setDirty() { setDirty(true); }

   public:
      Workspace();
      Workspace(const QString& n, const QString& p, bool d, bool r)
         : _name(n), _path(p), _dirty(d), _readOnly(r) {}

      QString id() const;
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
      void reset();

      const Workspace* sourceWorkspace() const;
      void setSourceWorkspaceName(const QString& sourceWorkspaceName) { _sourceWorkspaceName = sourceWorkspaceName; }
      QString sourceWorkspaceName() { return _sourceWorkspaceName; }

      std::unique_ptr<PaletteTree> getPaletteTree() const;

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
      };

class WorkspacesManager {      
   private:
      static void initWorkspaces();
      
   public:
      static Workspace* createNewWorkspace(const QString& name);
      static void refreshWorkspaces();
      
      static QString makeUserWorkspacePath(const QString& name);
      static void readWorkspaceFile(const QString& path, std::function<void(XmlReader&)> readWorkspace);
      static void retranslate(QList<Workspace*>& workspacesList);
      static void retranslateAll();
      
      static Workspace* findByName(const QString& name) {
            for (auto w : m_workspaces) {
                  if (w->name() == name)
                        return w;
                  }
            return nullptr;
            }
      
      static Workspace* findByTranslatableName(const QString& name) {
            for (auto w : m_workspaces) {
                  if (w->translatableName() == name)
                        return w;
                  }
            return nullptr;
            }
      
      static void remove(Workspace* workspace);
      static const QList<Workspace*>& workspaces() {
            if (isWorkspacesListDirty || m_workspaces.isEmpty())
                  initWorkspaces();
            return m_workspaces;
            }
      
      static const QList<Workspace*>& visibleWorkspaces() {
            if (isWorkspacesListDirty || m_visibleWorkspaces.isEmpty())
                  initWorkspaces();
            return m_visibleWorkspaces;
            }
      
      //replace with `const Workspace*` in future
      static Workspace* currentWorkspace() { return m_currentWorkspace; }
      static void setCurrentWorkspace(Workspace* currWorkspace) { m_currentWorkspace = currWorkspace; }
      
      static void initCurrentWorkspace();
      static bool isDefaultWorkspace(Workspace* workspace);
      static bool isDefaultEditedWorkspace(Workspace* workspace);
      static QString defaultWorkspaceTranslatableName(const QString& editedWorkspaceName);

      static void clearWorkspaces();
      
   public:
      static std::vector<QString> defaultWorkspaces;
      static std::vector<QString> defaultEditedWorkspaces;
      
   private:
      static QList<Workspace*> m_workspaces;
      static QList<Workspace*> m_visibleWorkspaces;
      static Workspace* m_currentWorkspace;
      static bool isWorkspacesListDirty;
      };

}
#endif

