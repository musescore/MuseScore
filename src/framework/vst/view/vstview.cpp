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

IMPLEMENT_QUERYINTERFACE(VstView, IPlugFrame, IPlugFrame::iid)

VstView::VstView(QQuickItem* parent)
    : QQuickItem(parent)
{

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

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(window()->winId()), currentPlatformUiType());
    if (attached != Steinberg::kResultOk) {
        LOGE() << "Unable to attach vst plugin view to window"
               << ", instance name: " << m_instance->name();
        return;
    }

    connect(window(), &QWindow::screenChanged, this, [this](QScreen*) {
        updateViewGeometry();
    });

    updateViewGeometry();
}

Steinberg::tresult VstView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize)
{
    view->checkSizeConstraint(newSize);

    QScreen* screen = window()->screen();
    QSize availableSize = screen->availableSize();

    int newWidth = newSize->getWidth();
    int newHeight = newSize->getHeight();

//! NOTE: newSize already includes the UI scaling on Windows, so we have to remove it before setting the fixed size.
//! Otherwise, the user will get an extremely large window and won't be able to resize it
#ifdef Q_OS_WIN
    qreal scaling = screen->devicePixelRatio();
    newWidth = newWidth / scaling;
    newHeight = newHeight / scaling;
#endif

    newWidth = std::min(newWidth, availableSize.width());
    newHeight = std::min(newHeight, availableSize.height());

    setImplicitHeight(newHeight);
    setImplicitWidth(newWidth);

    view->onSize(newSize);

    update();

    return Steinberg::kResultTrue;
}

void VstView::updateViewGeometry()
{
    IF_ASSERT_FAILED(m_view) {
        return;
    }

#ifdef Q_OS_WIN
    Steinberg::FUnknownPtr<IPluginContentScaleHandler> scalingHandler(m_view);
    if (scalingHandler) {
        scalingHandler->setContentScaleFactor(window()->screen()->devicePixelRatio());
    }
#endif

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
    if (m_instanceId == newInstanceId)
        return;
    m_instanceId = newInstanceId;
    emit instanceIdChanged();
}

QString VstView::title() const
{
    return m_title;
}
