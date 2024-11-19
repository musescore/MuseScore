/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockWidgetInstantiator.h"
#include "kddockwidgets/core/DockRegistry.h"
#include "core/DockWidget_p.h"
#include "ViewFactory.h"
#include "Config.h"
#include "Platform.h"

#include <kdbindings/signal.h>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

class DockWidgetInstantiator::Private
{
public:
    std::optional<bool> m_isFloating;
    QString m_uniqueName;
    QString m_sourceFilename;
    QString m_title;
    Core::DockWidget *m_dockWidget = nullptr;
    QVector<QString> m_affinities;

    KDBindings::ScopedConnection titleConnection;
    KDBindings::ScopedConnection closedConnection;
    KDBindings::ScopedConnection iconConnection;
    KDBindings::ScopedConnection actualTitleBarConnection;
    KDBindings::ScopedConnection optionsConnection;
    KDBindings::ScopedConnection windowActiveAboutToChangeConnection;
    KDBindings::ScopedConnection isOverlayedConnection;
    KDBindings::ScopedConnection isFocusedConnection;
    KDBindings::ScopedConnection isFloatingConnection;
    KDBindings::ScopedConnection isOpenConnection;
    KDBindings::ScopedConnection guestViewChangedConnection;
    KDBindings::ScopedConnection removedFromSideBarConnection;
};

DockWidgetInstantiator::DockWidgetInstantiator()
    : d(new Private())
{
}

DockWidgetInstantiator::~DockWidgetInstantiator()
{
    delete d;
}

QString DockWidgetInstantiator::uniqueName() const
{
    return d->m_uniqueName;
}

void DockWidgetInstantiator::setUniqueName(const QString &name)
{
    d->m_uniqueName = name;
    Q_EMIT uniqueNameChanged();
}

QString DockWidgetInstantiator::source() const
{
    return d->m_sourceFilename;
}

void DockWidgetInstantiator::setSource(const QString &source)
{
    d->m_sourceFilename = source;
    Q_EMIT sourceChanged();
}

QtQuick::DockWidget *DockWidgetInstantiator::dockWidget() const
{
    if (d->m_dockWidget) {
        return static_cast<QtQuick::DockWidget *>(d->m_dockWidget->view());
    }

    return nullptr;
}

KDDockWidgets::Core::DockWidget *DockWidgetInstantiator::controller() const
{
    return d->m_dockWidget;
}

QObject *DockWidgetInstantiator::actualTitleBar() const
{
    if (auto dockView = dockWidget()) {
        return dockView->actualTitleBarView();
    }

    return nullptr;
}

QString DockWidgetInstantiator::title() const
{
    return d->m_dockWidget ? d->m_dockWidget->title() : QString();
}

void DockWidgetInstantiator::setTitle(const QString &title)
{
    if (d->m_dockWidget)
        d->m_dockWidget->setTitle(title);
    d->m_title = title;
}

bool DockWidgetInstantiator::isFocused() const
{
    return d->m_dockWidget && d->m_dockWidget->isFocused();
}

bool DockWidgetInstantiator::isFloating() const
{
    return d->m_dockWidget && d->m_dockWidget->isFloating();
}

bool DockWidgetInstantiator::isOpen() const
{
    return d->m_dockWidget && d->m_dockWidget->isOpen();
}

void DockWidgetInstantiator::setFloating(bool is)
{
    if (d->m_dockWidget)
        d->m_dockWidget->setFloating(is);
    d->m_isFloating = is;
}

void DockWidgetInstantiator::addDockWidgetAsTab(QQuickItem *other, InitialVisibilityOption option)
{
    if (!other || !d->m_dockWidget)
        return;

    Core::DockWidget *otherDockWidget = Platform::dockWidgetForItem(other);
    d->m_dockWidget->addDockWidgetAsTab(otherDockWidget, option);
}

void DockWidgetInstantiator::addDockWidgetToContainingWindow(QQuickItem *other, Location location,
                                                             QQuickItem *relativeTo,
                                                             QSize initialSize,
                                                             InitialVisibilityOption option)
{
    if (!other || !d->m_dockWidget)
        return;

    Core::DockWidget *otherDockWidget = Platform::dockWidgetForItem(other);
    Core::DockWidget *relativeToDockWidget = Platform::dockWidgetForItem(relativeTo);

    d->m_dockWidget->addDockWidgetToContainingWindow(otherDockWidget, location, relativeToDockWidget,
                                                     InitialOption(option, initialSize));
}

void DockWidgetInstantiator::setAsCurrentTab()
{
    if (d->m_dockWidget)
        d->m_dockWidget->setAsCurrentTab();
}

void DockWidgetInstantiator::forceClose()
{
    if (d->m_dockWidget)
        d->m_dockWidget->forceClose();
}

