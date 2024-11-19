/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DropArea.h"
#include "Layout_p.h"
#include "Config.h"
#include "core/ViewFactory.h"
#include "DockRegistry.h"
#include "Platform.h"
#include "core/Draggable_p.h"
#include "core/Logging_p.h"
#include "core/Utils_p.h"
#include "core/layouting/Item_p.h"
#include "core/layouting/LayoutingGuest_p.h"
#include "core/layouting/LayoutingSeparator_p.h"
#include "core/WindowBeingDragged_p.h"
#include "core/DelayedCall_p.h"
#include "core/Group.h"
#include "core/FloatingWindow.h"
#include "core/DockWidget_p.h"
#include "core/MainWindow.h"
#include "core/DropIndicatorOverlay.h"
#include "core/indicators/ClassicDropIndicatorOverlay.h"
#include "core/indicators/NullDropIndicatorOverlay.h"
#include "core/indicators/SegmentedDropIndicatorOverlay.h"

#include "Window_p.h"
#include "kdbindings/signal.h"

#include <algorithm>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

namespace KDDockWidgets {
static Core::DropIndicatorOverlay *
createDropIndicatorOverlay(Core::DropArea *dropArea)
{
    switch (ViewFactory::s_dropIndicatorType) {
    case DropIndicatorType::Classic:
        return new Core::ClassicDropIndicatorOverlay(dropArea);
    case DropIndicatorType::Segmented:
        return new Core::SegmentedDropIndicatorOverlay(dropArea);
    case DropIndicatorType::None:
        return new Core::NullDropIndicatorOverlay(dropArea);
    }

    return new Core::ClassicDropIndicatorOverlay(dropArea);
}

namespace Core {
class DropArea::Private
{
public:
    explicit Private(DropArea *q, MainWindowOptions options, bool isMDIWrapper)
        : m_isMDIWrapper(isMDIWrapper)
        , m_dropIndicatorOverlay(createDropIndicatorOverlay(q))
        , m_centralGroup(createCentralGroup(options))
    {
    }

    bool m_inDestructor = false;
    const bool m_isMDIWrapper;
    QString m_affinityName;
    ObjectGuard<DropIndicatorOverlay> m_dropIndicatorOverlay;
    Core::Group *const m_centralGroup = nullptr;
    Core::ItemBoxContainer *m_rootItem = nullptr;
    KDBindings::ScopedConnection m_visibleWidgetCountConnection;
};
}

}

DropArea::DropArea(View *parent, MainWindowOptions options, bool isMDIWrapper)
    : Layout(ViewType::DropArea, Config::self().viewFactory()->createDropArea(this, parent))
    , d(new Private(this, options, isMDIWrapper))
{
    setRootItem(new Core::ItemBoxContainer(asLayoutingHost()));

    if (parent)
        setLayoutSize(parent->size());

    // Initialize min size
    updateSizeConstraints();

    KDDW_TRACE("DropArea CTOR");

    if (d->m_isMDIWrapper) {
        d->m_visibleWidgetCountConnection = Layout::d_ptr()->visibleWidgetCountChanged.connect([this] {
            auto dw = mdiDockWidgetWrapper();
            if (!dw) {
                KDDW_ERROR("Unexpected null wrapper dock widget");
                return;
            }

            if (visibleCount() > 0) {
                // The title of our MDI group will need to change to the app name if we have more
                // than 1 dock widget nested
                dw->d->titleChanged.emit(dw->title());
            } else {
                // Our wrapeper isn't needed anymore
                dw->destroyLater();
            }
        });
    }

    if (d->m_centralGroup)
        addWidget(d->m_centralGroup->view(), KDDockWidgets::Location_OnTop, {});
}

DropArea::~DropArea()
{
    d->m_inDestructor = true;
    delete d->m_dropIndicatorOverlay;
    delete d;
    KDDW_TRACE("~DropArea");
}

