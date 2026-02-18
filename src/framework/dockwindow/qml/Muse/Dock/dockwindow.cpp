/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "dockwindow.h"

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/LayoutSaver.h"
#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"
#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"
#include "thirdparty/KDDockWidgets/src/Config.h"

#include "global/async/async.h"

#include "dockcentralview.h"
#include "dockpageview.h"
#include "dockpanelview.h"
#include "dockstatusbar.h"
#include "docktoolbarview.h"
#include "dockingholderview.h"
#include "dockwindow.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse::dock;
using namespace muse::async;

#define DOCK_CONTEXT_GUARD \
    KDDockWidgets::ConfigContextGuard _ctxGuard( \
        iocContext() ? iocContext()->id : -1)

static const QString CONTEXT_GEOMETRY_KEY = QStringLiteral("__windowGeometry");

namespace muse::dock {
static const QList<Location> POSSIBLE_LOCATIONS {
    Location::Left,
    Location::Right,
    Location::Top,
    Location::Bottom
};

static KDDockWidgets::Location locationToKLocation(Location location)
{
    switch (location) {
    case Location::Left: return KDDockWidgets::Location_OnLeft;
    case Location::Right: return KDDockWidgets::Location_OnRight;
    case Location::Top: return KDDockWidgets::Location_OnTop;
    case Location::Bottom: return KDDockWidgets::Location_OnBottom;
    case Location::Center: break;
    case Location::Undefined: break;
    }

    return KDDockWidgets::Location_None;
}

static void clearRegistry(const DockPageView* page = nullptr)
{
    TRACEFUNC;

    auto registry = KDDockWidgets::DockRegistry::self();

#ifdef MUSE_MULTICONTEXT_WIP
    if (!page) {
        return;
    }

    // In multi-context mode, only clear docks belonging to the current page
    QSet<QString> pageDockNames;
    for (DockBase* dock : page->allDocks()) {
        pageDockNames.insert(dock->uniqueDockName());
    }

    for (KDDockWidgets::Frame* frame : registry->frames()) {
        for (KDDockWidgets::DockWidgetBase* dw : frame->dockWidgets()) {
            if (pageDockNames.contains(dw->uniqueName())) {
                frame->removeWidget(dw);
            }
        }
    }

    for (const QString& name : pageDockNames) {
        KDDockWidgets::DockWidgetBase* dw = registry->dockByName(name);
        if (dw) {
            registry->unregisterDockWidget(dw);
        }
    }
#else
    registry->clear();

    for (KDDockWidgets::DockWidgetBase* dock : registry->dockwidgets()) {
        registry->unregisterDockWidget(dock);
    }

    for (KDDockWidgets::Frame* frame : registry->frames()) {
        for (KDDockWidgets::DockWidgetBase* dock : frame->dockWidgets()) {
            frame->removeWidget(dock);
        }

        registry->unregisterFrame(frame);
    }
#endif
}
}

class DockWindow::UniqueConnectionHolder : public QObject
{
    Q_OBJECT
public:
    UniqueConnectionHolder(DockPageView* page, DockWindow* parent)
        : QObject(parent), m_page(page) {}

    void alignTopLevelToolBars()
    {
        static_cast<DockWindow*>(parent())->alignTopLevelToolBars(m_page);
    }

private:
    DockPageView* m_page = nullptr;
};

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent), muse::Contextable(muse::iocCtxForQmlObject(this)),
    m_toolBars(this),
    m_pages(this)
{
}

DockWindow::~DockWindow()
{
    dockWindowProvider()->deinit();

    // Without this, the connections would be deleted by the QObject destructor,
    // because they are child objects of this. But since they use DockWindow-
    // specific code (rather than QObject-specific), we need to delete them
    // before the end of the DockWindow destructor.
    qDeleteAll(m_pageConnections);

#ifdef MUSE_MULTICONTEXT_WIP
    if (iocContext() && iocContext()->id > 0) {
        application()->destroyContext(iocContext());
    }
#endif
}

