/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_VIEW_QTWIDGETS_H
#define KD_VIEW_QTWIDGETS_H

#pragma once

#include "kddockwidgets/core/Controller.h"
#include "kddockwidgets/qtcommon/View.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QWidget>
#include <QApplication>
#include <QWindow>

#include <memory>

namespace KDDockWidgets::QtWidgets {

DOCKS_EXPORT QSize boundedMaxSize(QSize min, QSize max);

template<typename Base>
class DOCKS_EXPORT View : public Base, public QtCommon::View_qt
{
public:
    using Core::View::close;
    using Core::View::height;
    using Core::View::minimumHeight;
    using Core::View::minimumWidth;
    using Core::View::move;
    using Core::View::pos;
    using Core::View::rect;
    using Core::View::resize;
    using Core::View::setSize;
    using Core::View::size;
    using Core::View::width;
    using Core::View::x;
    using Core::View::y;

    explicit View(Core::Controller *controller, Core::ViewType type,
                  QWidget *parent = nullptr, Qt::WindowFlags windowFlags = {});

    ~View() override = default;

    QSize minSize() const override
    {
        const int minW =
            Base::minimumWidth() > 0 ? Base::minimumWidth() : minimumSizeHint().width();

        const int minH =
            Base::minimumHeight() > 0 ? Base::minimumHeight() : minimumSizeHint().height();

        return QSize(minW, minH).expandedTo(Core::View::hardcodedMinimumSize());
    }

    QSize minimumSizeHint() const override
    {
        return Base::minimumSizeHint();
    }

    void setMinimumSize(QSize sz) override;

    QSize maxSizeHint() const override
    {
        // The max size is usually QWidget::maximumSize(), but we also honour the
        // QSizePolicy::Fixed+sizeHint() case as widgets don't need to have QWidget::maximumSize()
        // to have a max size honoured

        const QSize min = minSize();
        QSize max = Base::maximumSize();
        max = QtWidgets::boundedMaxSize(min, max); // for safety against weird values

        const auto vPolicy = QWidget::sizePolicy().verticalPolicy();
        const auto hPolicy = QWidget::sizePolicy().horizontalPolicy();

        if (vPolicy == QSizePolicy::Fixed || vPolicy == QSizePolicy::Maximum)
            max.setHeight(qMin(max.height(), Base::sizeHint().height()));
        if (hPolicy == QSizePolicy::Fixed || hPolicy == QSizePolicy::Maximum)
            max.setWidth(qMin(max.width(), Base::sizeHint().width()));

        max = QtWidgets::boundedMaxSize(min, max); // for safety against weird values
        return max;
    }

    QRect geometry() const override
    {
        return Base::geometry();
    }

    QRect normalGeometry() const override
    {
        return Base::normalGeometry();
    }

    void setGeometry(QRect geo) override
    {
        Base::setGeometry(geo);
    }

    void setMaximumSize(QSize sz) override;

    bool isVisible() const override
    {
        return Base::isVisible();
    }

    bool isExplicitlyHidden() const override
    {
        return Base::isHidden();
    }

    void setVisible(bool is) override
    {
        Base::setVisible(is);
    }

    void move(int x, int y) override
    {
        Base::move(x, y);
    }

    void setSize(int width, int height) override
    {
        Base::resize(width, height);
    }

    void setWidth(int width) override
    {
        setSize(width, QWidget::height());
    }

    void setHeight(int height) override
    {
        setSize(QWidget::width(), height);
    }

    void setFixedWidth(int w) override
    {
        QWidget::setFixedWidth(w);
    }

    void setFixedHeight(int h) override
    {
        QWidget::setFixedHeight(h);
    }

    void show() override
    {
        Base::show();
    }

    void createPlatformWindow() override
    {
        QWidget::create();
    }

    void hide() override
    {
        Base::hide();
    }

    void update() override
    {
        Base::update();
    }

    int zOrder() const override
    {
        if (auto p = QWidget::parentWidget()) {
            // Some of them might be non-QtWidget QObject, but this is good enough
            // to unit test raise() in MDI area
            // TODO: Remove cast to int, and make method return qsizetype after we add /WX
            return int(p->children().indexOf(const_cast<QtWidgets::View<Base> *>(this)));
        }

        return 0;
    }

    static void setParentFor(QWidget *widget, Core::View *parent)
    {
        if (!parent) {
            widget->QWidget::setParent(nullptr);
            return;
        }

        if (auto qwidget = View_qt::asQWidget(parent)) {
            widget->QWidget::setParent(qwidget);
        } else {
            qWarning() << Q_FUNC_INFO << "parent is not a widget, you have a bug";
            Q_ASSERT(false);
        }
    }

    void setParent(Core::View *parent) override
    {
        setParentFor(this, parent);
    }

    static void raiseAndActivate(QWidget *w)
    {
        w->window()->raise();
        const bool isWayland = qApp->platformName() == QLatin1String("wayland");
        if (!isWayland)
            w->window()->activateWindow();
    }

