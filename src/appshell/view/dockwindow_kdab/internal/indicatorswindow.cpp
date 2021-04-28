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

#include "indicatorswindow.h"

#include "dropindicators.h"

#include "thirdparty/KDDockWidgets/src/private/Utils_p.h"

#include <QQmlContext>

using namespace mu::dock;

IndicatorsWindow::IndicatorsWindow(DropIndicators* dropIndicators)
    : QQuickView(),
    m_dropIndicators(dropIndicators)
{
    setFlags(flags() | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint | Qt::Tool);
    setColor(Qt::transparent);

    rootContext()->setContextProperty(QStringLiteral("_window"), QVariant::fromValue<QObject*>(this));
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

KDDockWidgets::DropIndicatorOverlayInterface::DropLocation IndicatorsWindow::hover(QPoint pos)
{
    QQuickItem* item = indicatorForPos(pos);
    auto location = item ? locationForIndicator(item) : KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_None;
    dropIndicators()->setDropLocation(location);
    return location;
}

QQuickItem* IndicatorsWindow::indicatorForPos(QPoint pos) const
{
    const QVector<QQuickItem*> indicators = indicatorItems();
    Q_ASSERT(indicators.size() == 9);

    for (QQuickItem* item : indicators) {
        if (item->isVisible()) {
            QRect rect(0, 0, int(item->width()), int(item->height()));
            rect.moveTopLeft(item->mapToGlobal(QPointF(0, 0)).toPoint());
            if (rect.contains(pos)) {
                return item;
            }
        }
    }

    return nullptr;
}

QPoint IndicatorsWindow::posForIndicator(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation loc) const
{
    QQuickItem* indicator = IndicatorsWindow::indicatorForLocation(loc);
    return indicator->mapToGlobal(indicator->boundingRect().center()).toPoint();
}

QString IndicatorsWindow::iconName(int loc, bool active) const
{
    QString suffix = active ? QStringLiteral("_active")
                     : QString();

    QString name;
    switch (loc) {
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Center:
        name = QStringLiteral("center");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Left:
        name = QStringLiteral("inner_left");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Right:
        name = QStringLiteral("inner_right");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Bottom:
        name = QStringLiteral("inner_bottom");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_Top:
        name = QStringLiteral("inner_top");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterLeft:
        name = QStringLiteral("outter_left");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterBottom:
        name = QStringLiteral("outter_bottom");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterRight:
        name = QStringLiteral("outter_right");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_OutterTop:
        name = QStringLiteral("outter_top");
        break;
    case KDDockWidgets::DropIndicatorOverlayInterface::DropLocation_None:
        return QString();
    }

    return name + suffix;
}

DropIndicators* IndicatorsWindow::dropIndicators() const
{
    return m_dropIndicators;
}

QQuickItem* IndicatorsWindow::indicatorForLocation(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation loc) const
{
    const QVector<QQuickItem*> indicators = indicatorItems();
    Q_ASSERT(indicators.size() == 9);

    for (QQuickItem* item : indicators) {
        if (locationForIndicator(item) == loc) {
            return item;
        }
    }

    qWarning() << Q_FUNC_INFO << "Couldn't find indicator for location" << loc;
    return nullptr;
}

KDDockWidgets::DropIndicatorOverlayInterface::DropLocation IndicatorsWindow::locationForIndicator(const QQuickItem* indicator) const
{
    return KDDockWidgets::DropIndicatorOverlayInterface::DropLocation(indicator->property("indicatorType").toInt());
}

QVector<QQuickItem*> IndicatorsWindow::indicatorItems() const
{
    QVector<QQuickItem*> indicators;
    indicators.reserve(9);

    QQuickItem* root = rootObject();
    const QList<QQuickItem*> items = root->childItems();
    for (QQuickItem* item : items) {
        if (QString::fromLatin1(item->metaObject()->className()).startsWith(QLatin1String("DropIndicator_QMLTYPE"))) {
            indicators.push_back(item);
        } else if (item->objectName() == QLatin1String("innerIndicators")) {
            const QList<QQuickItem*> innerIndicators = item->childItems();
            for (QQuickItem* innerItem : innerIndicators) {
                if (QString::fromLatin1(innerItem->metaObject()->className()).startsWith(QLatin1String("DropIndicator_QMLTYPE"))) {
                    indicators.push_back(innerItem);
                }
            }
        }
    }

    return indicators;
}
