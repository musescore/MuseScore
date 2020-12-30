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
#ifndef MU_VST_VSTINSTANCEEDITORMODEL_H
#define MU_VST_VSTINSTANCEEDITORMODEL_H

#include <QObject>
#include "modularity/ioc.h"
#include "ivstinstanceregister.h"

namespace mu {
namespace vst {
class VSTInstanceEditorModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int instanceId READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY idChanged)
    INJECT(vst, IVSTInstanceRegister, vstInstanceRegister)

public:
    VSTInstanceEditorModel(QObject* parent = nullptr);

    instanceId id() const;
    void setId(instanceId id);

    QString name() const;

signals:
    void idChanged(mu::vst::instanceId);

private:
    instancePtr instance() const;
    instanceId m_id = IVSTInstanceRegister::ID_NOT_SETTED;
};
} // namespace vst
} // namespace mu

#endif // MU_VST_VSTINSTANCEEDITORMODEL_H