void DockWindow::componentComplete()
{
    TRACEFUNC;
    DOCK_CONTEXT_GUARD;

    QQuickItem::componentComplete();

    QString name = "mainWindow";
#ifdef MUSE_MULTICONTEXT_WIP
    if (iocContext()) {
        name += "_" + QString::number(iocContext()->id);
    }
#endif

    m_mainWindow = new KDDockWidgets::MainWindowQuick(name,
                                                      KDDockWidgets::MainWindowOption_None,
                                                      this);

#ifdef MUSE_MULTICONTEXT_WIP
    QString affinity = affinityName();
    if (!affinity.isEmpty()) {
        m_mainWindow->setAffinities({ affinity });
    }
#endif

    connect(qApp, &QCoreApplication::aboutToQuit, this, &DockWindow::onQuit);
    connect(this, &QQuickItem::windowChanged, this, &DockWindow::windowPropertyChanged);

    connect(this, &QQuickItem::widthChanged, this, [this]() {
        adjustContentForAvailableSpace(m_currentPage);
    });
}

QString DockWindow::affinityName() const
{
#ifdef MUSE_MULTICONTEXT_WIP
    if (iocContext()) {
        return QString("ctx_%1").arg(iocContext()->id);
    }
#endif
    return QString();
}

QString DockWindow::contextPageName(const QString& pageName) const
{
#ifdef MUSE_MULTICONTEXT_WIP
    if (iocContext()) {
        return pageName + "_ctx" + QString::number(iocContext()->id);
    }
#endif
    return pageName;
}

void DockWindow::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (!m_currentPage) {
        QQuickItem::geometryChange(newGeometry, oldGeometry);
        return;
    }

    //! NOTE: it is important to reset the current minimum width for all top-level toolbars
    //! Otherwise, the window content can be displaced after LayoutWidget::onResize(QSize newSize)
    //! due to lack of free space
    const QList<DockToolBarView*> topToolBars = topLevelToolBars(m_currentPage);
    for (DockToolBarView* toolBar : topToolBars) {
        toolBar->setMinimumWidth(toolBar->contentWidth());
    }

    QQuickItem::geometryChange(newGeometry, oldGeometry);

    alignTopLevelToolBars(m_currentPage);
}

void DockWindow::onQuit()
{
    TRACEFUNC;
    DOCK_CONTEXT_GUARD;

    IF_ASSERT_FAILED(m_currentPage) {
        return;
    }

    m_reloadCurrentPageAllowed = false;

    uiConfiguration()->setPageState(contextPageName(m_currentPage->objectName()), windowState());

    /// NOTE: The state of all dock widgets is also saved here,
    /// since the library does not provide the ability to save
    /// and restore only the application geometry.
#ifdef MUSE_MULTICONTEXT_WIP
    uiConfiguration()->setPageState(contextPageName(CONTEXT_GEOMETRY_KEY), windowState());
#else
    uiConfiguration()->setWindowGeometry(windowState());
#endif

    clearRegistry(m_currentPage);
}

QString DockWindow::currentPageUri() const
{
    return m_currentPage ? m_currentPage->uri() : QString();
}

QQmlListProperty<muse::dock::DockToolBarView> DockWindow::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<muse::dock::DockPageView> DockWindow::pagesProperty()
{
    return m_pages.property();
}

QQuickWindow* DockWindow::windowProperty() const
{
    return window();
}

void DockWindow::init()
{
    DOCK_CONTEXT_GUARD;

#ifdef MUSE_MULTICONTEXT_WIP
    for (const DockPageView* page : m_pages.list()) {
        clearRegistry(page);
    }
#else
    clearRegistry();
#endif
    restoreGeometry();

    dockWindowProvider()->init(this);

    int ctxId = iocContext() ? iocContext()->id : -1;

#ifdef MUSE_MULTICONTEXT_WIP
    uiConfiguration()->pageState(contextPageName(CONTEXT_GEOMETRY_KEY)).notification.onNotify(this, [this, ctxId]() {
        KDDockWidgets::ConfigContextGuard guard(ctxId);
        reloadCurrentPage();
    });
#else
    uiConfiguration()->windowGeometryChanged().onNotify(this, [this, ctxId]() {
        KDDockWidgets::ConfigContextGuard guard(ctxId);
        reloadCurrentPage();
    });
#endif

    workspaceManager()->currentWorkspaceAboutToBeChanged().onNotify(this, [this, ctxId]() {
        KDDockWidgets::ConfigContextGuard guard(ctxId);
        if (const DockPageView* page = currentPage()) {
            savePageState(page->objectName());
        }
    });
}

