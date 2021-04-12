//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "vstplugineditorview.h"

#include <QWindow>
#include <QResizeEvent>

#include "vsttypes.h"
#include "internal/vstplugin.h"

using namespace mu::vst;
using namespace Steinberg;

IMPLEMENT_FUNKNOWN_METHODS(VstPluginEditorView, IPlugFrame, IPlugFrame::iid)

VstPluginEditorView::VstPluginEditorView(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
}

VstPluginEditorView::VstPluginEditorView(const VstPluginEditorView& copy)
    : VstPluginEditorView(copy.parentWidget())
{
}

VstPluginEditorView::~VstPluginEditorView()
{
    if (m_view) {
        m_view->removed();
    }
}

tresult VstPluginEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    setGeometry(QRect(geometry().x(), geometry().y(), newSize->getWidth(), newSize->getHeight()));
    view->onSize(newSize);

    return kResultTrue;
}

QString VstPluginEditorView::pluginId() const
{
    return m_pluginId;
}

void VstPluginEditorView::setPluginId(QString pluginId)
{
    if (m_pluginId == pluginId) {
        return;
    }

    wrapPluginView(pluginId);

    m_pluginId = pluginId;
    emit pluginIdChanged(m_pluginId);
}

void VstPluginEditorView::wrapPluginView(const QString& pluginId)
{
    RetVal<VstPluginPtr> plugin = repository()->findPluginById(pluginId.toStdString());

    if (!plugin.ret) {
        return;
    }

    if (!plugin.val->view()) {
        return;
    }

    m_view = plugin.val->view();

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    m_view->setFrame(this);

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(windowHandle()->winId()), currentPlatformUiType());
    if (attached != kResultOk) {
        return;
    }

    ViewRect size;
    m_view->getSize(&size);
    resizeView(m_view, &size);
}

FIDString VstPluginEditorView::currentPlatformUiType() const
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
