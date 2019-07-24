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

#ifndef __PALETTEWIDGET_H__
#define __PALETTEWIDGET_H__

#include "qmldockwidget.h"

namespace Ms {

class PaletteWorkspace;

//---------------------------------------------------------
//   PaletteQmlInterface
//---------------------------------------------------------

class PaletteQmlInterface : public QObject
      {
      Q_OBJECT

      Q_PROPERTY(Ms::PaletteWorkspace* paletteWorkspace READ paletteWorkspace WRITE setPaletteWorkspace NOTIFY paletteWorkspaceChanged)

      PaletteWorkspace* w;

   signals:
      void paletteWorkspaceChanged();

   public:
      PaletteQmlInterface(PaletteWorkspace* workspace, QObject* parent = nullptr)
         : QObject(parent), w(workspace) {}

      PaletteWorkspace* paletteWorkspace() { return w; }
      const PaletteWorkspace* paletteWorkspace() const { return w; }
      void setPaletteWorkspace(PaletteWorkspace* workspace) { w = workspace; emit paletteWorkspaceChanged(); }

      Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const { return QGuiApplication::keyboardModifiers(); }
      };

//---------------------------------------------------------
//   PaletteWidget
//---------------------------------------------------------

class PaletteWidget : public QmlDockWidget
      {
      Q_OBJECT

      static void registerQmlTypes();

   public:
      PaletteWidget(PaletteWorkspace* w, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
      PaletteWidget(PaletteWorkspace* w, QQmlEngine* e, QWidget* parent, Qt::WindowFlags flags = Qt::WindowFlags());
      };

}
#endif