void DockWindow::loadPage(const QString& uri, const QVariantMap& params)
{
    TRACEFUNC;
    DOCK_CONTEXT_GUARD;

    if (currentPageUri() == uri) {
        if (m_currentPage) {
            m_currentPage->setParams(params);
        }
        return;
    }

    const bool isFirstOpening = (m_currentPage == nullptr);

    if (!isFirstOpening) {
        const QString pageName = m_currentPage->objectName();
        uiConfiguration()->pageState(contextPageName(pageName)).notification.disconnect(this);
        savePageState(pageName);
        clearRegistry(m_currentPage);
        m_currentPage->setVisible(false);
        m_currentPage->deinit();
    }

    bool ok = doLoadPage(uri, params);
    if (!ok) {
        return;
    }

    if (checkLayoutIsCorrupted()) {
        LOGE() << "Layout is corrupted, restoring default";
        restoreDefaultLayout();
    }

    auto notifyAboutPageLoaded = [this, &uri]() {
        emit currentPageUriChanged(uri);
        emit pageLoaded();
        notifyAboutDocksOpenStatus();
    };

    if (isFirstOpening) {
        async::Async::call(this, [this, notifyAboutPageLoaded]() {
            if (!m_hasGeometryBeenRestored
                || (m_mainWindow->windowHandle()->windowStates() & Qt::WindowFullScreen)) {
                //! NOTE: show window as maximized if no geometry has been restored
                //! or if the user had closed app in FullScreen mode
                m_mainWindow->windowHandle()->showMaximized();
            }

            notifyAboutPageLoaded();
        });
    } else {
        notifyAboutPageLoaded();
    }
}

bool DockWindow::isDockOpen(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockOpen(dockName);
}

void DockWindow::toggleDock(const QString& dockName)
{
    DOCK_CONTEXT_GUARD;
    setDockOpen(dockName, !isDockOpen(dockName));
}

void DockWindow::setDockOpen(const QString& dockName, bool open)
{
    DOCK_CONTEXT_GUARD;
    if (!m_currentPage) {
        return;
    }

    m_currentPage->setDockOpen(dockName, open);
    m_docksOpenStatusChanged.send({ dockName });
}

Channel<QStringList> DockWindow::docksOpenStatusChanged() const
{
    return m_docksOpenStatusChanged;
}

bool DockWindow::isDockFloating(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockFloating(dockName);
}

void DockWindow::toggleDockFloating(const QString& dockName)
{
    DOCK_CONTEXT_GUARD;
    if (m_currentPage) {
        m_currentPage->toggleDockFloating(dockName);
    }
}

DockPageView* DockWindow::currentPage() const
{
    return m_currentPage;
}

QQuickItem& DockWindow::asItem() const
{
    return *m_mainWindow;
}

void DockWindow::restoreDefaultLayout()
{
    TRACEFUNC;
    DOCK_CONTEXT_GUARD;

    //! HACK: notify about upcoming change of current URI
    //! so that all subscribers of this channel finish their work.
    //! For example, our popups and tooltips will close.
    interactive()->currentUriAboutToBeChanged().notify();

    if (m_currentPage) {
        for (DockBase* dock : m_currentPage->allDocks()) {
            dock->resetToDefault();
        }
    }

    m_reloadCurrentPageAllowed = false;
    for (const DockPageView* page : m_pages.list()) {
        uiConfiguration()->setPageState(contextPageName(page->objectName()), QByteArray());
    }

#ifdef MUSE_MULTICONTEXT_WIP
    uiConfiguration()->setPageState(contextPageName(CONTEXT_GEOMETRY_KEY), QByteArray());
#else
    uiConfiguration()->setWindowGeometry(QByteArray());
#endif
    m_reloadCurrentPageAllowed = true;

    reloadCurrentPage();
}

