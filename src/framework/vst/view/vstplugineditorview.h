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

#ifndef VSTPLUGINEDITORVIEW_H
#define VSTPLUGINEDITORVIEW_H

#include <QDialog>
#include <QWidget>

#include "ivstpluginrepository.h"
#include "modularity/ioc.h"

namespace mu::vst {
class VstPluginEditorView : public QDialog, public Steinberg::IPlugFrame
{
    Q_OBJECT

    Q_PROPERTY(QString pluginId READ pluginId WRITE setPluginId NOTIFY pluginIdChanged)

    INJECT(vst, IVstPluginRepository, repository)

    DECLARE_FUNKNOWN_METHODS
public:
    VstPluginEditorView(QWidget* parent = nullptr);
    VstPluginEditorView(const VstPluginEditorView& copy);

    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

    QString pluginId() const;

public slots:
    void setPluginId(QString pluginId);

signals:
    void pluginIdChanged(QString pluginId);

private:
    void wrapPluginView(const QString& pluginId);

    FIDString currentPlatformUiType() const;

    PluginView m_view = nullptr;
    QString m_pluginId;
};
}

Q_DECLARE_METATYPE(mu::vst::VstPluginEditorView)

#endif // VSTPLUGINEDITORVIEW_H
