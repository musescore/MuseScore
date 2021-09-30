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

#include "vstieditorview.h"

#include <QWindow>
#include <QResizeEvent>

#include "log.h"

#include "vsttypes.h"
#include "internal/vstplugin.h"

using namespace mu::vst;
using namespace Steinberg;

IMPLEMENT_FUNKNOWN_METHODS(VstiEditorView, IPlugFrame, IPlugFrame::iid)

VstiEditorView::VstiEditorView(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
}

VstiEditorView::VstiEditorView(const VstiEditorView& copy)
    : VstiEditorView(copy.parentWidget())
{
}

VstiEditorView::~VstiEditorView()
{
    if (m_view) {
        m_view->removed();
    }

    if (m_pluginPtr) {
        m_pluginPtr->loadingCompleted().resetOnNotify(this);
    }
}

tresult VstiEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    setGeometry(QRect(geometry().x(), geometry().y(), newSize->getWidth(), newSize->getHeight()));
    view->onSize(newSize);

    update();

    return kResultTrue;
}

void VstiEditorView::wrapPluginView()
{
    m_pluginPtr = pluginsRegister()->instrumentPlugin(m_trackId, m_resourceId.toStdString());

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

void VstiEditorView::attachView(VstPluginPtr pluginPtr)
{
    m_view = pluginPtr->view();

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    m_view->setFrame(this);

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(windowHandle()->winId()), currentPlatformUiType());
    if (attached != kResultOk) {
        LOGE() << "Unable to attach vst fx plugin view to window"
               << ", resourceId: " << m_resourceId;
        return;
    }
}

FIDString VstiEditorView::currentPlatformUiType() const
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

int VstiEditorView::trackId() const
{
    return m_trackId;
}

void VstiEditorView::setTrackId(int newTrackId)
{
    if (m_trackId == newTrackId) {
        return;
    }
    m_trackId = newTrackId;
    emit trackIdChanged();

    if (m_trackId != -1 && !m_resourceId.isEmpty()) {
        wrapPluginView();
    }
}

const QString& VstiEditorView::resourceId() const
{
    return m_resourceId;
}

void VstiEditorView::setResourceId(const QString& newResourceId)
{
    if (m_resourceId == newResourceId) {
        return;
    }
    m_resourceId = newResourceId;
    emit resourceIdChanged();

    setWindowTitle(newResourceId);

    if (m_trackId != -1 && !m_resourceId.isEmpty()) {
        wrapPluginView();
    }
}
