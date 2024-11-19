/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_VIEW_QTQUICK_H
#define KD_VIEW_QTQUICK_H

#pragma once

#include "kddockwidgets/core/Controller.h"
#include "kddockwidgets/qtcommon/View.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickView>
#include <QScreen>

#include <memory>

namespace KDDockWidgets::QtQuick {

class MouseEventRedirector;

inline QQuickItem *asQQuickItem(Core::View *view)
{
    if (!view)
        return nullptr;

    return qobject_cast<QQuickItem *>(QtCommon::View_qt::asQObject(view));
}

inline QQuickItem *asQQuickItem(Core::Controller *controller)
{
    if (!controller)
        return nullptr;

    return asQQuickItem(controller->view());
}

DOCKS_EXPORT QQmlContext *qmlContextFor(QQuickItem *);

class DOCKS_EXPORT View : public QQuickItem, public QtCommon::View_qt
{
    Q_OBJECT
    Q_PROPERTY(bool isFixedHeight READ isFixedHeight NOTIFY geometryUpdated)
    Q_PROPERTY(bool isFixedWidth READ isFixedWidth NOTIFY geometryUpdated)
public:
    using Core::View::close;
    using Core::View::height;
    using Core::View::rect;
    using Core::View::resize;
    using Core::View::width;

    explicit View(Core::Controller *controller, Core::ViewType type,
                  QQuickItem *parent = nullptr, Qt::WindowFlags windowFlags = {});

    ~View() override = default;

    QSize minSize() const override;
    QSize maxSizeHint() const override;
    QRect geometry() const override;
    QRect normalGeometry() const override;
    void setNormalGeometry(QRect geo);
    void setGeometry(QRect) override;
    void setMaximumSize(QSize sz) override;

    bool isVisible() const override;
    bool isExplicitlyHidden() const override;
    void setVisible(bool is) override;

    void move(int x, int y) override;
    void setSize(int w, int h) override final;

    void setWidth(int w) override;
    void setHeight(int h) override;
    void setFixedWidth(int w) override;
    void setFixedHeight(int h) override;
    void setFixedSize(Size);
    void show() override;
    void hide() override;
    void updateGeometry();
    void update() override;
    void setParent(Core::View *parent) override;
    void setParent(QQuickItem *parent);
    static void raiseAndActivate(QQuickItem *);
    void raiseAndActivate() override;
    void activateWindow() override;
    void raise() override;
    static bool isRootView(const QQuickItem *);
    bool isRootView() const override final;
    QQuickView *quickView() const;
    QPoint mapToGlobal(QPoint localPt) const override;
    QPoint mapFromGlobal(QPoint globalPt) const override;
    QPoint mapTo(Core::View *parent, QPoint pos) const override;
    void setWindowOpacity(double v) override;

    static bool close(QQuickItem *);
    bool close() override;
    void setFlag(Qt::WindowType f, bool on = true) override;
    void enableAttribute(Qt::WidgetAttribute attr, bool enable = true) override;
    bool hasAttribute(Qt::WidgetAttribute attr) const override;
    Qt::WindowFlags flags() const override;

    void setWindowTitle(const QString &title) override;
    void setWindowIcon(const QIcon &icon) override;
    bool isActiveWindow() const override;

    Q_INVOKABLE void redirectMouseEvents(QQuickItem *from);
    Q_INVOKABLE void showNormal() override;
    Q_INVOKABLE void showMinimized() override;
    Q_INVOKABLE void showMaximized() override;

    bool isMinimized() const override;
    bool isMaximized() const override;

    std::shared_ptr<Core::Window> window() const override;
    std::shared_ptr<Core::View> childViewAt(QPoint p) const override;
    std::shared_ptr<Core::View> rootView() const override;
    std::shared_ptr<Core::View> parentView() const override;

    std::shared_ptr<Core::View> asWrapper() override;

    void grabMouse() override;
    void releaseMouse() override;
    void releaseKeyboard() override;
    void setFocus(Qt::FocusReason reason) override;
    Qt::FocusPolicy focusPolicy() const override;
    bool hasFocus() const override;
    void setFocusPolicy(Qt::FocusPolicy) override;
    QString viewName() const override;
    void setMinimumSize(QSize sz) override;
    void render(QPainter *) override;
    void setCursor(Qt::CursorShape shape) override;
    void setMouseTracking(bool enable) override;
    QVector<std::shared_ptr<Core::View>> childViews() const override;
    int zOrder() const override;
    void setZOrder(int) override;
    virtual QQuickItem *visualItem() const;

    void onWindowStateChangeEvent(QWindowStateChangeEvent *);

    bool isFixedWidth() const;
    bool isFixedHeight() const;

    /// @brief This is equivalent to "anchors.fill: parent but in C++
    static void makeItemFillParent(QQuickItem *item);

    /// @brief Convenience to create a QQuickItem
    static QQuickItem *createItem(QQmlEngine *engine, const QString &filename, QQmlContext *context = nullptr);

    /// @brief Convenience to create a QQuickItem parented to @p parent.
    /// Uses the engine from the parent
    static QQuickItem *createItem(const QString &filename, QQuickItem *parent, QQmlContext *ctx = nullptr);

    static std::shared_ptr<Core::View> parentViewFor(const QQuickItem *);
    static std::shared_ptr<Core::View> asQQuickWrapper(QQuickItem *item);

Q_SIGNALS:
    void geometryUpdated(); // similar to QLayout stuff, when size constraints change
    void itemGeometryChanged(); // emitted when the geometry changes. QQuickItem::geometryChanged()
                                // isn't a signal, so prefixed item

protected:
    void QQUICKITEMgeometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(QQuickItem::ItemChange, const QQuickItem::ItemChangeData &) override;
    bool eventFilter(QObject *watched, QEvent *ev) override;
    bool event(QEvent *) override;

    /// Override in case you want to have different styling on hover
    virtual void onHoverEvent(QHoverEvent *, QPoint /*globalPos*/)
    {
    }

private:
    Q_DISABLE_COPY(View)
    friend class MouseEventRedirector;
    void updateNormalGeometry();
    void sendVisibleChangeEvent();
    void _setSize(QSize);
    bool m_inSetParent = false;
    Qt::WindowFlags m_windowFlags;
    int m_widgetAttributes = 0; // Qt::WidgetAttribute
    Qt::FocusPolicy m_focusPolicy = Qt::NoFocus;
    bool m_mouseTrackingEnabled = false;
    QRect m_normalGeometry;
    Qt::WindowStates m_oldWindowState = Qt::WindowState::WindowNoState;
    MouseEventRedirector *m_mouseEventRedirector = nullptr;
};

inline View *asView_qtquick(Core::View *view)
{
    if (!view)
        return nullptr;
    return static_cast<View *>(view);
}

inline View *asView_qtquick(Core::Controller *controller)
{
    if (!controller)
        return nullptr;

    return static_cast<View *>(controller->view());
}

inline qreal logicalDpiFactor(const QQuickItem *item)
{
#ifndef Q_OS_MACOS
    if (QQuickWindow *window = item->window()) {
        if (QScreen *s = window->screen()) {
            return s->logicalDotsPerInch() / 96.0;
        }
    }
#endif

    // It's always 72 on mac
    Q_UNUSED(item);
    return 1;
}

}

#endif