Core::Group::List DropArea::groups() const
{
    const Core::Item::List children = d->m_rootItem->items_recursive();
    Core::Group::List groups;

    for (const Core::Item *child : children) {
        if (auto guest = child->guest()) {
            if (!guest->freed()) {
                if (auto group = Group::fromItem(child)) {
                    groups.push_back(group);
                }
            }
        }
    }

    return groups;
}

Core::Group *DropArea::groupContainingPos(Point globalPos) const
{
    const Core::Item::List &items = this->items();
    for (Core::Item *item : items) {
        auto group = Group::fromItem(item);
        if (!group || !group->isVisible()) {
            continue;
        }

        if (group->containsMouse(globalPos))
            return group;
    }
    return nullptr;
}

void DropArea::updateFloatingActions()
{
    const Core::Group::List groups = this->groups();
    for (Core::Group *group : groups)
        group->updateFloatingActions();
}

Core::Item *DropArea::centralFrame() const
{
    const auto items = this->items();
    for (Core::Item *item : items) {
        if (auto group = Group::fromItem(item)) {
            if (group->isCentralGroup())
                return item;
        }
    }
    return nullptr;
}

DropIndicatorOverlay *DropArea::dropIndicatorOverlay() const
{
    return d->m_dropIndicatorOverlay;
}

void DropArea::addDockWidget(Core::DockWidget *dw, Location location,
                             Core::DockWidget *relativeTo, const InitialOption &option)
{
    if (!dw || dw == relativeTo || location == Location_None) {
        KDDW_ERROR("Invalid parameters {}, {} {}", ( void * )dw, ( void * )relativeTo, location);
        return;
    }

    Core::Group *relativeToGroup = relativeTo ? relativeTo->d->group() : nullptr;
    Core::Item *relativeToItem = relativeToGroup ? relativeToGroup->layoutItem() : nullptr;
    _addDockWidget(dw, location, relativeToItem, option);
}

void DropArea::_addDockWidget(Core::DockWidget *dw, Location location,
                              Core::Item *relativeToItem, const InitialOption &option)
{
    if (!dw || location == Location_None) {
        KDDW_ERROR("Invalid parameters {}, {}", ( void * )dw, location);
        return;
    }

    if ((option.visibility == InitialVisibilityOption::StartHidden) && dw->d->group() != nullptr) {
        // StartHidden is just to be used at startup, not for moving stuff around
        KDDW_ERROR("Dock widget was already opened, can't be used with InitialVisibilityOption::StartHidden");
        return;
    }

    if (!validateAffinity(dw))
        return;

    Core::DockWidget::Private::UpdateActions actionsUpdater(dw);

    Core::Group *group = nullptr;

    dw->d->saveLastFloatingGeometry();

    const bool hadSingleFloatingGroup = hasSingleFloatingGroup();

    // Check if the dock widget already exists in the layout
    if (containsDockWidget(dw)) {
        Core::Group *oldGroup = dw->d->group();
        if (oldGroup->hasSingleDockWidget()) {
            assert(oldGroup->containsDockWidget(dw));
            // The group only has this dock widget, and the group is already in the layout. So move
            // the group instead
            group = oldGroup;
        } else {
            group = new Core::Group();
            group->addTab(dw);
        }
    } else {
        group = new Core::Group();
        group->addTab(dw);
    }

    if (option.startsHidden()) {
        addWidget(dw->view(), location, relativeToItem, option);
    } else {
        addWidget(group->view(), location, relativeToItem, option);
    }

    if (hadSingleFloatingGroup && !hasSingleFloatingGroup()) {
        // The dock widgets that already existed in our layout need to have their floatAction()
        // updated otherwise it's still checked. Only the dropped dock widget got updated
        updateFloatingActions();
    }
}

bool DropArea::containsDockWidget(Core::DockWidget *dw) const
{
    return dw->d->group() && Layout::containsGroup(dw->d->group());
}

