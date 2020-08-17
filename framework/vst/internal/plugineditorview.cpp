//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "plugineditorview.h"
#include <QQmlContext>
#include "log.h"
using namespace mu::vst;
using namespace Steinberg;

DEF_CLASS_IID(IPlugFrame)
IMPLEMENT_FUNKNOWN_METHODS(PluginEditorView, IPlugFrame, IPlugFrame::iid)

std::map<IVSTInstanceRegister::instance_id, QWidget*> PluginEditorView::m_activeViews = {};

PluginEditorView::PluginEditorView(QWidget* parent)
    : QDialog(parent), m_instanceId(-1), m_view(nullptr)
{
    setAttribute(Qt::WA_NativeWindow, true);
}

PluginEditorView::PluginEditorView(const PluginEditorView& other)
    : QDialog(other.parentWidget()), m_instanceId(-1), m_view(nullptr)
{
    setAttribute(Qt::WA_NativeWindow, true);
}

PluginEditorView::~PluginEditorView()
{
    if (m_activeViews[instanceId()]) {
        m_activeViews[instanceId()] = nullptr;
    }
    if (m_view) {
        m_view->removed();
    }
}

int PluginEditorView::metaTypeId()
{
    return QMetaType::type("PluginEditorView");
}

void PluginEditorView::setInstanceId(IVSTInstanceRegister::instance_id id)
{
    IF_ASSERT_FAILED(m_instanceId == NOT_SETTED) {
        LOGE() << "can't reset instance id";
        return;
    }
    if (m_instanceId != id) {
        m_instanceId = id;
        initInstance();
        emit instanceIdChanged();
    }
}

void PluginEditorView::initInstance()
{
    if (m_activeViews.count(instanceId()) && m_activeViews[instanceId()]) {
        auto activeWidget = m_activeViews[instanceId()];
        m_instanceId = NOT_SETTED;
        activeWidget->raise();
        accept(); //set result code
        return;
    }
    m_activeViews[instanceId()] = this;

    auto instance = vstInstanceRegister()->instance(instanceId());
    IF_ASSERT_FAILED(instance) {
        return;
    }
    IF_ASSERT_FAILED(windowHandle()) {
        LOGE() << "widget doen't have native window handle";
        return;
    }
    m_view = instance->createView();

    if (m_view) {
        attachOriginalView();
    } else {
        attachQmlView(instance);
    }
}

Steinberg::tresult PluginEditorView::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize)
{
    auto windowPosition = mainWindow()->qMainWindow()->pos();
    QRect newGeometry(
        windowPosition.x(),
        windowPosition.y(),
        newSize->getWidth(),
        newSize->getHeight()
        );
    setGeometry(newGeometry);
    view->onSize(newSize);
    return kResultTrue;
}

void PluginEditorView::attachOriginalView()
{
#ifdef Q_OS_MAC
    FIDString platformType = kPlatformTypeNSView;
#elif Q_OS_IOS
    FIDString platformType = kPlatformTypeUIView;
#elif Q_OS_WIN
    FIDString platformType = kPlatformTypeHWND;
#elif Q_OS_LINUX
    FIDString platformType = kPlatformTypeX11EmbedWindowID;
#endif

    if (m_view->isPlatformTypeSupported(platformType) != kResultTrue) {
        LOGW() << "plugin doesn't support platform " << platformType;
        return;
    }

    m_view->setFrame(this);

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(windowHandle()->winId()), platformType);
    IF_ASSERT_FAILED_X(attached == kResultOk, "plugin's view was not attached") {
        return;
    }

    // some plugins don't call resize after attached
    ViewRect size;
    m_view->getSize(&size);
    resizeView(m_view, &size);
}

void PluginEditorView::attachQmlView(std::shared_ptr<PluginInstance> instance)
{
    setAttribute(Qt::WA_NativeWindow, false);
    resize(640, 480);
    setLayout(new QHBoxLayout);

    auto widget = new QQuickWidget(QUrl("qrc:/qml/VSTEditor.qml"));
    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    widget->rootContext()->setContextProperty("instance", instance.get());

    layout()->setMargin(0);
    layout()->setSpacing(0);
    layout()->addWidget(widget);
}
