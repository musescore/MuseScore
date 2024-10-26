/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ClassicIndicatorsWindow.h"
#include "core/DropIndicatorOverlay_p.h"
#include "kddockwidgets/core/indicators/ClassicDropIndicatorOverlay.h"
#include "core/Utils_p.h"
#include "qtquick/Platform.h"
#include "View.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets {

static QString iconName(DropLocation loc, bool active)
{
    QString suffix = active ? QStringLiteral("_active") : QString();

    QString name;
    switch (loc) {
    case DropLocation_Center:
        name = QStringLiteral("center");
        break;
    case DropLocation_Left:
        name = QStringLiteral("inner_left");
        break;
    case DropLocation_Right:
        name = QStringLiteral("inner_right");
        break;
    case DropLocation_Bottom:
        name = QStringLiteral("inner_bottom");
        break;
    case DropLocation_Top:
        name = QStringLiteral("inner_top");
        break;
    case DropLocation_OutterLeft:
        name = QStringLiteral("outter_left");
        break;
    case DropLocation_OutterBottom:
        name = QStringLiteral("outter_bottom");
        break;
    case DropLocation_OutterRight:
        name = QStringLiteral("outter_right");
        break;
    case DropLocation_OutterTop:
        name = QStringLiteral("outter_top");
        break;
    case DropLocation_None:
    case DropLocation_Inner:
    case DropLocation_Outter:
    case DropLocation_Horizontal:
    case DropLocation_Vertical:
        return QString();
    }

    return name + suffix;
}
}

ClassicDropIndicatorOverlay::ClassicDropIndicatorOverlay(Core::ClassicDropIndicatorOverlay *classicIndicators, Core::View *parent)
    : QObject(QtQuick::asView_qtquick(parent))
    , m_classicIndicators(classicIndicators)
    , m_window(isWayland() ? nullptr : new IndicatorWindow())
{
    Q_ASSERT(parent);
    classicIndicators->dptr()->hoveredGroupRectChanged.connect([this] { hoveredGroupRectChanged(); });
    classicIndicators->dptr()->currentDropLocationChanged.connect([this] { currentDropLocationChanged(); });

    if (isWayland()) {
        auto subContext = new QQmlContext(plat()->qmlEngine()->rootContext(), this);
        subContext->setContextProperty(QStringLiteral("_kddw_overlayWindow"), this);
        m_overlayItem = QtQuick::View::createItem(qmlSouceUrl().toString(), QtQuick::asView_qtquick(parent), subContext);
        Q_ASSERT(m_overlayItem);
        View::makeItemFillParent(m_overlayItem);
        m_overlayItem->setZ(2);
    } else {
        m_window->rootContext()->setContextProperty(QStringLiteral("_kddw_overlayWindow"), this);
        m_window->init(qmlSouceUrl());
    }
}

ClassicDropIndicatorOverlay::~ClassicDropIndicatorOverlay()
{
    delete m_window;
    delete m_overlayItem;
}

QString ClassicDropIndicatorOverlay::iconName(int loc, bool active) const
{
    return KDDockWidgets::iconName(DropLocation(loc), active);
}

bool ClassicDropIndicatorOverlay::innerLeftIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_Left);
}

bool ClassicDropIndicatorOverlay::innerRightIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_Right);
}

bool ClassicDropIndicatorOverlay::innerTopIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_Top);
}

bool ClassicDropIndicatorOverlay::innerBottomIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_Bottom);
}

bool ClassicDropIndicatorOverlay::outterLeftIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_OutterLeft);
}

bool ClassicDropIndicatorOverlay::outterRightIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_OutterRight);
}

bool ClassicDropIndicatorOverlay::outterTopIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_OutterTop);
}

bool ClassicDropIndicatorOverlay::outterBottomIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_OutterBottom);
}

bool ClassicDropIndicatorOverlay::tabIndicatorVisible() const
{
    return m_classicIndicators->dropIndicatorVisible(DropLocation_Center);
}

QRect ClassicDropIndicatorOverlay::hoveredGroupRect() const
{
    return m_classicIndicators->hoveredGroupRect();
}

DropLocation ClassicDropIndicatorOverlay::currentDropLocation() const
{
    return m_classicIndicators->currentDropLocation();
}

DropLocation ClassicDropIndicatorOverlay::hover(QPoint pt)
{
    QQuickItem *item = indicatorForPos(pt);
    return item ? locationForIndicator(item) : DropLocation_None;
}

void ClassicDropIndicatorOverlay::updateIndicatorVisibility()
{
    Q_EMIT indicatorsVisibleChanged();
}