bool DropArea::hasSingleFloatingGroup() const
{
    const Core::Group::List groups = this->groups();
    return groups.size() == 1 && groups.first()->isFloating();
}

bool DropArea::hasSingleGroup() const
{
    return visibleCount() == 1;
}

Vector<QString> DropArea::affinities() const
{
    if (auto mw = mainWindow()) {
        return mw->affinities();
    } else if (auto fw = floatingWindow()) {
        return fw->affinities();
    }

    return {};
}

void DropArea::layoutParentContainerEqually(Core::DockWidget *dw)
{
    Core::Item *item = itemForGroup(dw->d->group());
    if (!item) {
        KDDW_ERROR("Item not found for dw={}, group={}", ( void * )dw, ( void * )dw->d->group());
        return;
    }

    layoutEqually(item->parentBoxContainer());
}

DropLocation DropArea::hover(WindowBeingDragged *draggedWindow, Point globalPos)
{
    if (Config::self().dropIndicatorsInhibited() || !validateAffinity(draggedWindow))
        return DropLocation_None;

    if (!d->m_dropIndicatorOverlay) {
        KDDW_ERROR("The frontend is missing a drop indicator overlay");
        return DropLocation_None;
    }

    Core::Group *group = groupContainingPos(
        globalPos); // Group is nullptr if MainWindowOption_HasCentralFrame isn't set
    d->m_dropIndicatorOverlay->setWindowBeingDragged(true);
    d->m_dropIndicatorOverlay->setHoveredGroup(group);
    draggedWindow->updateTransparency(true);

    return d->m_dropIndicatorOverlay->hover(globalPos);
}

static bool isOutterLocation(DropLocation location)
{
    switch (location) {
    case DropLocation_OutterLeft:
    case DropLocation_OutterTop:
    case DropLocation_OutterRight:
    case DropLocation_OutterBottom:
        return true;
    default:
        return false;
    }
}

bool DropArea::drop(WindowBeingDragged *droppedWindow, Point globalPos)
{
    // fv might be null, if on wayland
    Core::View *fv = droppedWindow->floatingWindowView();

    if (fv && fv->equals(window())) {
        KDDW_ERROR("Refusing to drop onto itself"); // Doesn't happen
        return false;
    }

    if (d->m_dropIndicatorOverlay->currentDropLocation() == DropLocation_None) {
        KDDW_DEBUG("DropArea::drop: bailing out, drop location = none");
        return false;
    }

    KDDW_DEBUG("DropArea::drop: {}", ( void * )droppedWindow);

    hover(droppedWindow, globalPos);
    auto droploc = d->m_dropIndicatorOverlay->currentDropLocation();
    Core::Group *acceptingGroup = d->m_dropIndicatorOverlay->hoveredGroup();
    if (!(acceptingGroup || isOutterLocation(droploc))) {
        KDDW_ERROR("DropArea::drop: asserted with group={}, location={}", ( void * )acceptingGroup, droploc);
        return false;
    }

    return drop(droppedWindow, acceptingGroup, droploc);
}

