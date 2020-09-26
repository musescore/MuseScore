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
#ifndef MU4_VST_VSTDEVTOOLS_H
#define MU4_VST_VSTDEVTOOLS_H

#include <QObject>
#include <QString>
#include <QQmlListProperty>

#include "internal/vstscanner.h"
#include "view/pluginlistmodel.h"
#include "view/vstinstanceeditormodel.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/isequencer.h"
#include "ivstinstanceregister.h"
#include "framework/global/iinteractive.h"

namespace mu {
namespace vst {
class VSTDevTools : public QObject, public async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(PluginListModel * plugins READ plugins CONSTANT)
    Q_PROPERTY(QQmlListProperty<mu::vst::VSTInstanceEditorModel> instances READ instances NOTIFY instancesChanged)

    INJECT(vst, IVSTInstanceRegister, vstInstanceRegister)
    INJECT(vst, VSTScanner, vstScanner) //for PluginListModel
    INJECT(vst, audio::ISequencer, sequencer) //for play
    INJECT(ui, mu::framework::IInteractive, interactive)//for showEditor

public:
    VSTDevTools(QObject* parent = nullptr);

    PluginListModel* plugins();
    Q_INVOKABLE void addInstance(unsigned int index);
    Q_INVOKABLE QString pluginName(int index);
    Q_INVOKABLE void play(int index);
    Q_INVOKABLE void showEditor(int index);

    QQmlListProperty<VSTInstanceEditorModel> instances();

private:

    void makeArpeggio();
    std::shared_ptr<midi::MidiStream> m_midiStream;

    PluginListModel* m_pluginsListModel;

    static int instancesCount(QQmlListProperty<VSTInstanceEditorModel>* list);
    static VSTInstanceEditorModel* instanceAt(QQmlListProperty<VSTInstanceEditorModel>* list, int index);

signals:
    void instancesChanged();
};
} // namespace vst
} // namespace mu4

#endif // MU4_VST_VSTDEVTOOLS_H