void DockWindow::loadPageContent(const DockPageView* page)
{
    TRACEFUNC;

    addDock(page->centralDock());

    loadPanels(page);
    loadToolBars(page);

    if (page->statusBar()) {
        addDock(page->statusBar(), Location::Bottom);
    }

    loadTopLevelToolBars(page);
}

void DockWindow::loadTopLevelToolBars(const DockPageView* page)
{
    TRACEFUNC;

    QList<DockToolBarView*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBarView* prevToolBar = nullptr;

    for (DockToolBarView* toolBar : allToolBars) {
        auto location = prevToolBar ? Location::Right : Location::Top;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }
}

void DockWindow::loadToolBars(const DockPageView* page)
{
    TRACEFUNC;

    for (DockToolBarView* toolBar : page->toolBars()) {
        addDock(toolBar, toolBar->location());
    }

    for (Location location : POSSIBLE_LOCATIONS) {
        if (auto holder = page->holder(DockType::ToolBar, location)) {
            addDock(holder, location);
        }
    }
}

void DockWindow::loadPanels(const DockPageView* page)
{
    TRACEFUNC;

    for (DockPanelView* panel : page->panels()) {
        if (DockPanelView* destinationPanel = page->findPanelForTab(panel)) {
            addPanelAsTab(panel, destinationPanel);
            continue;
        }

        const Location location = panel->location();
        const bool isSideLocation = location == Location::Left || location == Location::Right;
        addDock(panel, location, isSideLocation ? page->centralDock() : nullptr);
    }

    for (Location location : POSSIBLE_LOCATIONS) {
        if (auto holder = page->holder(DockType::Panel, location)) {
            addDock(holder, location);
        }
    }
}

void DockWindow::alignTopLevelToolBars(const DockPageView* page)
{
    QList<DockToolBarView*> topToolBars = topLevelToolBars(page);

    DockToolBarView* lastLeftToolBar = nullptr;
    DockToolBarView* lastCentralToolBar = nullptr;

    int leftToolBarsWidth = 0;
    int centralToolBarsWidth = 0;
    int rightToolBarsWidth = 0;

    int separatorThickness = KDDockWidgets::Config::self().separatorThickness();

    for (DockToolBarView* toolBar : topToolBars) {
        if (toolBar->floating() || !toolBar->isVisible()) {
            continue;
        }

        switch (static_cast<DockToolBarAlignment::Type>(toolBar->alignment())) {
        case DockToolBarAlignment::Left:
            lastLeftToolBar = toolBar;
            leftToolBarsWidth += toolBar->contentWidth();
            break;
        case DockToolBarAlignment::Center:
            lastCentralToolBar = toolBar;
            centralToolBarsWidth += (toolBar->contentWidth() + separatorThickness);
            break;
        case DockToolBarAlignment::Right:
            rightToolBarsWidth += (toolBar->contentWidth() + separatorThickness);
            break;
        }
    }

    if (!lastLeftToolBar || !lastCentralToolBar) {
        return;
    }

    int deltaForLastLeftToolbar = (width() - centralToolBarsWidth) / 2 - leftToolBarsWidth;
    int deltaForLastCentralToolBar = (width() - centralToolBarsWidth) / 2 - rightToolBarsWidth;

    deltaForLastLeftToolbar = std::max(deltaForLastLeftToolbar, 0);
    deltaForLastCentralToolBar = std::max(deltaForLastCentralToolBar, 0);

    int freeSpace = width() - (leftToolBarsWidth + centralToolBarsWidth + rightToolBarsWidth);

    if (deltaForLastLeftToolbar + deltaForLastCentralToolBar > freeSpace) {
        deltaForLastLeftToolbar = freeSpace;
        deltaForLastCentralToolBar = 0;
    }

    lastLeftToolBar->setMinimumWidth(lastLeftToolBar->contentWidth() + deltaForLastLeftToolbar);
    lastCentralToolBar->setMinimumWidth(lastCentralToolBar->contentWidth() + deltaForLastCentralToolBar);
}

