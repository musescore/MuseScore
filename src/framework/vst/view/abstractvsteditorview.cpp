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

#include "abstractvsteditorview.h"

#include <QWindow>
#include <QTimer>

#include "vsttypes.h"
#include "internal/vstplugin.h"

#include "async/async.h"
#include "log.h"

using namespace mu::vst;
using namespace Steinberg;

AbstractVstEditorView::AbstractVstEditorView(QWidget* parent)
    : TopLevelDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    m_scalingFactor = uiConfig()->guiScaling();
}

AbstractVstEditorView::~AbstractVstEditorView()
{
    if (m_view) {
        m_view->removed();
    }

    if (m_pluginPtr) {
        m_pluginPtr->loadingCompleted().resetOnNotify(this);
        m_pluginPtr->refreshConfig();
    }
}

tresult AbstractVstEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    setFixedSize(newSize->getWidth(), newSize->getHeight());
    view->onSize(newSize);

    update();

    return kResultTrue;
}

void AbstractVstEditorView::wrapPluginView()
{
    m_pluginPtr = getPluginPtr();

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

void AbstractVstEditorView::attachView(VstPluginPtr pluginPtr)
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
        LOGE() << "Unable to attach vst plugin view to window"
               << ", resourceId: " << m_resourceId;
        return;
    }

    FUnknownPtr<IPluginContentScaleHandler> scalingHandler(m_view);
    if (scalingHandler) {
        scalingHandler->setContentScaleFactor(m_scalingFactor);
    }

    QTimer::singleShot(0, [this]() {
        setupWindowGeometry();
    });
}

void AbstractVstEditorView::setupWindowGeometry()
{
    ViewRect size;
    m_view->getSize(&size);

    resizeView(m_view, &size);

    moveViewToMainWindowCenter();
}

void AbstractVstEditorView::moveViewToMainWindowCenter()
{
    QRectF mainWindowGeo = mainWindow()->qWindow()->geometry();

    int x = mainWindowGeo.x() + (mainWindowGeo.width() - width()) / 2;
    int y = mainWindowGeo.y() + (mainWindowGeo.height() - height()) / 2;

    move(x, y);
}

void AbstractVstEditorView::showEvent(QShowEvent* event)
{
    moveViewToMainWindowCenter();

    QDialog::showEvent(event);
}

FIDString AbstractVstEditorView::currentPlatformUiType() const
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

int AbstractVstEditorView::trackId() const
{
    return m_trackId;
}

void AbstractVstEditorView::setTrackId(int newTrackId)
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

const QString& AbstractVstEditorView::resourceId() const
{
    return m_resourceId;
}

void AbstractVstEditorView::setResourceId(const QString& newResourceId)
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
