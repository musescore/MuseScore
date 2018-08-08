//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

namespace Ms {

class XmlReader;

//---------------------------------------------------------
//   Workspace
//---------------------------------------------------------

class Workspace : public QObject {
      Q_OBJECT

      static QList<Workspace*> _workspaces;
      static Workspace _advancedWorkspace;
      static Workspace _basicWorkspace;

      QString _name;
      QString _path;
      bool _dirty;
      bool _readOnly;

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
      bool dirty() const             { return _dirty; }

      void save();
      void write();
      void read(XmlReader&);
      void read();
      bool readOnly() const          { return _readOnly; }
      void setReadOnly(bool val)     { _readOnly = val;  }

      static void initWorkspace();
      static Workspace* currentWorkspace;
      static QList<Workspace*>& workspaces();
      static Workspace* createNewWorkspace(const QString& name);
      static bool workspacesRead;
      static void writeBuiltinWorkspace();
      static QList<Workspace*>& refreshWorkspaces();
      };
}
#endif