bool DropArea::drop(WindowBeingDragged *draggedWindow, Core::Group *acceptingGroup,
                    DropLocation droploc)
{
    Core::FloatingWindow *droppedWindow =
        draggedWindow ? draggedWindow->floatingWindow() : nullptr;

    if (isWayland() && !droppedWindow) {
        // This is the Wayland special case.
        // With other platforms, when detaching a tab or dock widget we create the FloatingWindow
        // immediately. With Wayland we delay the floating window until we drop it. Ofc, we could
        // just dock the dockwidget without the temporary FloatingWindow, but this way we reuse 99%
        // of the rest of the code, without adding more wayland special cases
        droppedWindow =
            draggedWindow ? draggedWindow->draggable()->makeWindow()->floatingWindow() : nullptr;
        if (!droppedWindow) {
            // Doesn't happen
            KDDW_ERROR("Wayland: Expected window {}", ( void * )draggedWindow);
            return false;
        }
    }

    bool result = true;
    const bool needToFocusNewlyDroppedWidgets =
        Config::self().flags() & Config::Flag_TitleBarIsFocusable;
    const Core::DockWidget::List droppedDockWidgets = needToFocusNewlyDroppedWidgets
        ? droppedWindow->layout()->dockWidgets()
        : Core::DockWidget::List(); // just so save some memory allocations for the case
                                    // where this
    // variable isn't used

    switch (droploc) {
    case DropLocation_Left:
    case DropLocation_Top:
    case DropLocation_Bottom:
    case DropLocation_Right:
        result = drop(droppedWindow->view(),
                      DropIndicatorOverlay::multisplitterLocationFor(droploc), acceptingGroup);
        break;
    case DropLocation_OutterLeft:
    case DropLocation_OutterTop:
    case DropLocation_OutterRight:
    case DropLocation_OutterBottom:
        result = drop(droppedWindow->view(),
                      DropIndicatorOverlay::multisplitterLocationFor(droploc), nullptr);
        break;
    case DropLocation_Center:
        KDDW_DEBUG("Tabbing window={} into group={}", ( void * )droppedWindow, ( void * )acceptingGroup);

        if (!validateAffinity(droppedWindow, acceptingGroup))
            return false;
        acceptingGroup->addTab(droppedWindow);
        break;

    default:
        KDDW_ERROR("DropArea::drop: Unexpected drop location = {}", d->m_dropIndicatorOverlay->currentDropLocation());
        result = false;
        break;
    }

    if (result) {
        // Window receiving the drop gets raised
        // Window receiving the drop gets raised.
        // Exception: Under EGLFS we don't raise the fullscreen main window, as then all floating
        // windows would go behind. It's also unneeded to raise, as it's fullscreen.

        const bool isEGLFSRootWindow =
            isEGLFS() && (view()->window()->isFullScreen() || window()->isMaximized());
        if (!isEGLFSRootWindow)
            view()->raiseAndActivate();

        if (needToFocusNewlyDroppedWidgets) {
            // Let's also focus the newly dropped dock widget
            if (!droppedDockWidgets.isEmpty()) {
                // If more than 1 was dropped, we only focus the first one
                Core::Group *group = droppedDockWidgets.first()->d->group();
                group->FocusScope::focus(Qt::MouseFocusReason);
            } else {
                // Doesn't happen.
                KDDW_ERROR("Nothing was dropped?");
            }
        }
    }

    return result;
}

bool DropArea::drop(View *droppedWindow, KDDockWidgets::Location location,
                    Core::Group *relativeTo)
{
    KDDW_DEBUG("DropArea::drop");

    if (auto dock = droppedWindow->asDockWidgetController()) {
        if (!validateAffinity(dock))
            return false;

        auto group = new Core::Group();
        group->addTab(dock);
        Item *relativeToItem = relativeTo ? relativeTo->layoutItem() : nullptr;
        addWidget(group->view(), location, relativeToItem, DefaultSizeMode::FairButFloor);
    } else if (auto floatingWindow = droppedWindow->asFloatingWindowController()) {
        if (!validateAffinity(floatingWindow))
            return false;

        addMultiSplitter(floatingWindow->dropArea(), location, relativeTo,
                         DefaultSizeMode::FairButFloor);

        floatingWindow->scheduleDeleteLater();
        return true;
    } else {
        KDDW_ERROR("Unknown dropped widget {}", ( void * )droppedWindow);
        return false;
    }

    return true;
}

void DropArea::removeHover()
{
    d->m_dropIndicatorOverlay->removeHover();
}

template<typename T>
bool DropArea::validateAffinity(T *window, Core::Group *acceptingGroup) const
{
    if (!DockRegistry::self()->affinitiesMatch(window->affinities(), affinities())) {
        return false;
    }

    if (acceptingGroup) {
        // We're dropping into another group (as tabbed), so also check the affinity of the group
        // not only of the main window, which might be more forgiving
        if (!DockRegistry::self()->affinitiesMatch(window->affinities(),
                                                   acceptingGroup->affinities())) {
            return false;
        }
    }

    return true;
}

