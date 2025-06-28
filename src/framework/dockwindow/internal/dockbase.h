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
#ifndef MUSE_DOCK_DOCKBASE_H
#define MUSE_DOCK_DOCKBASE_H

#include <QQuickItem>

#include <optional>

#include "../docktypes.h"
#include "uicomponents/view/qmllistproperty.h"
#include "ui/view/navigationsection.h"

namespace KDDockWidgets {
class DockWidgetQuick;
}

namespace muse::dock {
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

    Q_PROPERTY(bool floatable READ floatable WRITE setFloatable NOTIFY floatableChanged)
    Q_PROPERTY(bool closable READ closable WRITE setClosable NOTIFY closableChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(bool separatorsVisible READ separatorsVisible WRITE setSeparatorsVisible NOTIFY separatorsVisibleChanged)

    Q_PROPERTY(bool isCompact READ isCompact WRITE setIsCompact NOTIFY isCompactChanged FINAL)
    Q_PROPERTY(int compactPriorityOrder READ compactPriorityOrder WRITE setCompactPriorityOrder NOTIFY compactPriorityOrderChanged FINAL)

    Q_PROPERTY(bool floating READ floating NOTIFY floatingChanged)

    Q_PROPERTY(bool inited READ inited NOTIFY initedChanged)

    Q_PROPERTY(
        muse::ui::NavigationSection * navigationSection READ navigationSection WRITE setNavigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(int contentNavigationPanelOrderStart READ contentNavigationPanelOrderStart NOTIFY contentNavigationPanelOrderStartChanged)

public:
    DockBase(DockType type, QQuickItem* parent = nullptr);

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

    QPoint globalPosition() const;

    QVariantList dropDestinationsProperty() const;
    QList<DropDestination> dropDestinations() const;

    bool floatable() const;
    bool closable() const;
    bool resizable() const;
    bool separatorsVisible() const;
    bool defaultVisibility() const;

    bool isCompact() const;
    int compactPriorityOrder() const;
    int nonCompactWidth() const;

    bool floating() const;

    bool inited() const;

    virtual void init();
    virtual void resetToDefault();

    void deinit();

    void showHighlighting(const QRect& highlightingRect);
    void hideHighlighting();

    QRect frameGeometry() const;

    bool isInSameFrame(const DockBase* other) const;
    void setFramePanelOrder(int order);

    Q_INVOKABLE bool isOpen() const;
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void resize(int width, int height);

    ui::NavigationSection* navigationSection() const;
    int contentNavigationPanelOrderStart() const;

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

    void setFloatable(bool floatable);
    void setClosable(bool closable);
    void setResizable(bool resizable);
    void setSeparatorsVisible(bool visible);

    void setIsCompact(bool compact);
    void setCompactPriorityOrder(int order);

    void setFloating(bool floating);

    void setNavigationSection(ui::NavigationSection* section);
    void setContentNavigationPanelOrderStart(int order);

signals:
    void titleChanged();

    void minimumSizeChanged();
    void maximumSizeChanged();
    void contentSizeChanged();

    void locationChanged();
    void dropDestinationsChanged();

    void floatableChanged();
    void closableChanged();
    void resizableChanged();
    void separatorsVisibleChanged();

    void isCompactChanged();
    void compactPriorityOrderChanged();

    void floatingChanged();
    void frameCurrentWidgetChanged();

    void initedChanged();

    void navigationSectionChanged();
    void contentNavigationPanelOrderStartChanged();

protected:
    friend class DockWindow;
    friend class DropController;

    void componentComplete() override;

    DockType type() const;
    KDDockWidgets::DockWidgetQuick* dockWidget() const;

protected slots:
    void applySizeConstraints();

private slots:
    void updateFloatingStatus();
    void onIsInMainWindowChanged();

private:
    void setUpFrameConnections();
    void doSetFloating(bool floating);

    void writeProperties();

    void setInited(bool inited);

    QString m_title;

    int m_minimumWidth = 0;
    int m_minimumHeight = 0;
    int m_maximumWidth = 0;
    int m_maximumHeight = 0;
    int m_contentWidth = 0;
    int m_contentHeight = 0;

    DockProperties m_properties;
    QVariantList m_dropDestinations;

    bool m_defaultVisibility = false;

    bool m_isCompact = false;
    int m_compactPriorityOrder = -1;
    int m_nonCompactWidth = 0;

    bool m_floating = false;

    bool m_inited = false;
    KDDockWidgets::DockWidgetQuick* m_dockWidget = nullptr;

    ui::NavigationSection* m_navigationSection = nullptr;
    int m_contentNavigationPanelOrderStart = 1;
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

#endif // MUSE_DOCK_DOCKBASE_H