void DockWindow::addDock(DockBase* dock, Location location, const DockBase* relativeTo)
{
    TRACEFUNC;

    registerDock(dock);

    KDDockWidgets::DockWidgetBase* relativeDock = relativeTo ? relativeTo->dockWidget() : nullptr;

    auto visibilityOption = dock->defaultVisibility() ? KDDockWidgets::InitialVisibilityOption::StartVisible
                            : KDDockWidgets::InitialVisibilityOption::StartHidden;

    KDDockWidgets::InitialOption options(visibilityOption, dock->preferredSize());

    m_mainWindow->addDockWidget(dock->dockWidget(), locationToKLocation(location), relativeDock, options);
}

void DockWindow::addPanelAsTab(DockPanelView* panel, DockPanelView* destinationPanel)
{
    registerDock(panel);

    if (panel->defaultVisibility()) {
        destinationPanel->addPanelAsTab(panel);
        destinationPanel->setCurrentTabIndex(0);
    }
}

void DockWindow::registerDock(DockBase* dock)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(dock) {
        return;
    }

    auto registry = KDDockWidgets::DockRegistry::self();
    auto dockWidget = dock->dockWidget();

#ifdef MUSE_MULTICONTEXT_WIP
    QString affinity = affinityName();
    if (!affinity.isEmpty() && dockWidget->affinities().isEmpty()) {
        dockWidget->setAffinityName(affinity);
    }
#endif

    if (!registry->containsDockWidget(dockWidget->uniqueName())) {
        registry->registerDockWidget(dockWidget);
    }
}

void DockWindow::handleUnknownDock(const DockPageView* page, DockBase* unknownDock)
{
    DockPanelView* unknownPanel = dynamic_cast<DockPanelView*>(unknownDock);
    if (!unknownPanel) {
        addDock(unknownDock, unknownDock->location(), page->centralDock());
        return;
    }

    if (DockPanelView* destinationPanel = page->findPanelForTab(unknownPanel)) {
        addPanelAsTab(unknownPanel, destinationPanel);
        return;
    }

    DockingHolderView* holder = page->holder(DockType::Panel, unknownPanel->location());
    IF_ASSERT_FAILED(holder) {
        addDock(unknownDock, unknownDock->location(), page->centralDock());
        return;
    }

    registerDock(unknownPanel);

    holder->open(); // init the frame...

    KDDockWidgets::Frame* frame = holder->dockWidget()->frame();
    frame->addWidget(unknownPanel->dockWidget());

    holder->close();

    if (!unknownPanel->isVisible()) {
        unknownPanel->close();
    }
}

DockPageView* DockWindow::pageByUri(const QString& uri) const
{
    for (DockPageView* page : m_pages.list()) {
        if (page->uri() == uri) {
            return page;
        }
    }

    return nullptr;
}

bool DockWindow::doLoadPage(const QString& uri, const QVariantMap& params)
{
    DockPageView* newPage = pageByUri(uri);
    IF_ASSERT_FAILED(newPage) {
        return false;
    }

    newPage->setVisible(true);

    loadPageContent(newPage);
    restorePageState(newPage);
    initDocks(newPage);

    newPage->setParams(params);

    m_currentPage = newPage;

    connect(m_currentPage, &DockPageView::layoutRequested,
            this, &DockWindow::forceLayout, Qt::UniqueConnection);

    return true;
}

void DockWindow::restoreGeometry()
{
    TRACEFUNC;

#ifdef MUSE_MULTICONTEXT_WIP
    QByteArray geometry = uiConfiguration()->pageState(contextPageName(CONTEXT_GEOMETRY_KEY)).val;
#else
    QByteArray geometry = uiConfiguration()->windowGeometry();
#endif

    if (geometry.isEmpty()) {
        return;
    }

    if (restoreLayout(geometry)) {
        m_hasGeometryBeenRestored = true;
    } else {
        LOGE() << "Could not restore the window geometry!";
    }
}