bool DropArea::isMDIWrapper() const
{
    return d->m_isMDIWrapper;
}

Core::DockWidget *DropArea::mdiDockWidgetWrapper() const
{
    if (d->m_isMDIWrapper) {
        return view()->parentView()->asDockWidgetController();
    }

    return nullptr;
}

Core::Group *DropArea::createCentralGroup(MainWindowOptions options)
{
    Core::Group *group = nullptr;

    if (options & MainWindowOption_HasCentralFrame) {
        FrameOptions groupOptions = FrameOption_IsCentralFrame;
        const bool hasPersistentCentralWidget =
            (options & MainWindowOption_HasCentralWidget) == MainWindowOption_HasCentralWidget;
        if (hasPersistentCentralWidget) {
            groupOptions |= FrameOption_NonDockable;
        } else {
            // With a persistent central widget we don't allow detaching it
            groupOptions |= FrameOption_AlwaysShowsTabs;
        }

        group = new Core::Group(nullptr, groupOptions);
        group->setObjectName(QStringLiteral("central group"));
    }

    return group;
}

bool DropArea::validateInputs(View *widget, Location location,
                              const Core::Item *relativeToItem, const InitialOption &option) const
{
    if (!widget) {
        KDDW_ERROR("Widget is null");
        return false;
    }

    const bool isDockWidget = widget->is(ViewType::DockWidget);
    const bool isStartHidden = option.startsHidden();

    const bool isLayout = widget->is(ViewType::DropArea) || widget->is(ViewType::MDILayout);
    if (!widget->is(ViewType::Group) && !isLayout && !isDockWidget) {
        KDDW_ERROR("Unknown widget type {}", ( void * )widget);
        return false;
    }

    if (isDockWidget != isStartHidden) {
        KDDW_ERROR("Wrong parameters isDockWidget={}, isStartHidden={}", isDockWidget, isStartHidden);
        return false;
    }

    if (relativeToItem) {
        auto relativeToGroup = Group::fromItem(relativeToItem);
        if (relativeToGroup && relativeToGroup->view()->equals(widget)) {
            KDDW_ERROR("widget can't be relative to itself");
            return false;
        }
    }

    Core::Item *item = itemForGroup(widget->asGroupController());

    if (containsItem(item)) {
        KDDW_ERROR("DropArea::addWidget: Already contains w={}", ( void * )widget);
        return false;
    }

    if (location == Location_None) {
        KDDW_ERROR("DropArea::addWidget: not adding to location None");
        return false;
    }

    const bool relativeToThis = relativeToItem == nullptr;
    if (!relativeToThis && !containsItem(relativeToItem)) {
        KDDW_ERROR("DropArea::addWidget: Doesn't contain relativeTo: relativeToItem{}, options={}", "; relativeToItem=", ( void * )relativeToItem, option);
        return false;
    }

    return true;
}

