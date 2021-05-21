/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "dropindicatorswindow.h"

#include "dropindicators.h"

#include "thirdparty/KDDockWidgets/src/private/Utils_p.h"

#include <QQmlContext>

using namespace mu::dock;

static constexpr int INDICATORS_COUNT = 9;

DropIndicatorsWindow::DropIndicatorsWindow(QObject* dropIndicators)
    : QQuickView(),
    m_dropIndicators(dropIndicators)
{
    setFlags(flags() | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint | Qt::Tool);
    setColor(Qt::transparent);

    rootContext()->setContextProperty(QStringLiteral("dockDropIndicatorsWindow"), QVariant::fromValue<QObject*>(this));
    setSource(QUrl(QStringLiteral("qrc:/qml/kdab/docksystem/DropIndicators.qml")));

    {
        // Small hack to avoid flickering when we drag over a window the first time
        // Not sure why a simply create() doesn't work instead
        // Not if offscreen though, as that QPA is flaky with window activation/focus
        if (!KDDockWidgets::isOffscreen()) {
            resize(QSize(1, 1));
            show();
            hide();
        }
    }
}

QObject* DropIndicatorsWindow::dropIndicators() const
{
    return m_dropIndicators;
}

QString DropIndicatorsWindow::iconName(int dropLocation, bool active) const
{
    QString suffix = active ? "_active" : QString();
    QString name;

    switch (dropLocation) {
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Center:
        name = "center";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Left:
        name = "inner_left";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Right:
        name = "inner_right";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Bottom:
        name = "inner_bottom";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Top:
        name = "inner_top";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterLeft:
        name = "outter_left";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterBottom:
        name = "outter_bottom";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterRight:
        name = "outter_right";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterTop:
        name = "outter_top";
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_None:
        return QString();
    }

    return name + suffix;
}

KDDockWidgets::DropIndicatorOverlayInterface::DropLocation DropIndicatorsWindow::dropLocationForPosition(const QPoint& pos) const
{
    const QQuickItem* item = indicatorForPos(pos);
    return item ? locationForIndicator(item) : KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_None;
}

const QQuickItem* DropIndicatorsWindow::indicatorForPos(QPoint pos) const
{
    for (const QQuickItem* item : indicatorItems()) {
        if (!item->isVisible()) {
            continue;
        }

        QRect rect(0, 0, int(item->width()), int(item->height()));
        rect.moveTopLeft(item->mapToGlobal(QPointF(0, 0)).toPoint());

        if (rect.contains(pos)) {
            return item;
        }
    }

    return nullptr;
}

QPoint DropIndicatorsWindow::positionForIndicator(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation dropLocation) const
{
    const QQuickItem* indicator = DropIndicatorsWindow::indicatorForLocation(dropLocation);
    return indicator->mapToGlobal(indicator->boundingRect().center()).toPoint();
}

KDDockWidgets::DropIndicatorOverlayInterface::DropLocation DropIndicatorsWindow::locationForIndicator(const QQuickItem* indicator) const
{
    return KDDockWidgets::DropIndicatorOverlayInterface::DropLocation(indicator->property("indicatorType").toInt());
}

const QQuickItem* DropIndicatorsWindow::indicatorForLocation(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation dropLocation) const
{
    for (const QQuickItem* item : indicatorItems()) {
        if (locationForIndicator(item) == dropLocation) {
            return item;
        }
    }

    return nullptr;
}

QList<const QQuickItem*> DropIndicatorsWindow::indicatorItems() const
{
    QList<const QQuickItem*> indicators;
    indicators.reserve(INDICATORS_COUNT);

    QList<QQuickItem*> items = rootObject()->childItems();

    for (const QQuickItem* item : items) {
        if (QString::fromLatin1(item->metaObject()->className()).startsWith(QLatin1String("DropIndicator_QMLTYPE"))) {
            indicators.push_back(item);
        } else if (item->objectName() == "innerIndicators") {
            const QList<QQuickItem*> innerIndicators = item->childItems();

            for (QQuickItem* innerItem : innerIndicators) {
                if (QString::fromLatin1(innerItem->metaObject()->className()).startsWith(QLatin1String("DropIndicator_QMLTYPE"))) {
                    indicators.push_back(innerItem);
                }
            }
        }
    }

    Q_ASSERT(indicators.size() == INDICATORS_COUNT);

    return indicators;
}
