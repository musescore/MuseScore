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

#include "palettewidget.h"

#include "palette/paletteworkspace.h"
#include "plugin/qmliconview.h"

#include <QQmlContext>

namespace Ms {

extern QString mscoreGlobalShare;

//---------------------------------------------------------
//   PaletteWidget
//---------------------------------------------------------

PaletteWidget::PaletteWidget(PaletteWorkspace* w, QQmlEngine* e, QWidget* parent, Qt::WindowFlags flags)
   : QmlDockWidget(e, tr("Palettes"), parent, flags)
      {
      registerQmlTypes();

      QQmlContext* ctx = rootContext();
      Q_ASSERT(ctx);

      PaletteQmlInterface* iface = new PaletteQmlInterface(w, this);
      ctx->setContextProperty("mscore", iface);

      setSource(QUrl("qrc:/qml/palettes/PalettesWidget.qml"));

//       setContextMenuPolicy(Qt::ActionsContextMenu);
      setObjectName("palette-widget");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      }

PaletteWidget::PaletteWidget(PaletteWorkspace* w, QWidget* parent, Qt::WindowFlags flags)
   : PaletteWidget(w, nullptr, parent, flags)
      {}

//---------------------------------------------------------
//   registerQmlTypes
//---------------------------------------------------------

void PaletteWidget::registerQmlTypes()
      {
      static bool registered = false;
      if (registered)
            return;

      qmlRegisterUncreatableType<PaletteWorkspace>("MuseScore.Palette", 3, 3, "PaletteWorkspace", "Cannot create palette workspace from QML");
      qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 3, 3, "PaletteController", "Cannot create palette controller from QML");

      qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 3, 3, "PaletteTreeModel", "Cannot create palette model from QML");
      qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 3, 3, "FilterPaletteTreeModel", "Cannot create palette model from QML");

      qmlRegisterType<QmlIconView>("MuseScore.Views", 3, 3, "QmlIconView");

      registered = true;
      }

}
