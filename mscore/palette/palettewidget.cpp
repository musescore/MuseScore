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

#include <QDockWidget>
#include <QQmlContext>

#include "musescore.h"
#include "palette/paletteworkspace.h"
#include "plugin/qmliconview.h"
#include "preferences.h"
#include "qml/nativetooltip.h"

#include "modularity/ioc.h"
#include "framework/ui/iuiengine.h"

namespace Ms {
//---------------------------------------------------------
//   PaletteQmlInterface
//---------------------------------------------------------

PaletteQmlInterface::PaletteQmlInterface(PaletteWorkspace* workspace, QmlNativeToolTip* t, bool enabled,
                                         QObject* parent)
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
    const bool useSinglePalette = preferences.getBool(PREF_APP_USESINGLEPALETTE);

    QQmlContext* ctx = rootContext();
    Q_ASSERT(ctx);

    QmlNativeToolTip* tooltip = new QmlNativeToolTip(widget());

    qmlInterface = new PaletteQmlInterface(w, tooltip, isEnabled(), this);
    setupStyle();
    ctx->setContextProperty("mscore", qmlInterface);

    setSource(QUrl("qrc:/qml/MuseScore/Palette/PalettesWidget.qml"));

    singlePaletteAction = new QAction(this);
    singlePaletteAction->setCheckable(true);
    singlePaletteAction->setChecked(useSinglePalette);
    addAction(singlePaletteAction);
    connect(singlePaletteAction, &QAction::toggled, this, &PaletteWidget::setSinglePalette);

    setContextMenuPolicy(Qt::ActionsContextMenu);
    setObjectName("palette-widget");
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

    retranslate();
}

PaletteWidget::PaletteWidget(PaletteWorkspace* w, QWidget* parent, Qt::WindowFlags flags)
    : PaletteWidget(w, nullptr, parent, flags)
{}

//---------------------------------------------------------
//   PaletteWidget::setSinglePalette
//---------------------------------------------------------

void PaletteWidget::setSinglePalette(bool val)
{
    preferences.setPreference(PREF_APP_USESINGLEPALETTE, val);
}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void PaletteWidget::retranslate()
{
    setWindowTitle(qApp->translate("Ms::PaletteBox", "Palettes"));
    singlePaletteAction->setText(qApp->translate("Ms::PaletteBox", "Single Palette"));
}

//---------------------------------------------------------
//   setupStyle
//---------------------------------------------------------

void PaletteWidget::setupStyle()
{
    if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR)
        && preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES)) {
        qmlInterface->setPaletteBackground(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
    } else {
        qmlInterface->setPaletteBackground(QColor("#f9f9f9"));
    }
}

//---------------------------------------------------------
//   PaletteWidget::activateSearchBox
//---------------------------------------------------------

void PaletteWidget::activateSearchBox()
{
    ensureQmlViewFocused();
    qmlInterface->requestPaletteSearch();
    adapter()->requestPaletteSearch();
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
    adapter()->notifyElementDraggedToScoreView();
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
        adapter()->setPaletteEnabled(isEnabled());
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
    if (registered) {
        return;
    }

    qmlRegisterUncreatableType<PaletteWorkspace>("MuseScore.Palette", 3, 3, "PaletteWorkspace",
                                                 "Cannot create palette workspace from QML");
    qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 3, 3, "PaletteController",
                                                          "Cannot create palette controller from QML");

    qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 3, 3, "PaletteElementEditor", "");

    qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 3, 3, "PaletteTreeModel",
                                                 "Cannot create palette model from QML");
    qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 3, 3, "FilterPaletteTreeModel",
                                                       "Cannot create palette model from QML");

    qmlRegisterUncreatableType<QmlNativeToolTip>("MuseScore.Palette", 3, 3, "NativeToolTip",
                                                 "Use mscore.palette global variable");

    qmlRegisterType<QmlIconView>("MuseScore.Views", 3, 3, "QmlIconView");

    registered = true;
}
}