QQuickItem *ClassicDropIndicatorOverlay::indicatorForPos(QPoint pt) const
{
    const QVector<QQuickItem *> indicators = indicatorItems();
    Q_ASSERT(indicators.size() == 9);

    for (QQuickItem *item : indicators) {
        if (item->isVisible()) {
            QRect rect(0, 0, int(item->width()), int(item->height()));
            rect.moveTopLeft(item->mapToGlobal(QPointF(0, 0)).toPoint());
            if (rect.contains(pt)) {
                return item;
            }
        }
    }

    return nullptr;
}

void ClassicDropIndicatorOverlay::updatePositions()
{
    // Not needed to implement, the Indicators use QML anchors
}

QPoint ClassicDropIndicatorOverlay::posForIndicator(KDDockWidgets::DropLocation loc) const
{
    QQuickItem *indicator = ClassicDropIndicatorOverlay::indicatorForLocation(loc);
    return indicator->mapToGlobal(indicator->boundingRect().center()).toPoint();
}

IndicatorWindow::IndicatorWindow()
{
    setFlags(flags() | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint | Qt::Tool);
    setColor(Qt::transparent);
}

IndicatorWindow::~IndicatorWindow() = default;

void IndicatorWindow::init(const QUrl &rootQml)
{
    setSource(rootQml);

    // Two workarounds for two unrelated bugs:
    if (KDDockWidgets::isOffscreen()) {
        // 1. We need to create the window asap, otherwise, if a drag triggers the indicator window
        // to show, that creates a QOffscreenWindow, which flushes events in the ctor, triggering
        // more hover events which will trigger another QOffscreenWindow.
        // We then end up with a QWindow with two QPlatformWindow, and only one is deleted in
        // at shutdown, meaning some timers aren't unregistered, meaning we get a crash when
        // the timer event is sent to the destroyed QWindow.
        create();
    } else {
        // 2.
        // Small hack to avoid flickering when we drag over a window the first time
        // Not sure why a simply create() doesn't work instead
        // Not if offscreen though, as that QPA is flaky with window activation/focus
        resize(QSize(1, 1));
        show();
        hide();
    }
}

QUrl ClassicDropIndicatorOverlay::qmlSouceUrl() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/ClassicIndicatorsOverlay.qml"));
}

QQuickItem *ClassicDropIndicatorOverlay::indicatorForLocation(DropLocation loc) const
{
    const QVector<QQuickItem *> indicators = indicatorItems();
    Q_ASSERT(indicators.size() == 9);

    for (QQuickItem *item : indicators) {
        if (locationForIndicator(item) == loc)
            return item;
    }

    qWarning() << Q_FUNC_INFO << "Couldn't find indicator for location" << loc;
    return nullptr;
}

DropLocation ClassicDropIndicatorOverlay::locationForIndicator(const QQuickItem *item) const
{
    return DropLocation(item->property("indicatorType").toInt());
}

QVector<QQuickItem *> ClassicDropIndicatorOverlay::indicatorItems() const
{
    QVector<QQuickItem *> indicators;
    indicators.reserve(9);

    QQuickItem *root = m_window ? m_window->rootObject() : m_overlayItem;
    const QList<QQuickItem *> items = root->childItems();
    for (QQuickItem *item : items) {
        if (QString::fromLatin1(item->metaObject()->className())
                .startsWith(QLatin1String("ClassicIndicator_QMLTYPE"))) {
            indicators.push_back(item);
        } else if (item->objectName() == QLatin1String("innerIndicators")) {
            const QList<QQuickItem *> innerIndicators = item->childItems();
            for (QQuickItem *innerItem : innerIndicators) {
                if (QString::fromLatin1(innerItem->metaObject()->className())
                        .startsWith(QLatin1String("ClassicIndicator_QMLTYPE"))) {
                    indicators.push_back(innerItem);
                }
            }
        }
    }

    return indicators;
}
void ClassicDropIndicatorOverlay::raise()
{
    if (m_window) {
        m_window->raise();
    } else {
        // The wayland item has an higher Z, no need to raise
    }
}

void ClassicDropIndicatorOverlay::setGeometry(QRect geo)
{
    if (m_window) {
        m_window->setGeometry(geo);
    } else {
        // The wayland item has "anchors.fill: parent", no need to resize
    }
}

void ClassicDropIndicatorOverlay::setObjectName(const QString &name)
{
    QObject::setObjectName(name);
}

void ClassicDropIndicatorOverlay::setVisible(bool is)
{
    if (m_window) {
        m_window->setVisible(is);
    } else {
        // Wayland case
        m_overlayItem->setVisible(is);
    }
}

void ClassicDropIndicatorOverlay::resize(QSize sz)
{
    if (m_window) {
        m_window->resize(sz);
    } else {
        // The wayland item has "anchors.fill: parent", no need to resize
    }
}

bool ClassicDropIndicatorOverlay::isWindow() const
{
    return m_window != nullptr;
}