Q_INVOKABLE bool DockWidgetInstantiator::close()
{
    if (d->m_dockWidget)
        return d->m_dockWidget->close();

    return false;
}

void DockWidgetInstantiator::open()
{
    if (d->m_dockWidget)
        d->m_dockWidget->open();
}

void DockWidgetInstantiator::show()
{
    // "show" is deprecated vocabulary
    open();
}

void DockWidgetInstantiator::raise()
{
    if (d->m_dockWidget)
        d->m_dockWidget->raise();
}

void DockWidgetInstantiator::moveToSideBar()
{
    if (d->m_dockWidget)
        d->m_dockWidget->moveToSideBar();
}

void DockWidgetInstantiator::deleteDockWidget()
{
    delete d->m_dockWidget;
    delete this;
}

void DockWidgetInstantiator::classBegin()
{
    // Nothing interesting to do here.
}

QVector<QString> DockWidgetInstantiator::affinities() const
{
    return d->m_dockWidget ? d->m_dockWidget->affinities() : QVector<QString>();
}

void DockWidgetInstantiator::setAffinities(const QVector<QString> &affinities)
{
    if (d->m_affinities != affinities) {
        d->m_affinities = affinities;
        Q_EMIT affinitiesChanged();
    }
}

void DockWidgetInstantiator::componentComplete()
{
    if (d->m_uniqueName.isEmpty()) {
        qWarning() << Q_FUNC_INFO
                   << "Each DockWidget need an unique name. Set the uniqueName property.";
        return;
    }

    if (DockRegistry::self()->containsDockWidget(d->m_uniqueName)) {
        // Dock widget already exists. all good.
        return;
    }

    if (d->m_dockWidget) {
        qWarning() << Q_FUNC_INFO << "Unexpected bug.";
        return;
    }
    const auto childItems = this->childItems();
    if (d->m_sourceFilename.isEmpty() && childItems.size() != 1) {
        qWarning() << Q_FUNC_INFO << "Either 'source' property must be set or add exactly one child"
                   << "; source=" << d->m_sourceFilename << "; num children=" << childItems.size();
        return;
    }

    d->m_dockWidget = ViewFactory::self()
                          ->createDockWidget(d->m_uniqueName, qmlEngine(this))
                          ->asDockWidgetController();

    d->titleConnection = d->m_dockWidget->d->titleChanged.connect([this](const QString &title) { Q_EMIT titleChanged(title); });
    d->closedConnection = d->m_dockWidget->d->closed.connect([this] { Q_EMIT closed(); });
    d->iconConnection = d->m_dockWidget->d->iconChanged.connect([this] { Q_EMIT iconChanged(); });
    d->actualTitleBarConnection = d->m_dockWidget->d->actualTitleBarChanged.connect([this] { Q_EMIT actualTitleBarChanged(); });
    d->optionsConnection = d->m_dockWidget->d->optionsChanged.connect([this](KDDockWidgets::DockWidgetOptions opts) { Q_EMIT optionsChanged(opts); });

    d->windowActiveAboutToChangeConnection = d->m_dockWidget->d->windowActiveAboutToChange.connect([this](bool is) { Q_EMIT windowActiveAboutToChange(is); });
    d->isFocusedConnection = d->m_dockWidget->d->isFocusedChanged.connect([this](bool is) { Q_EMIT isFocusedChanged(is); });

    d->isOverlayedConnection = d->m_dockWidget->d->isOverlayedChanged.connect([this](bool is) { Q_EMIT isOverlayedChanged(is); });
    d->isFloatingConnection = d->m_dockWidget->d->isFloatingChanged.connect([this](bool is) { Q_EMIT isFloatingChanged(is); });
    d->isOpenConnection = d->m_dockWidget->d->isOpenChanged.connect([this](bool is) { Q_EMIT isOpenChanged(is); });

    d->guestViewChangedConnection = d->m_dockWidget->d->guestViewChanged.connect([this] { Q_EMIT guestViewChanged(QtQuick::asQQuickItem(d->m_dockWidget->guestView().get())); });
    d->removedFromSideBarConnection = d->m_dockWidget->d->removedFromSideBar.connect([this] { Q_EMIT removedFromSideBar(); });

    auto view = this->dockWidget();
    if (d->m_sourceFilename.isEmpty()) {
        view->setGuestItem(childItems.constFirst());
    } else {
        view->setGuestItem(d->m_sourceFilename);
    }

    if (!d->m_title.isEmpty())
        d->m_dockWidget->setTitle(d->m_title);

    if (d->m_isFloating.has_value())
        d->m_dockWidget->setFloating(d->m_isFloating.value());

    d->m_dockWidget->setAffinities(d->m_affinities);

    Q_EMIT dockWidgetChanged();
}