    void raiseAndActivate() override
    {
        raiseAndActivate(this);
    }

    void activateWindow() override
    {
        Base::activateWindow();
    }

    void raise() override
    {
        Base::raise();
    }

    bool isRootView() const override
    {
        return QWidget::isWindow();
    }

    QPoint mapToGlobal(QPoint localPt) const override
    {
        return Base::mapToGlobal(localPt);
    }

    QPoint mapFromGlobal(QPoint globalPt) const override
    {
        return Base::mapFromGlobal(globalPt);
    }

    QPoint mapTo(Core::View *someAncestor, QPoint pos) const override
    {
        return QWidget::mapTo(View_qt::asQWidget(someAncestor), pos);
    }

    void setWindowOpacity(double v) override
    {
        QWidget::setWindowOpacity(v);
    }

    void render(QPainter *p) override
    {
        Base::render(p);
    }

    void setCursor(Qt::CursorShape shape) override
    {
        QWidget::setCursor(shape);
    }

    void setMouseTracking(bool enable) override
    {
        QWidget::setMouseTracking(enable);
    }

    bool close() override
    {
        return QWidget::close();
    }

    void setFlag(Qt::WindowType flag, bool on = true) override
    {
        QWidget::setWindowFlag(flag, on);
    }

    void enableAttribute(Qt::WidgetAttribute attr, bool enable = true) override
    {
        QWidget::setAttribute(attr, enable);
    }

    bool hasAttribute(Qt::WidgetAttribute attr) const override
    {
        return QWidget::testAttribute(attr);
    }

    Qt::WindowFlags flags() const override
    {
        return QWidget::windowFlags();
    }

    void setWindowTitle(const QString &title) override
    {
        QWidget::setWindowTitle(title);
    }

    void setWindowIcon(const QIcon &icon) override
    {
        QWidget::setWindowIcon(icon);
    }

    bool isActiveWindow() const override
    {
        return QWidget::isActiveWindow();
    }

    void showNormal() override
    {
        return QWidget::showNormal();
    }

    void showMinimized() override
    {
        return QWidget::showMinimized();
    }

    void showMaximized() override
    {
        return QWidget::showMaximized();
    }

    bool isMinimized() const override
    {
        return QWidget::isMinimized();
    }

    bool isMaximized() const override
    {
        return QWidget::isMaximized();
    }

    Qt::FocusPolicy focusPolicy() const override
    {
        return QWidget::focusPolicy();
    }

    bool hasFocus() const override
    {
        return QWidget::hasFocus();
    }

    std::shared_ptr<Core::View> childViewAt(QPoint localPos) const override;

    std::shared_ptr<Core::Window> window() const override;

    std::shared_ptr<Core::View> rootView() const override;

    std::shared_ptr<Core::View> parentView() const override;

    std::shared_ptr<Core::View> asWrapper() override;

    void grabMouse() override
    {
        QWidget::grabMouse();
    }

    void releaseMouse() override
    {
        QWidget::releaseMouse();
    }

    void releaseKeyboard() override
    {
        QWidget::releaseKeyboard();
    }

    void setFocus(Qt::FocusReason reason) override
    {
        QWidget::setFocus(reason);
    }

    void setFocusPolicy(Qt::FocusPolicy policy) override
    {
        QWidget::setFocusPolicy(policy);
    }

    QString viewName() const override
    {
        return QWidget::objectName();
    }

    static QVector<std::shared_ptr<Core::View>> childViewsFor(const QWidget *parent);

    QVector<std::shared_ptr<Core::View>> childViews() const override
    {
        return childViewsFor(this);
    }

protected:
    bool event(QEvent *e) override;
    void closeEvent(QCloseEvent *ev) override;

    void resizeEvent(QResizeEvent *ev) override
    {
        if (!onResize(ev->size()))
            Base::resizeEvent(ev);
    }

private:
    Q_DISABLE_COPY(View)
};

inline qreal logicalDpiFactor(const QWidget *w)
{
#ifdef DOCKS_DEVELOPER_MODE
    if (QtCommon::View_qt::s_logicalDpiFactorOverride > 0)
        return QtCommon::View_qt::s_logicalDpiFactorOverride;
#endif

#ifdef Q_OS_MACOS
    // It's always 72 on mac
    Q_UNUSED(w);
    return 1;
#else
    return w->logicalDpiX() / 96.0;
#endif
}

inline QWindow *windowForWidget(const QWidget *w)
{
    return w ? w->window()->windowHandle() : nullptr;
}


/// @brief sets the geometry on the QWindow containing the specified item
inline void setTopLevelGeometry(QRect geometry, const QWidget *widget)
{
    if (!widget)
        return;

    if (QWidget *topLevel = widget->window())
        topLevel->setGeometry(geometry);
}

}

#endif
