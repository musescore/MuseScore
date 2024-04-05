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
#include <QKeyEvent>
#include <QScreen>

#include "vsttypes.h"
#include "internal/vstplugin.h"

#include "async/async.h"
#include "log.h"

using namespace muse::vst;
using namespace Steinberg;

::Steinberg::uint32 PLUGIN_API AbstractVstEditorView::addRef()
{
    return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

::Steinberg::uint32 PLUGIN_API AbstractVstEditorView::release()
{
    if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0) {
        return 0;
    }
    return __funknownRefCount;
}

IMPLEMENT_QUERYINTERFACE(AbstractVstEditorView, IPlugFrame, IPlugFrame::iid)

AbstractVstEditorView::AbstractVstEditorView(QWidget* parent)
    : TopLevelDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow);
}

AbstractVstEditorView::~AbstractVstEditorView()
{
    deinit();
}

void AbstractVstEditorView::deinit()
{
    if (m_view) {
        m_view->setFrame(nullptr);
        m_view->removed();
        m_view = nullptr;
    }

    if (m_pluginPtr) {
        m_pluginPtr->loadingCompleted().resetOnNotify(this);
        m_pluginPtr->refreshConfig();
        m_pluginPtr = nullptr;
    }
}

tresult AbstractVstEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    view->checkSizeConstraint(newSize);

    QScreen* screen = this->screen();
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

    setFixedSize(newWidth, newHeight);

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
    if (!pluginPtr) {
        return;
    }

    m_view = pluginPtr->createView();
    if (!m_view) {
        return;
    }

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

    connect(windowHandle(), &QWindow::screenChanged, this, [this](QScreen*) {
        updateViewGeometry();
    });

    QTimer::singleShot(0, [this]() {
        updateViewGeometry();
        moveViewToMainWindowCenter();
    });
}

void AbstractVstEditorView::updateViewGeometry()
{
    IF_ASSERT_FAILED(m_view) {
        return;
    }

#ifdef Q_OS_WIN
    FUnknownPtr<IPluginContentScaleHandler> scalingHandler(m_view);
    if (scalingHandler) {
        scalingHandler->setContentScaleFactor(screen()->devicePixelRatio());
    }
#endif

    ViewRect size;
    m_view->getSize(&size);

    resizeView(m_view, &size);
}

void AbstractVstEditorView::moveViewToMainWindowCenter()
{
    QRectF mainWindowGeo = mainWindow()->qWindow()->geometry();

    int x = mainWindowGeo.x() + (mainWindowGeo.width() - width()) / 2;
    int y = mainWindowGeo.y() + (mainWindowGeo.height() - height()) / 2;

    move(x, y);
}

void AbstractVstEditorView::showEvent(QShowEvent* ev)
{
    moveViewToMainWindowCenter();

    TopLevelDialog::showEvent(ev);
}

void AbstractVstEditorView::closeEvent(QCloseEvent* ev)
{
    deinit();

    TopLevelDialog::closeEvent(ev);
}

bool AbstractVstEditorView::event(QEvent* ev)
{
    if (ev && ev->spontaneous() && ev->type() == QEvent::ShortcutOverride) {
        if (QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(ev)) {
            int key = keyEvent->key();

            if (key == 0 || key == static_cast<int>(Qt::Key_unknown)) {
                keyEvent->accept();
                return true;
            }
        }
    }

    return TopLevelDialog::event(ev);
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