void DockWindow::savePageState(const QString& pageName)
{
    TRACEFUNC;

    m_reloadCurrentPageAllowed = false;
    uiConfiguration()->setPageState(contextPageName(pageName), windowState());
    m_reloadCurrentPageAllowed = true;
}

void DockWindow::restorePageState(const DockPageView* page)
{
    TRACEFUNC;

    const QString& pageName = page->objectName();

    ValNt<QByteArray> pageStateValNt = uiConfiguration()->pageState(contextPageName(pageName));
    const bool layoutIsEmpty = pageStateValNt.val.isEmpty();

    QSet<DockBase*> unknownDocks;
    if (!layoutIsEmpty) {
        for (DockBase* dock : page->allDocks()) {
            const KDDockWidgets::DockWidgetBase* dockWidget = dock->dockWidget();
            if (!pageStateValNt.val.contains(dockWidget->uniqueName().toLocal8Bit())) {
                unknownDocks.insert(dock);
            }
        }
    }

    /// NOTE: Do not restore geometry
    bool ok = restoreLayout(pageStateValNt.val, true /*restoreRelativeToMainWindow*/);
    if (!ok) {
        LOGE() << "Could not restore the state of " << pageName << "!";
    }

    if (!layoutIsEmpty) {
        for (DockBase* dock : unknownDocks) {
            handleUnknownDock(page, dock);
        }
    }

    if (!pageStateValNt.notification.isConnected()) {
        int ctxId = iocContext() ? iocContext()->id : -1;
        pageStateValNt.notification.onNotify(this, [this, pageName, ctxId]() {
            KDDockWidgets::ConfigContextGuard guard(ctxId);
            bool isCurrentPage = m_currentPage && (m_currentPage->objectName() == pageName);
            if (isCurrentPage) {
                reloadCurrentPage();
            }
        });
    }
}

bool DockWindow::restoreLayout(const QByteArray& layout, bool restoreRelativeToMainWindow)
{
    if (layout.isEmpty()) {
        return true;
    }

    TRACEFUNC;

    auto option = restoreRelativeToMainWindow ? KDDockWidgets::RestoreOption_RelativeToMainWindow
                  : KDDockWidgets::RestoreOption_None;

    KDDockWidgets::LayoutSaver layoutSaver(option);
#ifdef MUSE_MULTICONTEXT_WIP
    QString affinity = affinityName();
    if (!affinity.isEmpty()) {
        layoutSaver.setAffinityNames({ affinity });
    }
#endif
    return layoutSaver.restoreLayout(layout);
}

bool DockWindow::checkLayoutIsCorrupted() const
{
    TRACEFUNC;

    for (const DockBase* dock : m_currentPage->allDocks()) {
        if (!dock) {
            continue;
        }

        if (!dock->floatable() && dock->floating()) {
            return true;
        }
    }

    return false;
}

void DockWindow::forceLayout()
{
    m_mainWindow->layoutEqually();
}

QByteArray DockWindow::windowState() const
{
    TRACEFUNC;

    KDDockWidgets::LayoutSaver layoutSaver;
#ifdef MUSE_MULTICONTEXT_WIP
    QString affinity = affinityName();
    if (!affinity.isEmpty()) {
        layoutSaver.setAffinityNames({ affinity });
    }
#endif
    return layoutSaver.serializeLayout();
}

void DockWindow::reloadCurrentPage()
{
    if (!m_reloadCurrentPageAllowed) {
        return;
    }

    TRACEFUNC;
    DOCK_CONTEXT_GUARD;

    clearRegistry(m_currentPage);

    for (DockBase* dock : m_currentPage->allDocks()) {
        dock->deinit();
    }

    QString currentPageUriBackup = currentPageUri();

    /// NOTE: for reset geometry
    m_currentPage = nullptr;

    if (doLoadPage(currentPageUriBackup)) {
        notifyAboutDocksOpenStatus();
    }
}

