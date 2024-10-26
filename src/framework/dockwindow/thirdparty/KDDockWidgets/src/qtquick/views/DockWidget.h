/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DOCKWIDGET_QUICK_H
#define KD_DOCKWIDGET_QUICK_H

#include "kddockwidgets/core/views/DockWidgetViewInterface.h"
#include "kddockwidgets/qtquick/Action.h"
#include "View.h"

QT_BEGIN_NAMESPACE
class QQmlEngine;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {
class Group;
class TitleBar;
}

namespace QtQuick {
/**
 * @brief Represents a dock widget.
 *
 * Most of the interface lives in Core::DockWidget, to facilitate sharing with QtQuick.
 */
class DOCKS_EXPORT DockWidget : public QtQuick::View,
                                public Core::DockWidgetViewInterface
{
    Q_OBJECT
    Q_PROPERTY(QObject *actualTitleBar READ actualTitleBarView NOTIFY actualTitleBarChanged)
    Q_PROPERTY(bool isFocused READ isFocused NOTIFY isFocusedChanged)
    Q_PROPERTY(bool isFloating READ isFloating WRITE setFloating NOTIFY isFloatingChanged)
    Q_PROPERTY(QString uniqueName READ uniqueName CONSTANT)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QObject *guestItem READ guestItem NOTIFY guestItemChanged)
    Q_PROPERTY(KDDockWidgets::DockWidgetOptions options READ options WRITE setOptions NOTIFY
                   optionsChanged)
public:
    /**
     * @brief constructs a new DockWidget
     * @param uniqueName the name of the dockwidget, should be unique. Use title for user visible
     * text.
     * @param options optional options controlling behaviour
     * @param layoutSaverOptions options regarding LayoutSaver behaviour
     * @param engine the QML engine this dock widget will be created on. If not specified then
     * Config::self().qmlEngine() will be used
     *
     * There's no parent argument. The DockWidget is either parented to FloatingWindow or MainWindow
     * when visible, or stays without a parent when hidden.
     */
    explicit DockWidget(const QString &uniqueName, DockWidgetOptions = {},
                        LayoutSaverOptions = {}, Qt::WindowFlags = Qt::Tool,
                        QQmlEngine *engine = nullptr);

    ///@brief destructor
    ~DockWidget() override;

    /// Sets the DockWidget's guest item
    /// @param qmlFilename The path to a QML file to load. This path is passed to QQmlComponent.
    /// @param context An optional QQmlContext. This is passed to QQmlComponent::create().
    void setGuestItem(const QString &qmlFilename, QQmlContext *context = nullptr);

    /// @reimp
    Q_INVOKABLE void setGuestItem(QQuickItem *);

    /// @brief Returns the guest item that we're hosting
    QQuickItem *guestItem() const;

    /// @reimp
    QSize minSize() const override;

    /// @reimp
    QSize maxSizeHint() const override;

    /// @brief Returns the title bar view
    /// Qt6 requires us to include TitleBar_p.h, so instead the Q_PROPERTY uses
    /// QObject so we don't include private headers in public headers
    QObject *actualTitleBarView() const;

    /// @brief Returns the visual item which represents Group in the screen
    /// Equivalent to Group::visualItem().
    QQuickItem *groupVisualItem() const;

    /// @brief Called by QtQuick when min-size changes
    Q_INVOKABLE void onGeometryUpdated();

    Q_INVOKABLE KDDockWidgets::QtQuick::Action *toggleAction() const;
    Q_INVOKABLE KDDockWidgets::QtQuick::Action *floatAction() const;

    std::shared_ptr<Core::View> focusCandidate() const override;

    // Override QQuickItem::show() as there's more to do.
    void show() override
    {
        Core::DockWidgetViewInterface::open();
    }

    // Override QQuickItem::raise() as there's more to do, like setting it as current tab
    // if it's tabbed
    void raise() override
    {
        Core::DockWidgetViewInterface::raise();
    }

#ifdef Q_MOC_RUN
    // DockWidgetViewInterface is not a QObject, so trick moc
    Q_INVOKABLE void setAsCurrentTab();
    Q_INVOKABLE void forceClose();
    Q_INVOKABLE bool isOpen() const;
    Q_INVOKABLE void open();
    Q_INVOKABLE void show();
    Q_INVOKABLE void raise();
    Q_INVOKABLE void moveToSideBar();
#endif
Q_SIGNALS:
    /// @brief The geometry of the group container this dock widget is in changed
    /// For example, when dragging a dockwidget
    void groupGeometryChanged(QRect);
    void actualTitleBarChanged();
    void isFocusedChanged();
    void isFloatingChanged();
    void titleChanged();
    void guestItemChanged();
    void optionsChanged();

protected:
    bool event(QEvent *e) override;

private:
    class Private;
    Private *const d;
};

}
}

#endif
