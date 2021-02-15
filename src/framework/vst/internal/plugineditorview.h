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
#ifndef PLUGINEDITORVIEW_H
#define PLUGINEDITORVIEW_H

#include <QObject>
#include <QDialog>

#include "internal/plugininstance.h"
#include "pluginterfaces/gui/iplugview.h"
#include "modularity/ioc.h"
#include "framework/ui/imainwindow.h"
#include "ui/iuiengine.h"

namespace mu {
namespace vst {
class PluginEditorView : public QDialog, public Steinberg::IPlugFrame
{
    Q_OBJECT
    Q_PROPERTY(int instanceId READ id WRITE setId NOTIFY idChanged)
    INJECT(ui, mu::ui::IMainWindow, mainWindow)
    INJECT(vst, IVSTInstanceRegister, vstInstanceRegister)
    INJECT(ui, mu::ui::IUiEngine, uiEngine)
public:
    PluginEditorView(QWidget* parent = nullptr);
    PluginEditorView(const PluginEditorView& other);
    ~PluginEditorView();

    DECLARE_FUNKNOWN_METHODS

    static int metaTypeId();

    instanceId id() const;
    void setId(instanceId id);
    virtual Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

private:
    instanceId m_instanceId = IVSTInstanceRegister::ID_NOT_SETTED;
    Steinberg::IPlugView* m_view = nullptr;

    void initInstance();

    //! attach view from plugin's UI
    void attachOriginalView();
    void attachQmlView(std::shared_ptr<PluginInstance> instance);

    static std::map<instanceId, QWidget*> m_activeViews;

signals:
    void idChanged();
};
}
}
Q_DECLARE_METATYPE(mu::vst::PluginEditorView)

#endif // PLUGINEDITORVIEW_H
