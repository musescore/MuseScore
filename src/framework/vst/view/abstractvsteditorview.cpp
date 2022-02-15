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
#include <QResizeEvent>
#include <QTimer>

#include "log.h"
#include "async/async.h"

#include "vsttypes.h"
#include "internal/vstplugin.h"

using namespace mu::vst;
using namespace Steinberg;

AbstractVstEditorView::AbstractVstEditorView(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);

    // We want the VST windows to be on top of the main window.
    // But not on top of all other applications when MuseScore isn't active.
    // On Windows, we achieve this by setting the transient parent.
    // On macOS, we have to use a workaround:
    // When the application becomes active, the VST windows will get the StayOnTop hint.
    // and when the application becomes inactive, the hint will be removed.
#ifdef Q_OS_MAC
    auto updateStayOnTopHint = [this]() {
        bool stay = qApp->applicationState() == Qt::ApplicationActive;

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
    updateStayOnTopHint();
    connect(qApp, &QApplication::applicationStateChanged, this, updateStayOnTopHint);
#else
    windowHandle()->setTransientParent(mainWindow()->qWindow());
#endif
}

AbstractVstEditorView::~AbstractVstEditorView()
{
    if (m_view) {
        m_view->removed();
    }

    if (m_pluginPtr) {
        m_pluginPtr->loadingCompleted().resetOnNotify(this);
    }
}

tresult AbstractVstEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
// TODO: the problem with the window size still exists on Windows
// See: https://github.com/musescore/MuseScore/issues/9756#issuecomment-1042903918
// The recommended solution is to implement Steinberg::IPlugViewContentScaleSupport
#ifdef Q_OS_WIN
    setGeometry(QRect(geometry().x(), geometry().y(), newSize->getWidth(), newSize->getHeight()));
#else
    setFixedSize(newSize->getWidth(), newSize->getHeight());
#endif

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