void DockWindow::initDocks(DockPageView* page)
{
    TRACEFUNC;

    //! before init we should correct toolbars sizes
    adjustContentForAvailableSpace(page);

    for (DockToolBarView* toolbar : m_toolBars.list()) {
        toolbar->setParentItem(this);
        toolbar->init();
    }

    if (page) {
        page->setParentItem(this);
        page->init();
    }

    alignTopLevelToolBars(page);

    if (!m_pageConnections.contains(page)) {
        m_pageConnections[page] = new UniqueConnectionHolder(page, this);
    }

    UniqueConnectionHolder* holder = m_pageConnections[page];

    for (DockToolBarView* toolbar : topLevelToolBars(page)) {
        connect(toolbar, &DockToolBarView::floatingChanged,
                holder, &UniqueConnectionHolder::alignTopLevelToolBars, Qt::UniqueConnection);

        connect(toolbar, &DockToolBarView::contentSizeChanged,
                holder, &UniqueConnectionHolder::alignTopLevelToolBars, Qt::UniqueConnection);

        connect(toolbar, &DockToolBarView::visibleChanged,
                holder, &UniqueConnectionHolder::alignTopLevelToolBars, Qt::UniqueConnection);
    }
}

void DockWindow::adjustContentForAvailableSpace(DockPageView* page)
{
    if (!page) {
        return;
    }

    int spaceWidth = width();

    auto adjustDocks = [&spaceWidth](QList<DockBase*> docks) {
        int width = 0;
        for (DockBase* dock : docks) {
            width += dock->contentWidth();
        }

        docks.erase(std::remove_if(docks.begin(), docks.end(), [](const DockBase* dock){
            return dock->compactPriorityOrder() == -1;
        }), docks.end());

        if (docks.empty()) {
            return;
        }

        std::sort(docks.begin(), docks.end(), [](const DockBase* dock1, DockBase* dock2) {
            return dock1->compactPriorityOrder() < dock2->compactPriorityOrder();
        });

        if (width >= spaceWidth) {
            for (DockBase* dock : docks) {
                if (!dock->isCompact()) {
                    dock->setIsCompact(true);

                    width -= dock->nonCompactWidth();
                    width += dock->width();
                }
            }
        } else {
            for (int i = docks.size() - 1; i >= 0; i--) {
                DockBase* dock = docks.at(i);
                if (!dock->isCompact()) {
                    continue;
                }

                int actualWidth = dock->contentWidth();
                int nonCompactWidth = dock->nonCompactWidth();
                if (width - actualWidth + nonCompactWidth < spaceWidth) {
                    dock->setIsCompact(false);
                }

                break;
            }
        }
    };

    QList<DockBase*> topLevelToolBarsDocks;

    for (DockToolBarView* toolBar : topLevelToolBars(page)) {
        if (!toolBar->dockWidget()->isFloating() && toolBar->isVisible()) {
            topLevelToolBarsDocks << toolBar;
        }
    }

    adjustDocks(topLevelToolBarsDocks);
}

void DockWindow::notifyAboutDocksOpenStatus()
{
    const DockPageView* page = currentPage();

    IF_ASSERT_FAILED(page) {
        return;
    }

    QStringList dockNames;

    for (DockToolBarView* toolBar : page->mainToolBars()) {
        dockNames << toolBar->objectName();
    }

    for (DockToolBarView* toolBar : page->toolBars()) {
        dockNames << toolBar->objectName();
    }

    for (DockPanelView* panel : page->panels()) {
        dockNames << panel->objectName();
    }

    if (page->statusBar()) {
        dockNames << page->statusBar()->objectName();
    }

    m_docksOpenStatusChanged.send(dockNames);
}

QList<DockToolBarView*> DockWindow::topLevelToolBars(const DockPageView* page) const
{
    QList<DockToolBarView*> toolBars = m_toolBars.list();

    if (page) {
        toolBars << page->mainToolBars();
    }

    return toolBars;
}

#include "dockwindow.moc"
