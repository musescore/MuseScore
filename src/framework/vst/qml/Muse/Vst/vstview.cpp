/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "vstview.h"

#include <QQuickWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#define USE_LINUX_RUNLOOP
#endif

#ifdef USE_LINUX_RUNLOOP
#include "platform/linux/runloop.h"
#endif

#include "global/types/number.h"
#include "log.h"

using namespace muse::vst;

::Steinberg::uint32 PLUGIN_API VstView::addRef()
{
    return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

::Steinberg::uint32 PLUGIN_API VstView::release()
{
    if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0) {
        return 0;
    }
    return __funknownRefCount;
}

Steinberg::tresult VstView::queryInterface(const ::Steinberg::TUID _iid, void** obj)
{
    QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::IPlugFrame);
    QUERY_INTERFACE(_iid, obj, Steinberg::IPlugFrame::iid, Steinberg::IPlugFrame);
    //As VST3 documentation states, IPlugFrame also has to provide
    //reference to the Steinberg::Linux::IRunLoop implementation.
#ifdef USE_LINUX_RUNLOOP
    if (m_runLoop && Steinberg::FUnknownPrivate::iidEqual(_iid, Steinberg::Linux::IRunLoop::iid)) {
        m_runLoop->addRef();
        *obj = static_cast<Steinberg::Linux::IRunLoop*>(m_runLoop);
        return ::Steinberg::kResultOk;
    }
#endif
    *obj = nullptr;
    return ::Steinberg::kNoInterface;
}

static FIDString currentPlatformUiType()
{
#ifdef Q_OS_MAC
    return Steinberg::kPlatformTypeNSView;
#elif defined(Q_OS_IOS)
    return Steinberg::kPlatformTypeUIView;
#elif defined(Q_OS_WIN)
    return Steinberg::kPlatformTypeHWND;
#else
    return Steinberg::kPlatformTypeX11EmbedWindowID;
#endif
}

VstView::VstView(QQuickItem* parent)
    : QQuickItem(parent), QAbstractNativeEventFilter(), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    FUNKNOWN_CTOR; // IPlugFrame

#ifdef USE_LINUX_RUNLOOP
    m_runLoop = new RunLoop();
#endif
}

VstView::~VstView()
{
    FUNKNOWN_DTOR; // IPlugFrame

    deinit();
}

void VstView::init()
{
    m_instance = instancesRegister()->instanceById(m_instanceId);
    IF_ASSERT_FAILED(m_instance) {
        return;
    }

    m_title = QString::fromStdString(m_instance->name());
    emit titleChanged();

    m_view = m_instance->createView();
    if (!m_view) {
        return;
    }

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    updateScreenMetrics();

    m_view->setFrame(this);

    m_vstWindow = new QWindow(window());

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(m_vstWindow->winId()), currentPlatformUiType());
    if (attached != Steinberg::kResultOk) {
        LOGE() << "Unable to attach vst plugin view to window"
               << ", instance name: " << m_instance->name();
        return;
    }

    // Do not rely on `QWindow::screenChanged` signal, which often does not get emitted though it should.
    // Proactively check for screen resolution changes instead.
    connect(&m_screenMetricsTimer, &QTimer::timeout, this, [this]() {
        QWindow* win = window();
        if (!win) {
            return;
        }
        const QScreen* const screen = win->screen();
        if (m_currentScreen != screen || m_screenMetrics.availableSize != screen->availableSize()
            || !is_equal(m_screenMetrics.devicePixelRatio, screen->devicePixelRatio())) {
            updateScreenMetrics();
            updateViewGeometry();
        }
    });
    m_screenMetricsTimer.start(std::chrono::milliseconds { 100 });

    updateViewGeometry();

    m_vstWindow->show();

#ifdef Q_OS_WIN
    HWND winHwnd = reinterpret_cast<HWND>(m_vstWindow->winId());
    HWND viewHwnd = GetWindow(winHwnd, GW_CHILD);
    m_rootHandle = GetAncestor(winHwnd, GA_ROOT);

    if (viewHwnd && IsWindow(viewHwnd)) {
        m_plugViewHandle = viewHwnd;
        qApp->installNativeEventFilter(this);
    }
#endif
}

void VstView::deinit()
{
#ifdef Q_OS_WIN
    qApp->removeNativeEventFilter(this);
#endif

    m_screenMetricsTimer.stop();

    if (m_view) {
        m_view->setFrame(nullptr);
#ifndef Q_OS_MAC
        m_view->removed();
#endif
        m_view = nullptr;
        m_plugViewHandle = nullptr;
        m_rootHandle = nullptr;

        m_vstWindow->hide();
        delete m_vstWindow;
        m_vstWindow = nullptr;
    }

#ifdef USE_LINUX_RUNLOOP
    if (m_runLoop) {
        m_runLoop->stop();
        delete m_runLoop;
    }
#endif

    if (m_instance) {
        m_instance->refreshConfig();
        m_instance = nullptr;
    }
}

