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

#include "musescore.h"
#include "palette/paletteworkspace.h"
#include "plugin/qmliconview.h"
#include "preferences.h"
#include "qml/nativetooltip.h"

#include <QQmlContext>

namespace Ms {

//---------------------------------------------------------
//   PaletteQmlInterface
//---------------------------------------------------------

PaletteQmlInterface::PaletteQmlInterface(PaletteWorkspace* workspace, QmlNativeToolTip* t, bool enabled, QObject* parent)
   : QObject(parent), w(workspace), tooltip(t), _palettesEnabled(enabled)
      {
      tooltip->setParent(this);
      }

//---------------------------------------------------------
//   PaletteQmlInterface::setPaletteBackground
//---------------------------------------------------------

void PaletteQmlInterface::setPaletteBackground(const QColor& val)
      {
      if (_paletteBackground != val) {
            _paletteBackground = val;
            emit paletteBackgroundChanged();
            }
      }

//---------------------------------------------------------
//   PaletteQmlInterface::setPalettesEnabled
//---------------------------------------------------------

void PaletteQmlInterface::setPalettesEnabled(bool val)
      {
      if (_palettesEnabled != val) {
            _palettesEnabled = val;
            emit palettesEnabledChanged();
            }
      }

//---------------------------------------------------------
//   PaletteWidget
//---------------------------------------------------------

PaletteWidget::PaletteWidget(PaletteWorkspace* w, QQmlEngine* e, QWidget* parent, Qt::WindowFlags flags)
   : QmlDockWidget(e, qApp->translate("Ms::PaletteBox", "Palettes"), parent, flags)
      {
      registerQmlTypes();

      QQmlContext* ctx = rootContext();
      Q_ASSERT(ctx);

      QmlNativeToolTip* tooltip = new QmlNativeToolTip(widget());

      qmlInterface = new PaletteQmlInterface(w, tooltip, isEnabled(), this);
      setupStyle();
      ctx->setContextProperty("mscore", qmlInterface);

      setSource(QUrl(qmlSourcePrefix() + "qml/palettes/PalettesWidget.qml"));
      setObjectName("palette-widget");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

      retranslate();
      }

PaletteWidget::PaletteWidget(PaletteWorkspace* w, QWidget* parent, Qt::WindowFlags flags)
   : PaletteWidget(w, nullptr, parent, flags)
      {}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void PaletteWidget::retranslate()
      {
      setWindowTitle(qApp->translate("Ms::PaletteBox", "Palettes"));
      }

//---------------------------------------------------------
//   setupStyle
//---------------------------------------------------------

void PaletteWidget::setupStyle()
      {
      if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR) && preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES))
            qmlInterface->setPaletteBackground(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
      else
            qmlInterface->setPaletteBackground(QColor("#f9f9f9"));
      }

//---------------------------------------------------------
//   PaletteWidget::activateSearchBox
//---------------------------------------------------------

void PaletteWidget::activateSearchBox()
      {
      ensureQmlViewFocused();
      qmlInterface->requestPaletteSearch();
      }

//---------------------------------------------------------
//   PaletteWidget::applyCurrentPaletteElement
//---------------------------------------------------------

void PaletteWidget::applyCurrentPaletteElement()
      {
      const bool invoked = QMetaObject::invokeMethod(rootObject(), "applyCurrentPaletteElement");
      Q_UNUSED(invoked);
      Q_ASSERT(invoked);
      }

//---------------------------------------------------------
//   PaletteWidget::notifyElementDraggedToScoreView
//---------------------------------------------------------

void PaletteWidget::notifyElementDraggedToScoreView()
      {
      qmlInterface->notifyElementDraggedToScoreView();
      }

//---------------------------------------------------------
//   PaletteWidget::showEvent
//---------------------------------------------------------

void PaletteWidget::showEvent(QShowEvent* evt)
      {
      QDockWidget::showEvent(evt);
      if (!wasShown) {
            wasShown = true;
            if (mscoreFirstStart) {
                  // set default width for palettes
                  mscore->resizeDocks({ this }, { int(200 * guiScaling) }, Qt::Horizontal);
                  }
            }
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PaletteWidget::changeEvent(QEvent* evt)
      {
      QmlDockWidget::changeEvent(evt);
      switch (evt->type()) {
            case QEvent::LanguageChange:
                  retranslate();
                  break;
            case QEvent::StyleChange:
                  setupStyle();
                  break;
            case QEvent::EnabledChange:
                  qmlInterface->setPalettesEnabled(isEnabled());
                  break;
            default:
                  break;
            }
      }

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

      qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 3, 3, "PaletteElementEditor", "");

      qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 3, 3, "PaletteTreeModel", "Cannot create palette model from QML");
      qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 3, 3, "FilterPaletteTreeModel", "Cannot create palette model from QML");

      qmlRegisterUncreatableType<QmlNativeToolTip>("MuseScore.Palette", 3, 3, "NativeToolTip", "Use mscore.palette global variable");

      qmlRegisterType<QmlIconView>("MuseScore.Views", 3, 3, "QmlIconView");

      registered = true;
      }

} // namespace Ms
