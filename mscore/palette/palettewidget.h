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
class QmlNativeToolTip;

//---------------------------------------------------------
//   PaletteQmlInterface
//---------------------------------------------------------

class PaletteQmlInterface : public QObject
      {
      Q_OBJECT

      Q_PROPERTY(bool palettesEnabled READ palettesEnabled NOTIFY palettesEnabledChanged)
      Q_PROPERTY(Ms::PaletteWorkspace* paletteWorkspace READ paletteWorkspace WRITE setPaletteWorkspace NOTIFY paletteWorkspaceChanged)
      Q_PROPERTY(Ms::QmlNativeToolTip* tooltip READ getTooltip)
      Q_PROPERTY(QColor paletteBackground READ paletteBackground NOTIFY paletteBackgroundChanged)

      PaletteWorkspace* w;
      QmlNativeToolTip* tooltip;
      QColor _paletteBackground;
      bool _palettesEnabled;

      QmlNativeToolTip* getTooltip() { return tooltip; }

   signals:
      void palettesEnabledChanged();
      void paletteWorkspaceChanged();
      void paletteBackgroundChanged();
      void paletteSearchRequested();
      void elementDraggedToScoreView();

   public:
      PaletteQmlInterface(PaletteWorkspace* workspace, QmlNativeToolTip* t, bool enabled, QObject* parent = nullptr);

      PaletteWorkspace* paletteWorkspace() { return w; }
      const PaletteWorkspace* paletteWorkspace() const { return w; }
      void setPaletteWorkspace(PaletteWorkspace* workspace) { w = workspace; emit paletteWorkspaceChanged(); }

      QColor paletteBackground() const { return _paletteBackground; }
      void setPaletteBackground(const QColor& val);

      bool palettesEnabled() const { return _palettesEnabled; }
      void setPalettesEnabled(bool val);

      void notifyElementDraggedToScoreView() { emit elementDraggedToScoreView(); }

      void requestPaletteSearch() { emit paletteSearchRequested(); }

      Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const { return QGuiApplication::keyboardModifiers(); }
      };

//---------------------------------------------------------
//   PaletteWidget
//---------------------------------------------------------

class PaletteWidget : public QmlDockWidget
      {
      Q_OBJECT

      PaletteQmlInterface* qmlInterface;

      bool wasShown = false;

      static void registerQmlTypes();

      void retranslate();
      void setupStyle();

   public:
      PaletteWidget(PaletteWorkspace* w, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
      PaletteWidget(PaletteWorkspace* w, QQmlEngine* e, QWidget* parent, Qt::WindowFlags flags = Qt::WindowFlags());

      void activateSearchBox();
      void applyCurrentPaletteElement();
      void notifyElementDraggedToScoreView();

      void showEvent(QShowEvent* event) override;
      void changeEvent(QEvent* evt) override;
      };

}
#endif

