/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_MULTISPLITTER_SEPARATOR_QUICK_H
#define KD_MULTISPLITTER_SEPARATOR_QUICK_H
#pragma once

#include "View.h"
#include "kddockwidgets/docks_export.h"

#include <QQuickItem>

namespace KDDockWidgets::Core {
class Separator;
}

namespace KDDockWidgets {

namespace QtQuick {

class DOCKS_EXPORT Separator : public QtQuick::View
{
    Q_OBJECT
    Q_PROPERTY(bool isVertical READ isVertical NOTIFY isVerticalChanged)
public:
    explicit Separator(Core::Separator *controller, QQuickItem *parent = nullptr);
    bool isVertical() const;

public:
    // Interface with QML:
    Q_INVOKABLE void onMousePressed();
    Q_INVOKABLE void onMouseMoved(QPointF localPos);
    Q_INVOKABLE void onMouseReleased();
    Q_INVOKABLE void onMouseDoubleClicked();
Q_SIGNALS:
    // constant but it's only set after Separator::init
    void isVerticalChanged();

private:
    void init() override final;
    QSize minSize() const override;
    Core::Separator *const m_controller;
};

}

}

#endif
