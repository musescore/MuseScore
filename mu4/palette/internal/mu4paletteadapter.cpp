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
#include "mu4paletteadapter.h"
#include "log.h"

#include "palette/paletteworkspace.h"
#include "palette/palettetree.h"
#include "palette/palettemodel.h"
#include "palette/palettecreator.h"

using namespace mu;
using namespace mu::palette;

using namespace Ms;

MU4PaletteAdapter::MU4PaletteAdapter()
{
    m_paletteEnabled.val = true;
}

QAction* MU4PaletteAdapter::getAction(const char* id_) const
{
    QString id(id_);
    QAction* a = m_actions.value(id, nullptr);
    if (!a) {
        a = new QAction();
        m_actions.insert(id, a);
    }
    return a;
}

QString MU4PaletteAdapter::actionHelp(const char* id) const
{
    return QString(id);
}

void MU4PaletteAdapter::showMasterPalette(const QString& arg)
{
    Q_UNUSED(arg);
    NOT_IMPLEMENTED;
}

bool MU4PaletteAdapter::isSelected() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    return !notation->interaction()->selection()->isNone();
}

bool MU4PaletteAdapter::applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers)
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    return notation->interaction()->applyPaletteElement(element, modifiers);
}

Ms::PaletteWorkspace* MU4PaletteAdapter::paletteWorkspace() const
{
    if (!m_paletteWorkspace) {
        PaletteTreeModel* emptyModel = new PaletteTreeModel(new PaletteTree);
        PaletteTreeModel* masterPaletteModel = new PaletteTreeModel(PaletteCreator::newMasterPaletteTree());

        m_paletteWorkspace = new PaletteWorkspace(emptyModel, masterPaletteModel, /* parent */ nullptr);
        emptyModel->setParent(m_paletteWorkspace);
        masterPaletteModel->setParent(m_paletteWorkspace);

//        if (WorkspacesManager::currentWorkspace()) {
//            connect(paletteWorkspace, &PaletteWorkspace::userPaletteChanged,
//                    WorkspacesManager::currentWorkspace(), QOverload<>::of(&Workspace::setDirty), Qt::UniqueConnection);
//        }
    }

    return m_paletteWorkspace;
}

ValCh<bool> MU4PaletteAdapter::paletteEnabled() const
{
    return m_paletteEnabled;
}

void MU4PaletteAdapter::setPaletteEnabled(bool arg)
{
    m_paletteEnabled.set(arg);
}

void MU4PaletteAdapter::requestPaletteSearch()
{
    m_paletteSearchRequested.notify();
}

mu::async::Notification MU4PaletteAdapter::paletteSearchRequested() const
{
    return m_paletteSearchRequested;
}

void MU4PaletteAdapter::notifyElementDraggedToScoreView()
{
    m_elementDraggedToScoreView.notify();
}

mu::async::Notification MU4PaletteAdapter::elementDraggedToScoreView() const
{
    return m_elementDraggedToScoreView;
}
