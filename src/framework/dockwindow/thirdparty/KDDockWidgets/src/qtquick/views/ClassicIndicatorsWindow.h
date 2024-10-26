/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_CLASSICINDICATORS_WINDOW_QTQUICK_H
#define KD_CLASSICINDICATORS_WINDOW_QTQUICK_H
#pragma once

#include "kddockwidgets/core/views/ClassicIndicatorWindowViewInterface.h"

#include <QQuickView>

namespace KDDockWidgets {

namespace Core {
class ClassicDropIndicatorOverlay;
class View;
}

namespace QtQuick {

class ClassicDropIndicatorOverlay;

class IndicatorWindow : public QQuickView
{
    Q_OBJECT
public:
    explicit IndicatorWindow();
    ~IndicatorWindow() override;

    void init(const QUrl &rootQml);
};

class ClassicDropIndicatorOverlay : public QObject, public Core::ClassicIndicatorWindowViewInterface
{
    Q_OBJECT
    Q_PROPERTY(bool innerLeftIndicatorVisible READ innerLeftIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool innerRightIndicatorVisible READ innerRightIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(
        bool innerTopIndicatorVisible READ innerTopIndicatorVisible NOTIFY indicatorsVisibleChanged)
    Q_PROPERTY(bool innerBottomIndicatorVisible READ innerBottomIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool outterLeftIndicatorVisible READ outterLeftIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool outterRightIndicatorVisible READ outterRightIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool outterTopIndicatorVisible READ outterTopIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool outterBottomIndicatorVisible READ outterBottomIndicatorVisible NOTIFY
                   indicatorsVisibleChanged)
    Q_PROPERTY(bool tabIndicatorVisible READ tabIndicatorVisible NOTIFY indicatorsVisibleChanged)
    Q_PROPERTY(QRect hoveredGroupRect READ hoveredGroupRect NOTIFY hoveredGroupRectChanged)
    Q_PROPERTY(KDDockWidgets::DropLocation currentDropLocation READ currentDropLocation NOTIFY
                   currentDropLocationChanged)
public:
    explicit ClassicDropIndicatorOverlay(Core::ClassicDropIndicatorOverlay *, Core::View *parent);
    ~ClassicDropIndicatorOverlay() override;

    Q_INVOKABLE QString iconName(int loc, bool active) const;

    // Property getters for QML:
    bool innerLeftIndicatorVisible() const;
    bool innerRightIndicatorVisible() const;
    bool innerTopIndicatorVisible() const;
    bool innerBottomIndicatorVisible() const;
    bool outterLeftIndicatorVisible() const;
    bool outterRightIndicatorVisible() const;
    bool outterTopIndicatorVisible() const;
    bool outterBottomIndicatorVisible() const;
    bool tabIndicatorVisible() const;
    QRect hoveredGroupRect() const;
    DropLocation currentDropLocation() const;

    DropLocation hover(QPoint globalPos) override;
    void updatePositions() override;
    QPoint posForIndicator(DropLocation) const override;
    void raise() override;
    void setVisible(bool) override;
    bool isWindow() const override;
    void setGeometry(QRect) override;
    void resize(QSize) override;
    void setObjectName(const QString &) override;
    void updateIndicatorVisibility() override;

    QQuickItem *indicatorForLocation(DropLocation loc) const;

Q_SIGNALS:
    void indicatorsVisibleChanged();
    void hoveredGroupRectChanged();
    void currentDropLocationChanged();

private:
    Core::ClassicDropIndicatorOverlay *const m_classicIndicators;
    DropLocation locationForIndicator(const QQuickItem *) const;
    QQuickItem *indicatorForPos(QPoint) const;
    QVector<QQuickItem *> indicatorItems() const;
    QUrl qmlSouceUrl() const;

    IndicatorWindow *const m_window;
    QQuickItem *m_overlayItem = nullptr;
};

}

}

#endif
