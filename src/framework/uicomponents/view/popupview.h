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

#ifndef MU_UICOMPONENTS_POPUPVIEW_H
#define MU_UICOMPONENTS_POPUPVIEW_H

#include <QQuickItem>
#include <QQmlParserStatus>

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

namespace mu::uicomponents {
class PopupWindow;
class PopupView : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QQuickItem * parent READ parentItem WRITE setParentItem NOTIFY parentItemChanged)
    Q_PROPERTY(QQuickItem * contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)

    //! NOTE Local, related parent
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)

    Q_PROPERTY(bool isOpened READ isOpened NOTIFY isOpenedChanged)
    Q_PROPERTY(ClosePolicy closePolicy READ closePolicy WRITE setClosePolicy NOTIFY closePolicyChanged)

    Q_ENUMS(ClosePolicy)

    INJECT(uicomponents, ui::IMainWindow, mainWindow)

public:

    explicit PopupView(QQuickItem* parent = nullptr);

    enum ClosePolicy {
        NoAutoClose = 0,
        CloseOnPressOutsideParent,
        CloseOnReleaseOutsideParent
    };

    QQuickItem* parentItem() const;
    QQuickItem* contentItem() const;

    qreal x() const;
    qreal y() const;

    Q_INVOKABLE void forceActiveFocus();

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void toggleOpened();

    ClosePolicy closePolicy() const;

    bool isOpened() const;

public slots:
    void setParentItem(QQuickItem* parent);
    void setContentItem(QQuickItem* content);
    void setX(qreal x);
    void setY(qreal y);
    void setClosePolicy(ClosePolicy closePolicy);

signals:
    void parentItemChanged();
    void contentItemChanged();
    void xChanged(qreal x);
    void yChanged(qreal y);
    void closePolicyChanged(ClosePolicy closePolicy);

    void isOpenedChanged();
    void opened();
    void closed();

private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);

private:
    void classBegin() override;
    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    bool isMouseWithinBoundaries(const QPoint& mousePos) const;

    PopupWindow* m_window = nullptr;
    QQuickItem* m_contentItem = nullptr;

    QPointF m_localPos;
    ClosePolicy m_closePolicy = ClosePolicy::CloseOnPressOutsideParent;
};
}

#endif // MU_UICOMPONENTS_POPUPVIEW_H
