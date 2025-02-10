/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "../internal/platform/linux/runloop.h"

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
    if (m_runLoop && Steinberg::FUnknownPrivate::iidEqual(_iid, Steinberg::Linux::IRunLoop::iid)) {
        m_runLoop->addRef();
        *obj = static_cast<Steinberg::Linux::IRunLoop*>(m_runLoop);
        return ::Steinberg::kResultOk;
    }
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
    : QQuickItem(parent)
{
    FUNKNOWN_CTOR; // IPlugFrame

//! NOTE Required for Linux only
#ifdef Q_OS_LINUX
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

    m_view->setFrame(this);

    m_window = new QWindow(window());

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(m_window->winId()), currentPlatformUiType());
    if (attached != Steinberg::kResultOk) {
        LOGE() << "Unable to attach vst plugin view to window"
               << ", instance name: " << m_instance->name();
        return;
    }

    connect(window(), &QWindow::screenChanged, this, [this](QScreen*) {
        updateScreenMetrics();
        updateViewGeometry();
    });

    updateScreenMetrics();
    updateViewGeometry();

    m_window->show();
}

void VstView::deinit()
{
    if (m_view) {
        m_view->setFrame(nullptr);
        m_view->removed();
        m_view = nullptr;

        m_window->hide();
        delete m_window;
        m_window = nullptr;
    }

    if (m_runLoop) {
        m_runLoop->stop();
        delete m_runLoop;
    }

    if (m_instance) {
        m_instance->refreshConfig();
        m_instance = nullptr;
    }
}

Steinberg::tresult VstView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSize)
{
    IF_ASSERT_FAILED(m_window) {
        return Steinberg::kResultFalse;
    }

    view->checkSizeConstraint(requiredSize);

    int newWidth = requiredSize->getWidth();
    int newHeight = requiredSize->getHeight();

//! NOTE: newSize already includes the UI scaling on Windows, so we have to remove it before setting the fixed size.
//! Otherwise, the user will get an extremely large window and won't be able to resize it
#ifdef Q_OS_WIN
    newWidth = newWidth / m_screenMetrics.devicePixelRatio;
    newHeight = newHeight / m_screenMetrics.devicePixelRatio;
#endif

    newWidth = std::min(newWidth, m_screenMetrics.availableSize.width());
    newHeight = std::min(newHeight, m_screenMetrics.availableSize.height());

    setImplicitHeight(newHeight);
    setImplicitWidth(newWidth);

    m_window->setGeometry(this->x(), this->y(), this->implicitWidth(), this->implicitHeight());
    Steinberg::ViewRect vstSize;
    vstSize.right = m_window->width() * m_screenMetrics.devicePixelRatio;
    vstSize.bottom = m_window->height() * m_screenMetrics.devicePixelRatio;
    view->onSize(&vstSize);

    return Steinberg::kResultTrue;
}

void VstView::updateScreenMetrics()
{
    QScreen* screen = window()->screen();
    m_screenMetrics.availableSize = screen->availableSize();
    m_screenMetrics.devicePixelRatio = screen->devicePixelRatio();
}

void VstView::updateViewGeometry()
{
    IF_ASSERT_FAILED(m_view) {
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