void DropArea::addWidget(View *w, Location location, Core::Item *relativeToItem,
                         const InitialOption &option)
{

    auto group = w->asGroupController();
    if (itemForGroup(group) != nullptr) {
        // Item already exists, remove it.
        // Changing the group parent will make the item clean itself up. It turns into a placeholder
        // and is removed by unrefOldPlaceholders
        group->setParentView(nullptr); // so ~Item doesn't delete it
        group->setLayoutItem(nullptr); // so Item is destroyed, as there's no refs to it
    }

    // Make some sanity checks:
    if (!validateInputs(w, location, relativeToItem, option))
        return;

    if (!relativeToItem)
        relativeToItem = d->m_rootItem;

    Core::Item *newItem = nullptr;

    Core::Group::List groups = groupsFrom(w);
    unrefOldPlaceholders(groups);
    auto dw = w->asDockWidgetController();

    if (group) {
        newItem = new Core::Item(asLayoutingHost());
        newItem->setGuest(group->asLayoutingGuest());
    } else if (dw) {
        newItem = new Core::Item(asLayoutingHost());
        group = new Core::Group();
        newItem->setGuest(group->asLayoutingGuest());
        group->addTab(dw, option);
    } else if (auto ms = w->asDropAreaController()) {
        newItem = ms->d->m_rootItem;
        newItem->setHost(asLayoutingHost());

        if (auto fw = ms->floatingWindow()) {
            newItem->setSize_recursive(fw->size());
        }

        delete ms;
    } else {
        // This doesn't happen but let's make coverity happy.
        // Tests will fail if this is ever printed.
        KDDW_ERROR("Unknown widget added", ( void * )w);
        return;
    }

    assert(!newItem->geometry().isEmpty());
    Core::ItemBoxContainer::insertItemRelativeTo(newItem, relativeToItem, location, option);

    if (dw && option.startsHidden())
        delete group;
}

void DropArea::addMultiSplitter(Core::DropArea *sourceMultiSplitter, Location location,
                                Core::Group *relativeToGroup, const InitialOption &option)
{
    KDDW_DEBUG("DropArea::addMultiSplitter: {} {} {}", ( void * )sourceMultiSplitter, ( int )location, ( void * )relativeToGroup);
    Item *relativeToItem = relativeToGroup ? relativeToGroup->layoutItem() : nullptr;

    addWidget(sourceMultiSplitter->view(), location, relativeToItem, option);

    // Some widgets changed to/from floating
    updateFloatingActions();
}

Vector<Core::LayoutingSeparator *> DropArea::separators() const
{
    return d->m_rootItem->separators_recursive();
}

int DropArea::availableLengthForOrientation(Qt::Orientation orientation) const
{
    if (orientation == Qt::Vertical)
        return availableSize().height();
    else
        return availableSize().width();
}

Size DropArea::availableSize() const
{
    return d->m_rootItem->availableSize();
}

void DropArea::layoutEqually()
{
    if (!checkSanity())
        return;

    layoutEqually(d->m_rootItem);
}

void DropArea::layoutEqually(Core::ItemBoxContainer *container)
{
    if (container) {
        container->layoutEqually_recursive();
    } else {
        KDDW_ERROR("null container");
    }
}

void DropArea::setRootItem(Core::ItemBoxContainer *root)
{
    Layout::setRootItem(root);
    d->m_rootItem = root;
}

Core::ItemBoxContainer *DropArea::rootItem() const
{
    return d->m_rootItem;
}

Rect DropArea::rectForDrop(const WindowBeingDragged *wbd, Location location,
                           const Core::Item *relativeTo) const
{
    Core::Item item(nullptr);
    if (!wbd)
        return {};

    item.setSize(wbd->size().boundedTo(wbd->maxSize()));
    item.setMinSize(wbd->minSize());
    item.setMaxSizeHint(wbd->maxSize());

    Core::ItemBoxContainer *container =
        relativeTo ? relativeTo->parentBoxContainer() : d->m_rootItem;

    return container->suggestedDropRect(&item, relativeTo, location);
}

bool DropArea::deserialize(const LayoutSaver::MultiSplitter &l)
{
    setRootItem(new Core::ItemBoxContainer(asLayoutingHost()));
    return Layout::deserialize(l);
}

int DropArea::numSideBySide_recursive(Qt::Orientation o) const
{
    return d->m_rootItem->numSideBySide_recursive(o);
}

DropLocation DropArea::currentDropLocation() const
{
    return d->m_dropIndicatorOverlay ? d->m_dropIndicatorOverlay->currentDropLocation() : DropLocation_None;
}

Core::Group *DropArea::centralGroup() const
{
    return d->m_centralGroup;
}
