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
#ifndef MU_DOCK_DOCKBASE_H
#define MU_DOCK_DOCKBASE_H

#include <QQuickItem>

#include <optional>

#include "../docktypes.h"
#include "uicomponents/view/qmllistproperty.h"

namespace KDDockWidgets {
class DockWidgetQuick;
}

namespace mu::dock {
struct DropDestination;
class DockBase : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumSizeChanged)
    Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumSizeChanged)
    Q_PROPERTY(int maximumWidth READ maximumWidth WRITE setMaximumWidth NOTIFY maximumSizeChanged)
    Q_PROPERTY(int maximumHeight READ maximumHeight WRITE setMaximumHeight NOTIFY maximumSizeChanged)
    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentSizeChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentSizeChanged)

    Q_PROPERTY(int location READ locationProperty WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(QVariantList dropDestinations READ dropDestinationsProperty WRITE setDropDestinations NOTIFY dropDestinationsChanged)

    Q_PROPERTY(bool persistent READ persistent WRITE setPersistent NOTIFY persistentChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(bool separatorsVisible READ separatorsVisible WRITE setSeparatorsVisible NOTIFY separatorsVisibleChanged)

    Q_PROPERTY(bool floating READ floating NOTIFY floatingChanged)

public:
    explicit DockBase(QQuickItem* parent = nullptr);

    QString title() const;

    int minimumWidth() const;
    int minimumHeight() const;
    int maximumWidth() const;
    int maximumHeight() const;
    int contentWidth() const;
    int contentHeight() const;
    QSize preferredSize() const;

    int locationProperty() const;
    Location location() const;

    QVariantList dropDestinationsProperty() const;
    QList<DropDestination> dropDestinations() const;

    bool persistent() const;
    bool resizable() const;
    bool separatorsVisible() const;

    bool floating() const;

    virtual void init();

    bool isOpen() const;
    void open();
    void close();

    void showHighlighting(const QRect& highlightingRect);
    void hideHighlighting();

    QRect frameGeometry() const;

    void resetToDefault();

    Q_INVOKABLE void resize(int width, int height);

public slots:
    void setTitle(const QString& title);

    void setMinimumWidth(int width);
    void setMinimumHeight(int height);
    void setMaximumWidth(int width);
    void setMaximumHeight(int height);
    void setContentWidth(int width);
    void setContentHeight(int height);

    void setLocation(int location);
    void setDropDestinations(const QVariantList& destinations);

    void setPersistent(bool persistent);
    void setResizable(bool resizable);
    void setSeparatorsVisible(bool visible);

    void setFloating(bool floating);

signals:
    void titleChanged();

    void minimumSizeChanged();
    void maximumSizeChanged();
    void contentSizeChanged();

    void locationChanged();
    void dropDestinationsChanged();

    void persistentChanged();
    void resizableChanged();
    void separatorsVisibleChanged();

    void floatingChanged();

protected:
    friend class DockWindow;
    friend class DropController;

    virtual DockType type() const = 0;

    void componentComplete() override;

    KDDockWidgets::DockWidgetQuick* dockWidget() const;

protected slots:
    void applySizeConstraints();

private slots:
    void updateFloatingStatus();

private:
    void listenFloatingChanges();

    void writeProperties();

    QString m_title;

    int m_minimumWidth = 0;
    int m_minimumHeight = 0;
    int m_maximumWidth = 0;
    int m_maximumHeight = 0;
    int m_contentWidth = 0;
    int m_contentHeight = 0;

    int m_location = Location::Undefined;
    QVariantList m_dropDestinations;

    bool m_persistent = false;
    bool m_resizable = false;
    bool m_separatorsVisible = false;
    QRect m_highlightingRect;

    bool m_defaultVisibility = false;

    std::optional<bool> m_floating = std::nullopt;

    KDDockWidgets::DockWidgetQuick* m_dockWidget = nullptr;
};

struct DropDestination
{
    DockBase* dock = nullptr;
    Location dropLocation = Location::Undefined;
    int dropDistance = 0;

    bool operator==(const DropDestination& dest) const;

    bool isValid() const;
    void clear();
};
}

#endif // MU_DOCK_DOCKBASE_H