Steinberg::tresult VstView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSize)
{
    IF_ASSERT_FAILED(m_vstWindow) {
        return Steinberg::kResultFalse;
    }

    // anti recursion
    if (m_resizeViewCalled) {
        return Steinberg::kResultTrue;
    }

    m_resizeViewCalled = true;

    view->checkSizeConstraint(requiredSize);

    const int titleBarHeight = window()->frameGeometry().height() - window()->geometry().height();

    const int requiredWidth = requiredSize->getWidth() / m_screenMetrics.devicePixelRatio;
    const int requiredHeight = requiredSize->getHeight() / m_screenMetrics.devicePixelRatio;

    const int availableWidth = m_screenMetrics.availableSize.width() - 2 * m_sidePadding;
    const int availableHeight = m_screenMetrics.availableSize.height() - titleBarHeight - m_topPadding - m_bottomPadding;

    const int newWidth = std::min(requiredWidth, availableWidth);
    const int newHeight = std::min(requiredHeight, availableHeight);

    const int implicitWidth = std::max(m_minimumWidth, newWidth);
    setImplicitWidth(implicitWidth);
    setImplicitHeight(newHeight);

    const int sidePadding = std::max(m_sidePadding, (implicitWidth - newWidth) / 2);
    m_vstWindow->setGeometry(sidePadding, m_topPadding, newWidth, newHeight);

    Steinberg::ViewRect vstSize;
    vstSize.right = newWidth * m_screenMetrics.devicePixelRatio;
    vstSize.bottom = newHeight * m_screenMetrics.devicePixelRatio;
    // Some VST plugins won't cooperate and size down, in which case their UI will be clipped
    // by `m_vstWindow`. This is better than an overflow, because the host might want to place
    // control buttons below the UI, and these ought not to be hidden.
    view->onSize(&vstSize);

    m_resizeViewCalled = false;

    return Steinberg::kResultTrue;
}

bool VstView::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (!m_vstWindow || !m_vstWindow->isVisible() || !m_plugViewHandle || !m_rootHandle) {
        return false;
    }

    if (eventType != QByteArrayLiteral("windows_generic_MSG")) {
        return false;
    }

    const MSG* msg = static_cast<MSG*>(message);
    if (!msg->hwnd) {
        return false;
    }
    if (msg->message != WM_ERASEBKGND && msg->message != WM_WINDOWPOSCHANGED) {
        return false;
    }

    if (msg->hwnd != m_rootHandle) {
        const HWND root = GetAncestor(msg->hwnd, GA_ROOT);
        if (root != m_rootHandle) {
            return false;
        }
    }

    if (msg->message == WM_ERASEBKGND) {
        *result = 1;
        return true; // tell Windows: "already erased"
    }

    if (msg->message == WM_WINDOWPOSCHANGED && !m_updatePending) {
        m_updatePending = true;
        QTimer::singleShot(16, this, [this]() { // ~60 FPS
            m_updatePending = false;

            HWND hwnd = reinterpret_cast<HWND>(m_plugViewHandle);
            if (!hwnd || !IsWindow(hwnd)) {
                m_plugViewHandle = nullptr;
                return;
            }

            RedrawWindow(hwnd, nullptr, nullptr,
                         RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
        });
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif

    return false;
}

void VstView::updateScreenMetrics()
{
    m_currentScreen = window()->screen();
    m_screenMetrics.availableSize = m_currentScreen->availableSize();
#ifdef Q_OS_MAC
    constexpr auto devicePixelRatio = 1.0;
#else
    const auto devicePixelRatio = m_currentScreen->devicePixelRatio();
#endif
    m_screenMetrics.devicePixelRatio = devicePixelRatio;
}

void VstView::updateViewGeometry()
{
    if (!m_view) {
        return;
    }

    Steinberg::ViewRect size;
    m_view->getSize(&size);

    resizeView(m_view, &size);
}

int VstView::instanceId() const
{
    return m_instanceId;
}

void VstView::setInstanceId(int newInstanceId)
{
    if (m_instanceId == newInstanceId) {
        return;
    }
    m_instanceId = newInstanceId;
    emit instanceIdChanged();
}

QString VstView::title() const
{
    return m_title;
}

int VstView::sidePadding() const
{
    return m_sidePadding;
}

void VstView::setsidePadding(int sidePadding)
{
    if (m_sidePadding == sidePadding) {
        return;
    }
    m_sidePadding = sidePadding;
    emit sidePaddingChanged();
    updateViewGeometry();
}

int VstView::topPadding() const
{
    return m_topPadding;
}

void VstView::setTopPadding(int topPadding)
{
    if (m_topPadding == topPadding) {
        return;
    }
    m_topPadding = topPadding;
    emit topPaddingChanged();
    updateViewGeometry();
}

int VstView::bottomPadding() const
{
    return m_bottomPadding;
}

void VstView::setBottomPadding(int bottomPadding)
{
    if (m_bottomPadding == bottomPadding) {
        return;
    }
    m_bottomPadding = bottomPadding;
    emit bottomPaddingChanged();
    updateViewGeometry();
}

int VstView::minimumWidth() const
{
    return m_minimumWidth;
}

void VstView::setMinimumWidth(int minimumWidth)
{
    if (m_minimumWidth == minimumWidth) {
        return;
    }
    m_minimumWidth = minimumWidth;
    emit minimumWidthChanged();
    updateViewGeometry();
}
