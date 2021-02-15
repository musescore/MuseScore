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
#include "vstinstanceeditormodel.h"
#include "internal/plugininstance.h"

using namespace mu::vst;

VSTInstanceEditorModel::VSTInstanceEditorModel(QObject* parent)
    : QObject(parent)
{
}

instanceId VSTInstanceEditorModel::id() const
{
    return m_id;
}

void VSTInstanceEditorModel::setId(instanceId id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged(m_id);
    }
}

QString VSTInstanceEditorModel::name() const
{
    auto inst = instance();
    if (inst) {
        return QString::fromStdString(inst->plugin().getName());
    }
    return QString();
}

instancePtr VSTInstanceEditorModel::instance() const
{
    return vstInstanceRegister()->instance(m_id);
}
