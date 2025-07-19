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
#ifndef MUSE_UI_QMLTOOLTIP_H
#define MUSE_UI_QMLTOOLTIP_H

#include <QObject>
#include <QQuickItem>
#include <QTimer>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveprovider.h"
#include "ui/iuiconfiguration.h"

namespace muse::ui {
class QmlToolTip : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Inject<IInteractiveProvider> interactiveProvider = { this };
    Inject<IUiConfiguration> uiConfiguration = { this };

public:
    explicit QmlToolTip(QObject* parent, const modularity::ContextPtr& iocCtx);

    Q_INVOKABLE void show(QQuickItem* item, const QString& title, const QString& description = "", const QString& shortcut = "");
    Q_INVOKABLE void hide(QQuickItem* item, bool force = false);

    Q_INVOKABLE void init();

private slots:
    void doShow();
    void onItemDestruction();
    void doHide();

signals:
    void showToolTip(QQuickItem* item, const QString& title, const QString& description, const QString& shortcut);
    void hideToolTip();

private:
    friend class QmlToolTipTests;

    bool eventFilter(QObject* watched, QEvent* event) override;

    void clear();

    QQuickItem* m_item = nullptr;
    QString m_title;
    QString m_description;
    QString m_shortcut;

    QTimer m_openTimer;
    QTimer m_closeTimer;
    bool m_shouldBeClosed = false;
};
}

#endif // MUSE_UI_QMLTOOLTIP_H
