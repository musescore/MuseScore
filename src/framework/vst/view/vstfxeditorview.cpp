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

#include "vstfxeditorview.h"

#include <QWindow>
#include <QResizeEvent>

#include "log.h"
#include "async/async.h"

#include "vsttypes.h"
#include "internal/vstplugin.h"

using namespace mu::vst;
using namespace Steinberg;

IMPLEMENT_FUNKNOWN_METHODS(VstFxEditorView, IPlugFrame, IPlugFrame::iid)

VstFxEditorView::VstFxEditorView(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    updateStayOnTopness();
    connect(qApp, &QApplication::applicationStateChanged, this, &VstFxEditorView::updateStayOnTopness);
}

VstFxEditorView::VstFxEditorView(const VstFxEditorView& copy)
    : VstFxEditorView(copy.parentWidget())
{
}

VstFxEditorView::~VstFxEditorView()
{
    if (m_view) {
        m_view->removed();
    }

    if (m_pluginPtr) {
        m_pluginPtr->loadingCompleted().resetOnNotify(this);
    }
}

tresult VstFxEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    setGeometry(QRect(geometry().x(), geometry().y(), newSize->getWidth(), newSize->getHeight()));
    view->onSize(newSize);

    return kResultTrue;
}

bool VstFxEditorView::isAbleToWrapPlugin() const
{
    return !m_resourceId.isEmpty() && m_chainOrder != -1;
}

void VstFxEditorView::wrapPluginView()
{
    if (m_trackId == -1) {
        m_pluginPtr = pluginsRegister()->masterFxPlugin(m_resourceId.toStdString(), m_chainOrder);
    } else {
        m_pluginPtr = pluginsRegister()->fxPlugin(m_trackId, m_resourceId.toStdString(), m_chainOrder);
    }

    if (!m_pluginPtr) {
        return;
    }

    if (m_pluginPtr->isLoaded()) {
        attachView(m_pluginPtr);
    } else {
        m_pluginPtr->loadingCompleted().onNotify(this, [this]() {
            attachView(m_pluginPtr);
        });
    }
}

void VstFxEditorView::attachView(VstPluginPtr pluginPtr)
{
    if (!pluginPtr || !pluginPtr->view()) {
        return;
    }

    m_view = pluginPtr->view();

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    m_view->setFrame(this);

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(windowHandle()->winId()), currentPlatformUiType());
    if (attached != kResultOk) {
        LOGE() << "Unable to attach vsti plugin view to window"
               << ", resourceId: " << m_resourceId;
        return;
    }
}

FIDString VstFxEditorView::currentPlatformUiType() const
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

int VstFxEditorView::trackId() const
{
    return m_trackId;
}

void VstFxEditorView::setTrackId(int newTrackId)
{
    if (m_trackId == newTrackId) {
        return;
    }
    m_trackId = newTrackId;
    emit trackIdChanged();

    if (isAbleToWrapPlugin()) {
        wrapPluginView();
    }
}

const QString& VstFxEditorView::resourceId() const
{
    return m_resourceId;
}

void VstFxEditorView::setResourceId(const QString& newResourceId)
{
    if (m_resourceId == newResourceId) {
        return;
    }
    m_resourceId = newResourceId;
    emit resourceIdChanged();

    setWindowTitle(newResourceId);

    if (isAbleToWrapPlugin()) {
        wrapPluginView();
    }
}

int VstFxEditorView::chainOrder() const
{
    return m_chainOrder;
}

void VstFxEditorView::setChainOrder(int newChainOrder)
{
    if (m_chainOrder == newChainOrder) {
        return;
    }
    m_chainOrder = newChainOrder;
    emit chainOrderChanged();

    if (isAbleToWrapPlugin()) {
        wrapPluginView();
    }
}

// We want the VST windows to be on top of the main window.
// But not on top of all other applications when MuseScore isn't active.
// Qt has no API for this, so we use a hack.
// When the application becomes active, the VST windows will get the StayOnTop hint.
// When the application becomes inactive, the hint will be dropped.
// The nice thing is that it works... (with some ceremony)
void VstFxEditorView::updateStayOnTopness()
{
    auto setStayOnTop = [this](bool stay) {
        bool wasShown = isVisible();
        bool wasActive = isActiveWindow();

        setWindowFlag(Qt::WindowStaysOnTopHint, stay);
        if (wasShown) {
            if (!wasActive) {
                setAttribute(Qt::WA_ShowWithoutActivating, true);
            }
            show();
            setAttribute(Qt::WA_ShowWithoutActivating, false);
        }
    };

    setStayOnTop(qApp->applicationState() == Qt::ApplicationActive);
}
